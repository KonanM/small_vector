[![Actions Status](https://github.com/KonanM/small_vector/workflows/MacOS/badge.svg)](https://github.com/KonanM/small_vector/actions)
[![Actions Status](https://github.com/KonanM/small_vector/workflows/Windows/badge.svg)](https://github.com/KonanM/small_vector/actions)
[![Actions Status](https://github.com/KonanM/small_vector/workflows/Ubuntu/badge.svg)](https://github.com/KonanM/small_vector/actions)
[![Actions Status](https://github.com/KonanM/small_vector/workflows/Install/badge.svg)](https://github.com/KonanM/small_vector/actions)

<img src="logo.png" width="300" align="middle"/>

# small_vector

`sbo::small_vector` is an adapter over `std::vector` with a small buffer. This means that `sbo::small_vector<T,N>` has a customizable initial capacity `N` that is not dynamically allocated on the heap, but on the stack. This allows normal "small" cases to be fast (by avoiding heap allocations) without losing generality for large inputs.

I like the simplicty of this implementation and that `sbo::small_vector` is fully move constructible/ assignable. While the small buffer is not active `sbo::small_vector` behaves identical to `std::vector` and a move is super cheap O(1). Since the small buffer memory is allocated on the stack and it is not relocatable (similar to `std::array`) an element wise move has to be performed when the small buffer is active O(N). 
Otherwise it should basically behave identical to std::vector with the minor difference that moving might invalidate iterators to the `small_vector`.

## Implementation
This implementation was basically inspired by a quite unknown customization point called ['propagate_on_container_move_assignment'](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer), but lets start with the basics.
For our purposes we need a stack allocated piece of memory, which I will refer to as the small buffer. The small buffer can be used to insert elements until we reach `MaxSize`. For memory requests bigger than the small buffer, we will simply use a `std::alllocator`.

The basic requirements for an allocator are quite simple - provide a value type, an allocate and deallocate function (see also https://howardhinnant.github.io/allocator_boilerplate.html ). Lets have a look at how the basic implementation works: 
```cpp
template<typename T, size_t MaxSize>
struct small_buffer_vector_allocator{
    using value_type = T;
    [[nodiscard]] constexpr T* allocate(const size_t n);
    constexpr void deallocate(void* p, const size_t n);
    
    alignas(alignof(T)) std::byte m_smallBuffer[MaxSize * sizeof(T)];
    std::allocator<T> m_alloc{};
    //...
};
```

[Allocator aware](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer) containers like `std::vector` check for a property called `propagate_on_container_move_assignment`, which is defaulted to true. 
This property is needed when two allocators of the same type (but different instantiations) can't deallocate the memory of each other (usually needed for stateful allocators). 
For a vector this means that (on move assignment), it's not possible to simply copy the pointer to the memory of the other vector, because the other vector couldn't deallocate it. 
Instead the moved to vector has to make sure it has enough memory and an element wise move has to be performed into that memory. To indicate that an element wise move is necessary the two instances of the allocator should compare false.

With this knowledge we can now implement our custom allocator:
```cpp
template<typename T, size_t MaxSize>
struct small_buffer_vector_allocator{
    //...
    bool m_smallBufferUsed = false;
    using propagate_on_container_move_assignment = std::false_type;
    using is_always_equal = std::false_type;
    friend constexpr bool operator==(const small_buffer_vector_allocator& lhs, const small_buffer_vector_allocator& rhs) {
        return !lhs.m_smallBufferUsed && !rhs.m_smallBufferUsed;
    }
    friend constexpr bool operator!=(const small_buffer_vector_allocator& lhs, const small_buffer_vector_allocator& rhs) {
        return !(lhs == rhs);
    }
```

The last missing pieces for the allocator implementation are the allocate and deallocate member functions. The real implementation is a bit more complex, due to one implementation detail I left out (allocator rebinding, which is often usedfor the implmentation for debug iterators), but since it otherwise doesn't differ I will keep it a bit more simple here.
```cpp
    [[nodiscard]] constexpr T* allocate(const size_t n) {
        //use the small buffer
        if( n <= MaxSize) {
            m_smallBufferUsed = true;
            return reinterpret_cast<T*>(&m_smallBuffer);
        }
        m_smallBufferUsed = false;
        //otherwise use the default allocator
        return m_alloc.allocate(n);
    }
    constexpr void deallocate(void* p, const size_t n) {
        //we don't deallocate anything if the memory was allocated in small buffer
        if (&m_smallBuffer != p)
            m_alloc.deallocate(static_cast<T*>(p), n);
        m_smallBufferUsed = false;
    }
```
After the allocator implementaion, I simply derived from `std::vector` and made sure the constructors reserve the memory for the small buffer before they do any insertions. This whole `small_vector` implementation is only less than 100 lines (mostly constructors), which hopefully only leaves little room for mistakes.


```cpp
template<typename T, size_t N = 8>
class small_vector : public std::vector<T, small_buffer_vector_allocator<T, N>>{
public:
    using vectorT = std::vector<T, small_buffer_vector_allocator<T, N>>;
    //default initialize with the small buffer size
    constexpr small_vector() noexcept { vectorT::reserve(N); }
    small_vector(const small_vector&) = default;
    small_vector& operator=(const small_vector&) = default;
    small_vector(small_vector&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        vectorT::reserve(N);
        vectorT::operator=(std::move(other));
    }
    small_vector& operator=(small_vector&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        vectorT::reserve(N);
        vectorT::operator=(std::move(other));
        return *this;
    }
    //use the default constructor first to reserve then construct the values
    explicit small_vector(size_t count) : small_vector() { vectorT::resize(count); }
    small_vector(size_t count, const T& value)  : small_vector() { vectorT::assign(count, value); }
    template< class InputIt >
    small_vector(InputIt first, InputIt last)   : small_vector() { vectorT::insert(vectorT::begin(), first, last); }
    small_vector(std::initializer_list<T> init) : small_vector() { vectorT::insert(vectorT::begin(), init); }
};
```

## Benchmarks

I used google benmark to test the performance against `std::vector` and `llvm_smalvec::SmallVector`. You can rerun the tests on your machine with 0 configuration overhead when you open the `CMakeLists.txt` folder bench.
Some unsurprising key takeaways:

- Constructing the `sbo::small_vector` is more expensive than (default) constructing `std::vector` or `llvm_smalvec::SmallVector`, because we introduce the overhead of having to call `.reserve()` (2ns vs. 10ns). 
- The speed difference against other small vector implementations can be explained by this overhead
- `sbo::small_vector` is faster than `std::vector` when the small buffer is active, because we save the initial allocation
- When the small buffer is not active the performance is nearly identical, because our allocator only performs very cheap operations compared to an allocation. 
- For some use cases the better cache locality of `sbo::small_vector` can make a (small) difference (compared against a vector with it's size reserved)
- Since most of the timing gains can be achieved by saving the the initial dynamic allocation of `std::vector`, I don't think `small_vector` is worth it for types that need a dynamic allocation.
- It's seems to be easier for some compilers to completely optimize  

tldr: Don't use this implementation if you need the last bit of performance, a custom `small_vector` is cheaper to construct. Use it when simplicity, correctness and exception safety matter.

## Usage
There are three very easy options:

- simply copy the header to your project
- copy the few lines of code directly
- use CPM 
```
CPMAddPackage(
  NAME small_vector
  GITHUB_REPOSITORY konanM/small_vector
  VERSION 1.0
)
```
## License (unlicense)
See https://unlicense.org, tldr: do whatever you want including removing the license
