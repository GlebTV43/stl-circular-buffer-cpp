#pragma once

#include <memory>
#include <cstddef>
#include <initializer_list>
#include <cassert>
#include <stdexcept>
#include <utility>

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

    explicit circular_buffer(size_type capacity) : circular_buffer(capacity, allocator_type{}) {}

    explicit circular_buffer(const allocator_type& allocator) : circular_buffer(0, allocator) {}

    circular_buffer(size_type capacity, const allocator_type& allocator) : allocator_(allocator), capacity_(capacity)
    {
        if (capacity != 0)
        {
            data_ = allocator_traits::allocate(allocator_, capacity_);
        }
    }

    // rof 

    circular_buffer(const circular_buffer& other) : allocator_(allocator_traits::select_on_container_copy_construction(other.allocator_)), 
        capacity_(other.capacity_)
    {
        if (capacity_ == 0) return ;

        data_ = allocator_traits::allocate(allocator_, capacity_);

        try
        {
            for (; size_ < other.size_; ++size_)
            {
                allocator_traits::construct(allocator_, std::addressof(data_[size_]), other[size_]);
            }
        }
        catch(...)
        {
            clear();

            allocator_traits::deallocate(allocator_, data_, capacity_);

            throw; // пробрасываю дальше 
        }
    }

    circular_buffer(circular_buffer&& other) noexcept : allocator_(std::move(other.allocator_))
    {
        take_memory(other);
    }

    circular_buffer& operator=(const circular_buffer& other) 
    {
        if (this == &other)
        {
            return *this;
        }

        allocator_type new_alloc = allocator_;

        if constexpr (allocator_traits::propagate_on_container_copy_assignment::value)
        {
            new_alloc = other.allocator_;
        }

        circular_buffer tmp(other.capacity_, new_alloc);

        for (;tmp.size_ < other.size_; ++tmp.size_)
        {
            allocator_traits::construct(new_alloc, std::addressof(tmp.data_[tmp.size_]), other[tmp.size_]);
        }

        clear_memory();

        if constexpr (allocator_traits::propagate_on_container_copy_assignment::value)
        {
            allocator_ = other.allocator_;
        }

        take_memory(tmp);

        return *this;
    }

    circular_buffer& operator=(circular_buffer&& other)
    {
        if (this == &other)
        {
            return *this;
        }

        if constexpr (allocator_traits::propagate_on_container_move_assignment::value)
        {
            clear_memory();
            allocator_ = std::move(other.allocator_);
            take_memory(other);
        } else if (allocator_ == other.allocator_)
        {
            clear_memory();
            take_memory(other);
        } else 
        {
            circular_buffer tmp(other.capacity_, allocator_);

            for (; tmp.size_ < other.size_; ++tmp.size_)
            {
                allocator_traits::construct(allocator_, std::addressof(tmp.data_[tmp.size_]), std::move(other[tmp.size_]));
            }

            clear_memory();
            take_memory(tmp);
            other.clear();
        }

        return *this;
    }

    ~circular_buffer() noexcept
    {
        clear_memory();
    }

private:

    allocator_type allocator_;

    pointer data_{};
    size_type size_ = 0;
    size_type capacity_ = 0;
    size_type head_ = 0;

    size_type real_index(size_type logical_index) const noexcept
    {
        assert(capacity_ != 0);
        return (head_ + logical_index) % capacity_;
    }

    void clear_memory() noexcept
    {
        clear();

        if (capacity_ != 0)
        {
            allocator_traits::deallocate(allocator_, data_, capacity_);
        }

        data_ = pointer{};
        capacity_ = 0;
    }

    void take_memory(circular_buffer& other) noexcept
    {
        data_ = std::exchange(other.data_, pointer{});
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        head_ = std::exchange(other.head_, 0);
    }

    void double_memory()
    {
        if (capacity_ == 0)
        {
            const size_type start_value = 4;
            data_ = allocator_traits::allocate(allocator_, start_value);
            capacity_ = start_value;
            return;
        }

        const size_type new_capacity = capacity_ * 2;

        circular_buffer tmp(new_capacity, allocator_);

        for (; tmp.size_ < size_; ++tmp.size_)
        {
            allocator_traits::construct(tmp.allocator_, std::addressof(tmp.data_[tmp.size_]), (*this)[tmp.size_]);
        }

        clear_memory();
        take_memory(tmp);
    }

public:

    template<bool IsConst, bool IsReverse>
    class basic_iterator;

    using iterator = basic_iterator<false, false>;

    using const_iterator = basic_iterator<true, false>;

    using reverse_iterator = basic_iterator<false, true>;

    using const_reverse_iterator = basic_iterator<true, true>;

    size_type size() const
    {
        return size_;
    }

    size_type capacity() const 
    {
        return capacity_;
    }

    iterator begin()
    {
        return iterator(this, 0);
    }

    iterator end()
    {
        return iterator(this, size_);
    }

    const_iterator begin() const
    {
        return const_iterator(this, 0);
    }

    const_iterator end() const
    {
        return const_iterator(this, size_);
    }

    const_iterator cbegin() const
    {
        return const_iterator(this, 0);
    }

    const_iterator cend() const
    {
        return const_iterator(this, size_);
    }
    
    reverse_iterator rbegin()
    {
        return reverse_iterator(this, 0);
    }

    reverse_iterator rend()
    {
        return reverse_iterator(this, size_);
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(this, 0);
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(this, size_);
    }

    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(this, 0);
    }

    const_reverse_iterator crend() const
    {
        return const_reverse_iterator(this, size_);
    }

    bool empty() const
    {
        return (size_ == 0);
    }

    reference operator[](size_type n)
    {
        return data_[real_index(n)];
    }

    reference at(size_type n)
    {
        if (n >= size_)
        {
            throw std::out_of_range("circular_buffer::at");
        }

        return (*this)[n];
    }

    const_reference operator[](size_type n) const
    {
        return data_[real_index(n)];
    }

    const_reference at(size_type n) const
    {
        if (n >= size_)
        {
            throw std::out_of_range("circular_buffer::at(const)");
        }

        return (*this)[n];
    }    

    allocator_type get_allocator() const
    {
        return allocator_;
    }

    reference front()
    {
        assert(!empty());
        return (*this)[0];
    }

    reference back()
    {
        assert(!empty());
        return (*this)[size_ - 1];
    }

    const_reference front() const
    {
        assert(!empty());
        return (*this)[0];
    }

    const_reference back() const
    {
        assert(!empty());
        return (*this)[size_ - 1];
    }

    void clear()
    {
        for (size_type n = 0; n < size_; ++n)
        {
            allocator_traits::destroy(allocator_, std::addressof(data_[real_index(n)]));
        }

        head_ = 0;
        size_ = 0;
    }

    void push_back(const value_type& value)
    {
        if (size_ == capacity_)
        {
            if constexpr (Extendable)
            {
                double_memory();
            } else 
            {
                throw std::length_error("can`t extand buffer");
            }
        }
        allocator_traits::construct(allocator_, std::addressof(data[real_index(size_)]), value);
        ++size_;
    }

    void push_back(value_type&& value)
    {
        if (size_ == capacity_)
        {
            if constexpr (Extendable)
            {
                double_memory();
            } else 
            {
                throw std::length_error("can`t extand buffer");
            }
        }
        allocator_traits::construct(allocator_, std::addressof(data[real_index(size_)]), std::move(value));
        ++size_;
    }

    void pop_back()
    {
        if (size_ == 0)
        {
            throw std::logic_error("can`t pop nothing");
        }

        allocator_traits::destroy(allocator_, std::addressof(data_[real_index(size_ - 1)]));
        --size_;
    }

    void push_front(const value_type& value)
    {
        if (size_ == capacity_)
        {
            if constexpr (Extendable)
            {
                double_memory();
            } else 
            {
                throw std::length_error("can`t extand buffer");
            }
        }

        const size_type new_head = (head_ + capacity_ - 1) % capacity_;

        allocator_traits::construct(allocator_, std::addressof(data[new_head]), value);
        ++size_;

        head_ = new_head;
    }

    void push_front(value_type&& value)
    {
        if (size_ == capacity_)
        {
            if constexpr (Extendable)
            {
                double_memory();
            } else 
            {
                throw std::length_error("can`t extand buffer");
            }
        }
        const size_type new_head = (head_ + capacity_ - 1) % capacity_;

        allocator_traits::construct(allocator_, std::addressof(data[new_head]), std::move(value));
        ++size_;

        head_ = new_head;
    }

    void pop_front()
    {
        if (size_ == 0)
        {
            throw std::logic_error("can`t pop nothing");
        }

        allocator_traits::destroy(allocator_, std::addressof(data_[head_]));
        --size_;
        head_ = (head_ +1) % capacity_;
    }
};

#include "iterators.h"
