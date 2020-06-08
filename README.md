[![Actions Status](https://github.com/KonanM/small_vector/workflows/MacOS/badge.svg)](https://github.com/KonanM/small_vector/actions)
[![Actions Status](https://github.com/KonanM/small_vector/workflows/Windows/badge.svg)](https://github.com/KonanM/small_vector/actions)
[![Actions Status](https://github.com/KonanM/small_vector/workflows/Ubuntu/badge.svg)](https://github.com/KonanM/small_vector/actions)
[![Actions Status](https://github.com/KonanM/small_vector/workflows/Style/badge.svg)](https://github.com/KonanM/small_vector/actions)
[![Actions Status](https://github.com/KonanM/small_vector/workflows/Install/badge.svg)](https://github.com/KonanM/small_vector/actions)

<img src="logo.png" width="300" align="middle"/>

# small_vector

I will keep it short - `sbo::small_vector` is basically an adapter over `std::vector` with a small buffer. This means that `sbo::small_vector<T,N>` has a customizable initial capacity `N` that is not dynamically allocated on the heap, but on the stack.


What makes this implementation unique (compared to allocators that use a small buffer) is that `sbo::small_vector` fully move constructible. While the small buffer is not active `sbo::small_vector` behaves identical to `std::vector` and a move is super cheap O(1). Since the small buffer memory is allocated on the stack and it is not relocatable (similar to `std::array`) an element wise move has to be performed when the small buffer is active O(N). 
Otherwise it should basically behave identical to std::vector with the minor difference that moving might invalidate iterators to the `small_vector`.


## Implementation
This implementation was basically inspired by a quite unknown customization point called ['propagate_on_container_move_assignment'](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer). 

[Allocator aware](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer) containers like `std::vector` check for this property and do the element wise move when the allocators don't compare equal. So I implemted a small custom allocator which sets `propagate_on_container_move_assignment = false` and doesn't compare equal when the small buffer is active. 
After the allocator implementaion, I simply derived from `std::vector` and made sure the constructors reserve the memory for the small buffer before they do any insertions. While the allocator implementation is quite short it was surprisingly tricky to get right (because an allocator can be rebound). The whole `small_vector` implementation is only a few lines, which hopefully only leaves little room for mistakes.

```cpp
    template<typename T, size_t N = 8>
    class small_vector : public std::vector<T, small_buffer_vector_allocator<T, N>>{
    public:
        using vectorT = std::vector<T, small_buffer_vector_allocator<T, N>>;
        //default initialize with the small buffer size
        small_vector() noexcept { vectorT::reserve(N); }
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

I used google benmark to test the performance against `std::vector` and unsurprisingly `sbo::small_vector` is faster when the small buffer is active. When the small buffer is not active the performance is nearly identical (some overhead when constructing `sbo::small_vector`), but otherwise our allocator only performs very cheap operations compared to an allocation. Most of the timing gains can be achieved by saving the the dynamic allocation of `std::vector`. So for types that need a dynamic allocation themselves, I don't think `small_vector` is worth it. 

So best use it for non allocating types where you (on average) only have a few elements .

## Usage



## License (unlicense/public domain)
Do whatever you want with this small piece of code :-)