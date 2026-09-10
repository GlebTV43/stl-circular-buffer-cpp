#pragma once

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
    
    using iterator_concept = std::random_access_iterator_tag;

    using value_type = T;

    using difference_type = typename container_type::difference_type;
    
    using reference = std::conditional_t<IsConst, const value_type&, value_type&>;

    using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;

    using size_type = typename container_type::size_type;

    basic_iterator() = default; // тк есть другой закрытый конструктор =6 

    template<bool OtherConst>
    requires (IsConst && !OtherConst)
    basic_iterator(const basic_iterator<OtherConst, IsReverse>& other) : container_(other.container_), position_(other.position_) {}

    reference operator*() const
    {
        assert(container_ != nullptr);
        assert(position_ >= 0);
        assert(position_ < container_->size());

        return container_->data_[real_index(position_)];
    }

    pointer operator->() const // прочитал про std::adressof, его исполоьзвать лучш тк пользовательский тип может перегрузить оператор взятия адреса &
    {
        return std::addressof(**this);
    }

    basic_iterator& operator++()
    {
        assert(container_ != nullptr);
        assert(position_ >= 0);
        assert(position_ < container_->size());

        ++position_;
        return *this; 
    }

    basic_iterator operator++(int)
    {
        basic_iterator iter = *this;
        ++(*this);
        return iter;
    }

    basic_iterator& operator--()
    {
        assert(container_ != nullptr);
        assert(position_ > 0);
        assert(position_ <= container_->size());

        --position_;
        return *this;
    }

    basic_iterator operator--(int)
    {
        basic_iterator iter = *this;
        --(*this);
        return iter;
    }

    basic_iterator& operator+=(difference_type n)
    {
        assert(container_ != nullptr);

        difference_type new_pos = position_ + n;
        
        assert(new_pos >= 0);
        assert(new_pos <= container_->size());

        position_ = new_pos;
        return *this;
    }

    basic_iterator& operator-=(difference_type n)
    {
        assert(container_ != nullptr);

        difference_type new_pos = position_ - n;
        
        assert(new_pos >= 0);
        assert(new_pos <= container_->size());

        position_ = new_pos;
        return *this;
    }

    basic_iterator operator+(difference_type n) const
    {
        basic_iterator iter = *this;
        iter += n;
        return iter; 
    }

    friend basic_iterator operator+(difference_type n, const basic_iterator& iter)
    {
        return iter + n;
    }

    basic_iterator operator-(difference_type n) const
    {
        basic_iterator iter = *this;
        iter -= n;
        return iter;
    }

    template<bool OtherConst>
    difference_type operator-(const basic_iterator<OtherConst, IsReverse>& other) const
    {
        assert(container_ != nullptr);
        assert(container_ == other.container_);

        return position_ - other.position_;
    }

    reference operator[](difference_type n) const
    {
        return *(*this + n);
    }

    friend bool operator==(const basic_iterator& left, const basic_iterator& right) 
    {
        return left.container_ == right.container_ && left.position_ == right.position_;
    }

    friend bool operator<(const basic_iterator& left, const basic_iterator& right)
    {
        assert(left.container_ == right.container_); // не разрешаю сравнивать итераторы из двух разных контейнеров 
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

    basic_iterator(real_container_type* container, difference_type index) : container_(container), position_(index) {}

    size_type real_index(size_type pos) const
    {
        assert(pos < container_->size()); // балуюсь ассертами, надо спросить насколько это хорошо? я вот просто не хочу пользователю разрешать такие действия 

        size_type real_index_k;
        if constexpr (IsReverse)
        {
            real_index_k = container_->size() - pos - 1;
        }
        else 
        {
            real_index_k = pos;
        }

        return (container_->head_ + real_index_k) % container_->capacity();
    }

    friend class circular_buffer<T, Extendable, Allocator>;

    template<bool, bool>
    friend class basic_iterator;
};

// еще касты по хорошему добавить. 