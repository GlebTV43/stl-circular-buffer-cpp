#pragma once

#include "circular_buffer.h" // pragma once остановит тк я подключаю его в самом конце. чисто чтобы эту бесячую красную полоску убрать 

#include <cassert>
#include <iterator>
#include <memory>
#include <type_traits>

template<typename T, bool Extendable, typename Allocator>
template<bool IsConst, bool IsReverse> 
class circular_buffer<T, Extendable, Allocator>::basic_iterator {
public:
    using container_type = circular_buffer<T, Extendable, Allocator>;

    using iterator_category = std::random_access_iterator_tag;

    using value_type = T;

    using difference_type = typename container_type::difference_type;
    
    using reference = std::conditional_t<IsConst, const value_type&, value_type&>;

    using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;

    basic_iterator() = default; // тк есть другой закрытый конструктор =6 

    reference operator*() const;
    pointer operator->() const;

    basic_iterator& operator++();
    basic_iterator operator++(int);

    basic_iterator& operator--();
    basic_iterator operator--(int);

    basic_iterator& operator+=(difference_type n);

    basic_iterator& operator-=(difference_type n);

    basic_iterator& operator+(difference_type n)
    {

    }

    basic_iterator& operator-(difference_type n)
    {

    }

    basic_iterator& operator+(const basic_iterator& other)
    {

    }

    basic_iterator& operator-(const basic_iterator& other)
    {

    }



    reference operator[](difference_type n) const;

    friend bool operator==(const basic_iterator& left, const basic_iterator& right) 
    {
        return left.container_ == right.container_ && left.position_ == right.position_;
    }

    friend bool operator<(const basic_iterator& left, const basic_iterator& right)
    {
        assert(left.container_ == right.container_);
        return left.position_ < right.position_;
    }

    friend bool operator>(const basic_iterator& left, const basic_iterator& right)
    {
        return right < left;
    }

    friend bool operator>=(const basic_iterator& left, const basic_iterator& right)
    {
        return !(left < right);
    }

    friend bool operator<=(const basic_iterator& left, const basic_iterator& right)
    {
        return !(right < left);
    }

    friend bool operator!=(const basic_iterator& left, const basic_iterator& right) 
    {
        return !(left == right);
    }

private:

    using real_container_type = std::conditional_t<IsConst, const container_type, container_type>;

    real_container_type* container_ = nullptr;

    difference_type position_ = 0;

    basic_iterator(real_container_type* container, difference_type index) : container_(container), position_(index) {};

    friend class circular_buffer<T, Extendable, Allocator>;

    template<bool, bool>
    friend class basic_iterator;
};