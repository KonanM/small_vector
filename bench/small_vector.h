//
//  small_vector.h
//  small_vector
//
//  Created by Ben Saratikyan on 11/9/19.
//  Copyright © 2019 Ben Saratikyan. All rights reserved.
//

#ifndef small_vector_h
#define small_vector_h

/**
 * @brief small_vector<T, N> is a template container which contains N elements in stack memory and if size increases
 *        in N it moves all its elements to heap memory and after that starts collecting new elements in heap memory.
 *
 * @tparam T  First template parameter indicates container's value type
 * @tparam N  Second template parameter indicates container's stack mamory size
 *
 * @author Sincerely Yours Ben Saratikyan
 *
 * @link  https://github.com/bensaratikian/small_vector
 */
template<typename T, std::size_t N>
class small_vector {
    static_assert(N > 0, "Number of small vector elements in stack can't be 0!");
public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef T value_type;

    /**
     * @brief Default constructor.
     */
    small_vector()
    : _data()
    , _size(0)
    ,_isCArray(true) {}
    
    /**
     * @brief Parameterized constructor.
     *
     * @param n Initial size and capacity to be set
     */
    small_vector(std::size_t n) {
        if (n > N) {
            new (&_data._vec) std::vector<T>(n);
            _isCArray = false;
        } else {
            for (std::size_t i = 0; i < n; ++i) {
                new (&_data._arr[i]) T();
            }
            _isCArray = true;
        }
        _size = n;
    }
    
    /**
     * @brief Copy constructor.
     */
    small_vector(const small_vector<T, N>& other) {
        if(!other._isCArray) {
            new (&_data._vec) std::vector<T>(other._data._vec);
        } else {
            for (std::size_t i = 0; i < other._size; ++i) {
                new (&_data._arr[i]) T(other._data._arr[i]);
            }
        }
        _size = other._size;
        _isCArray = other._isCArray;
    }
    
    /**
     * @brief Constructor with initializer list.
     *
     * @param il initializer list to be used
     */
    small_vector(std::initializer_list<T> il)
    : _data()
    , _size(il.size())
    , _isCArray(false) {
        if(il.size() > N) {
            new (&_data._vec) std::vector<T>(il.begin(), il.end());
        } else {
            std::size_t i = 0;
            for (auto it = il.begin(); it < il.end(); ++it, ++i) {
                new (&_data._arr[i]) T(*it);
            }
            _isCArray = true;
        }
    }
    
    /**
     * @brief Copy assignment operator.
     */
    small_vector<T, N>& operator=(const small_vector<T, N>& other) {
        if (this == &other) return *this;
          if (!other._isCArray) {
              new (&_data._vec) std::vector<T>(other._data._vec);
          } else {
              for (std::size_t i = 0; i < other._size; ++i) {
                  new (&_data._arr[i]) T(other._data._arr[i]);
              }
          }
        _size = other._size;
        _isCArray = other._isCArray;
        return *this;
    }
    
    /**
     * @brief Operator equals checks equality of two objects belonging to this class.
     */
    bool operator==(const small_vector<T, N>& other) const {
        if (_size != other._size) return false;
        if (this == &other) return true;
        if (_isCArray) {
            for (std::size_t i = 0; i < _size; ++i) {
                if (other._data._arr[i] != _data._arr[i]) return false;
            }
            return true;
        } else {
            return _data._vec == other._data._vec;
        }
    }
    
    /**
     * @brief Does the reverse action of equals operator.
     */
    bool operator!=(const small_vector<T, N>& other) const {
        return !(*this == other);
    }
    
    /**
     * @brief Returns a reference to the element at position n in the vector container.
     */
    const T& operator [](std::size_t idx) const noexcept {
        if (_isCArray) {
            return _data._arr[idx];
        } else {
            return _data._vec[idx];
        }
    }
    
    /**
     * @brief Returns a reference to the element at position n in the vector container.
     */
    T& operator [](std::size_t idx) noexcept {
       return const_cast<T&>(static_cast<const small_vector<T, N>&>(*this)[idx]);
    }
    
    /**
     * @brief Adds a new element at the end of the vector, after its current last element.
     * This effectively increases the container size by one, which causes an automatic reallocation
     * of the allocated storage space if -and only if- the new vector size surpasses the current vector capacity.
     *
     * @param arg 's content is copied to the new element.
     */
    void push_back(const T& arg) noexcept {
        if (!_isCArray) {
            _data._vec.push_back(arg);
        } else if (_size == N) {
            std::vector<T> temp;
            temp.reserve(2 * N);
            for (T& t : _data._arr) {
                temp.push_back(std::move(t));
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    t.~T();
                }
            }

            temp.push_back(arg);
            new (&_data._vec) std::vector<T>(std::move(temp));
            _isCArray = false;
        } else {
            new (&_data._arr[_size]) T(arg);
        }
        ++_size;
    }
    
    /**
     * @brief Appends a new element to the end of the container. The element is constructed through
     * std::allocator_traits::construct, which typically uses placement-new to construct the element
     * in-place at the location provided by the container.
     *
     * @param args are forwarded to the constructor as std::forward<Args>(args)...
     */
    template<typename... Args>
    void emplace_back(Args&&... args) noexcept {
        if (!_isCArray) {
            _data._vec.emplace_back(std::forward<Args>(args)...);
        } else if (_size == N) {
            std::vector<T> temp;
            temp.reserve(2 * N);
            for (T& t : _data._arr) {
                temp.push_back(std::move(t));
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    t.~T();
                }
            }

            temp.emplace_back(std::forward<Args>(args)...);
            new (&_data._vec) std::vector<T>(std::move(temp));
            _isCArray = false;
        } else {
            new (&_data._arr[_size]) T(std::forward<Args>(args)...);
        }
        ++_size;
    }
    
    /**
     * @brief Removes the last element in the small_vector, effectively reducing the container size by one.
     */
    void pop_back() noexcept {
        //TODO: add DCHECK
        if(_size) {
        if(_isCArray) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                _data._arr[_size - 1].~T();
            }
        } else {
            _data._vec.pop_back();
        }
        --_size;
        }
    }
    
    /**
     * @brief If n is greater than the current vector capacity, the function causes the container to
     * reallocate its storage increasing its capacity to n (or greater).
     * In all other cases, the function call does not cause a reallocation and the vector capacity is not affected.
     * This function has no effect on the vector size and cannot alter its elements.
     */
    void reserve(std::size_t n) noexcept {
        if(n <= N) return;
        
        if(!_isCArray) {
            _data._vec.reserve(n);
        } else {
            std::vector<T> temp(begin(), end());
            for (std::size_t i = 0; i < _size; ++i) {
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    _data._arr[i].~T();
                }
            }
            
            new (&_data._vec) std::vector<T>(std::move(temp));
            _data._vec.reserve(n);
            _isCArray = false;
        }
    }
    
    /**
     * @brief Resizes the container so that it contains n elements.
     * If n is smaller than the current container size, the content is reduced to its first n elements, removing those beyond (and destroying them).
     * If n is greater than the current container size, the content is expanded by inserting at the end as many elements as needed to
     * reach a size of n. If val is specified, the new elements are initialized as copies of val, otherwise, they are value-initialized.
     * If n is also greater than the current container capacity, an automatic reallocation of the allocated storage space takes place.
     */
    void resize(std::size_t n) noexcept {
        if(!_isCArray) {
            _data._vec.resize(n);
        } else if (n > N) {
            std::vector<T> temp(begin(), end());
            clear();
            new (&_data._vec) std::vector<T>(std::move(temp));
            _data._vec.resize(n);
            _isCArray = false;
        } else {
            for (std::size_t i = n; i < _size; ++i) {
                if constexpr (!std::is_trivially_destructible_v<T>)
                    _data._arr[i].~T();
            }
        }
        _size = n;
    }
    
    /**
     * @brief Removes all elements from the vector (which are destroyed), leaving the container with a size of 0.
     */
    void clear() noexcept {
        if(!_size) return;
        
        if (_isCArray) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (std::size_t i = 0; i < _size; ++i) {
                    _data._arr[i].~T();
                }
            }
        } else {
            _data._vec.clear();
        }
        _size = 0;
    }
    
    /**
     * @brief Tests whether vector is empty
     */
    bool empty() const {
        return !_size;
    }

    /**
     * @brief Returns the number of elements in the small_vector
     */
    std::size_t size() const {
        return _size;
    }
    
    /**
     * @brief Returns a reference to the first element in the container.
     * Calling front on an empty container is undefined.
     */
    const T& front() const {
           return _isCArray ? _data._arr[0] : _data._vec.front();
    }
    
    /**
     * @brief Returns a reference to the last element in the container.
     * Calling back on an empty container is undefined.
     */
    const T& back() const {
        return _isCArray ? _data._arr[_size - 1] : _data._vec.back();
    }
    
    /**
     * @brief Returns a pointer pointing to the first element in the vector.
     */
    T* begin() {
        return _isCArray ? _data._arr : &_data._vec.front();
    }
    
    /**
     * @brief Returns an pointer referring to the past-the-end element in the small_vector container.
     */
    T* end() {
        return _isCArray ? _data._arr + _size : &_data._vec.back() + 1;
    }
    
    /**
     * @brief Returns a pointer-to-const pointing to the first element in the vector.
     */
    const T* begin() const {
        return _isCArray ? _data._arr : &_data._vec.front();
    }
    
    /**
     * @brief Returns a pointer-to-const referring to the past-the-end element in the small_vector container.
     */
    const T* end() const {
        return _isCArray ? _data._arr + _size : &_data._vec.back() + 1;
    }
    
    /**
     * @brief Returns a direct pointer to the memory array used internally by the small_vector to store its owned elements.
     */
    const T* data() const {
          return begin();
    }
    
    /**
     * @brief Destructor of small_vector container
     */
    ~small_vector() {
        clear();
        if(!_isCArray) _data._vec.~vector<T>();
    }

private:
    union Data {
        Data() {}
        
        ~Data() {}
        
        T _arr[N];
        std::vector<T> _vec;
    } _data;
    
    std::size_t _size;
    bool _isCArray;
};

#endif /* small_vector_h */
