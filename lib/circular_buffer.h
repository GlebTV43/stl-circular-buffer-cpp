#pragma once

#include <memory>
#include <cstddef>
#include <initializer_list>

template<typename T, bool Extendable = false, typename Allocator = std::allocator<T>>
class circular_buffer {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    using allocator_type = Allocator;
    using allocator_traits = std::allocator_traits<allocator_type>;

    using pointer = allocator_traits::pointer;
    using const_pointer = allocator_traits::const_pointer;

    using difference_type = typename allocator_traits::difference_type;
    using size_type = typename allocator_traits::size_type;

    // ну погнали, ... конструкторы заново проходить 

    circular_buffer() = default;

    explicit circular_buffer(size_type capacity);

    explicit circular_buffer(const allocator_type& allocator);

    circular_buffer(size_type capacity, const allocator_type& allocator);

private:

    allocator_type allocator_;

    pointer data_{};
    size_type size_ = 0;
    size_type capacity_ = 0;
    size_type head_ = 0;

public:

    template<bool IsConst, bool IsReverse>
    class basic_iterator;

    using iterator = basic_iterator<false, false>;

    using const_iterator = basic_iterator<true, false>;

    using reverse_iterator = basic_iterator<false, true>;

    using const_reverse_iterator = basic_iterator<true, true>;
};

#include "iterators.h"
