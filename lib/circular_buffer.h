#pragma once

#include <memory>
#include <cstddef>
#include <initializer_list>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <type_traits>

template<typename T, bool Extendable = false, typename Allocator = std::allocator<T>>
class circular_buffer {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    using allocator_type = Allocator;
    using allocator_traits = std::allocator_traits<allocator_type>;

    using pointer = typename allocator_traits::pointer;
    using const_pointer = typename allocator_traits::const_pointer;

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

    // special ctors

    circular_buffer(size_type times, const value_type& value) : circular_buffer(times)
    {
        for (size_type t = 0; t < times; ++t)
        {
            push_back(value);
        }
    }

    circular_buffer(std::initializer_list<value_type> values) : circular_buffer(values.size())
    {
        for (const value_type& value : values)
        {
            push_back(value);
        }
    }

    circular_buffer(circular_buffer&& other, const allocator_type& allocator) : allocator_(allocator)
    {
        if (allocator_ == other.allocator_)
        {
            take_memory(other);
            return;
        }

        if (other.capacity_ == 0) return;

        capacity_ = other.capacity_;
        data_ = allocator_traits::allocate(allocator_, capacity_);

        try
        {
            for (; size_ < other.size_; ++size_)
            {
                allocator_traits::construct(allocator_, std::addressof(data_[size_]), std::move_if_noexcept(other[size_]));
            }
        }
        catch(...)
        {
            clear_memory();
            throw;
        }
        
        other.clear();
    }

    circular_buffer& operator=(std::initializer_list<value_type> list)
    {
        assign(list);
        return *this;
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

    void reallocate(size_type new_capacity) {
    pointer new_data = allocator_traits::allocate(allocator_, new_capacity);

    size_type constructed = 0;

    try {
        for (; constructed < size_; ++constructed) {
            allocator_traits::construct(allocator_, new_data + constructed, std::move_if_noexcept((*this)[constructed]));
        }
    } catch (...) {
        for (size_type i = 0; i < constructed; ++i) 
        {
            allocator_traits::destroy(allocator_, new_data + i);
        }

        allocator_traits::deallocate(allocator_, new_data, new_capacity);

        throw;
    }

    for (size_type i = 0; i < size_; ++i) 
    {
        allocator_traits::destroy(allocator_, data_ + real_index(i));
    }

    if (data_ != nullptr) 
    {
        allocator_traits::deallocate(allocator_, data_, capacity_);
    }

    data_ = new_data;
    capacity_ = new_capacity;
    head_ = 0;
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

    size_type max_size() const
    {
        return allocator_traits::max_size(allocator_);
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
        if (empty()) throw std::out_of_range("buffer has no obj");
        return (*this)[0];
    }

    reference back()
    {
        if (empty()) throw std::out_of_range("buffer has no obj");
        return (*this)[size_ - 1];
    }

    const_reference front() const
    {
        if (empty()) throw std::out_of_range("buffer has no obj");
        return (*this)[0];
    }

    const_reference back() const
    {
        if (empty()) throw std::out_of_range("buffer has no obj");
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

    bool operator==(const circular_buffer& other) const
    {
        if (size_ != other.size_)
        {
            return false;
        }

        for (size_type ind = 0; ind < size_; ++ind)
        {
            if ((*this)[ind] != other[ind])
            {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const circular_buffer& other) const
    {
        return !(*this == other);
    }

    // push & pop

    void push_back(const value_type& value)
    {
        if (size_ < capacity_)
        {
            allocator_traits::construct(allocator_, std::addressof(data_[real_index(size_)]), value);
            ++size_;
            return;
        }

        if constexpr (Extendable)
        {
            value_type save = value;

            double_memory();

            allocator_traits::construct(allocator_, std::addressof(data_[real_index(size_)]), save);
            ++size_;
        }
        else
        {
            if (capacity_ == 0)
            {
                return;
            }

            data_[head_] = value;
            head_ = (head_ + 1) % capacity_;
        }
    }

    void push_back(value_type&& value)
    {
        if (size_ < capacity_)
        {
            allocator_traits::construct(allocator_, std::addressof(data_[real_index(size_)]), std::move(value));
            ++size_;
            return;
        }

        if constexpr (Extendable)
        {
            value_type save = std::move(value);

            double_memory();

            allocator_traits::construct(allocator_, std::addressof(data_[real_index(size_)]), std::move(save));
            ++size_;
        }
        else
        {   
            if (capacity_ == 0)
            {
                return;
            }

            value_type save = std::move(value);

            data_[head_] = std::move(save);
            head_ = (head_ + 1) % capacity_;
        }
    }

    void pop_back()
    {
        if (size_ == 0)
        {
            throw std::out_of_range("can`t pop nothing");
        }

        allocator_traits::destroy(allocator_, std::addressof(data_[real_index(size_ - 1)]));
        --size_;
    }

    void push_front(const value_type& value)
    {
        if (size_ < capacity_)
        {
            const size_type new_head = (head_ + capacity_ - 1) % capacity_;

            allocator_traits::construct(allocator_, std::addressof(data_[new_head]), value);

            head_ = new_head;
            ++size_;
            return;
        }

        if constexpr (Extendable)
        {
            value_type save = value;

            double_memory();

            const size_type new_head = (head_ + capacity_ - 1) % capacity_;

            allocator_traits::construct(allocator_, std::addressof(data_[new_head]), save);

            head_ = new_head;
            ++size_;
        }
        else
        {
            if (capacity_ == 0)
            {
                return;
            }

            const size_type new_head = (head_ + capacity_ - 1) % capacity_;

            data_[new_head] = value;
            head_ = new_head;
        }
    }

    void push_front(value_type&& value)
    {
        if (size_ < capacity_)
        {
            const size_type new_head = (head_ + capacity_ - 1) % capacity_;

            allocator_traits::construct(allocator_, std::addressof(data_[new_head]), std::move(value));

            head_ = new_head;
            ++size_;
            return;
        }

        if constexpr (Extendable)
        {
            value_type save = std::move(value);

            double_memory();

            const size_type new_head = (head_ + capacity_ - 1) % capacity_;

            allocator_traits::construct(allocator_, std::addressof(data_[new_head]), std::move(save));

            head_ = new_head;
            ++size_;
        }
        else
        {
            if (capacity_ == 0)
            {
                return;
            }

            const size_type new_head = (head_ + capacity_ - 1) % capacity_;

            value_type save = std::move(value);
            data_[new_head] = std::move(save);
            head_ = new_head;
        }
    }

    void pop_front()
    {
        if (size_ == 0)
        {
            throw std::out_of_range("can`t pop nothing");
        }

        allocator_traits::destroy(allocator_, std::addressof(data_[head_]));
        --size_;
        head_ = (head_ +1) % capacity_;
    }

    // assigns

    void assign(size_type times, const value_type& value)
    {
        circular_buffer tmp(capacity_, allocator_);

        for (size_type ind = 0; ind < times; ++ind)
        {
            tmp.push_back(value);
        }

        clear_memory();
        take_memory(tmp);
    }

    template<std::input_iterator InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        circular_buffer tmp(capacity_, allocator_);

        for (; first != last; ++first)
        {
            tmp.push_back(*first);
        }

        clear_memory();
        take_memory(tmp);
    }

    void assign(std::initializer_list<value_type> list)
    {
        assign(list.begin(), list.end());
    }

    // resize

    void resize(size_type new_size, const value_type& value)
    {
        if constexpr (!Extendable)
        {
            if (new_size > capacity_)
            {
                throw std::length_error("new_size more than capacity");
            }
        }

        const value_type save = value;

        while (new_size < size_)
        {
            pop_back();
        }

        while (new_size > size_)
        {
            push_back(save);
        }
    }

    void resize(size_type new_size)
    {
        if (new_size <= size_)
        {
            while (new_size < size_)
            {
                pop_back();
            }

            return;
        }

        if constexpr (std::is_default_constructible_v<value_type>)
        {
            resize(new_size, value_type{});
        } else
        {
            throw std::logic_error("value_type is`t default constructible");
        } 
    }

    // erase

    iterator erase(const_iterator iter)
    {
        if (iter.container_ != this || iter.position_ < 0 || iter.position_ >= size_)
        {
            throw std::out_of_range("wrong erase position");
        }

        for (size_type ind = iter.position_; ind < size_ - 1; ++ind)
        {
            (*this)[ind] = std::move((*this)[ind + 1]);
        }

        pop_back();

        return iterator(this, iter.position_);
    }

    iterator erase(const_iterator first_iter, const_iterator last_iter)
    {
        if (first_iter.container_ != this || last_iter.container_ != this || first_iter.position_ > last_iter.position_
            || first_iter.position_ < 0 || last_iter.position_ > size_)
        {
            throw std::out_of_range("wrong erase range");
        }

        const size_type count = static_cast<size_type>(last_iter - first_iter);

        for (size_type ind = first_iter.position_; ind < size_ - count; ++ind)
        {
            (*this)[ind] = std::move((*this)[ind + count]);
        }

        for (size_type ind = 0; ind < count; ++ind)
        {
            pop_back();
        }

        return iterator(this, first_iter.position_);
    }

    iterator insert(const_iterator iter, const value_type& value)
    {
        if (iter.container_ != this || iter.position_ < 0 || iter.position_ > size_)
        {
            throw std::out_of_range("wrong insert position");
        }

        value_type save = value;

        if (size_ == capacity_)
        {
            if constexpr (!Extendable)
            {
                throw std::length_error("buffer if full ");
            } else 
            {
                double_memory();
            }
        }

        if (size_ == iter.position_)
        {
            push_back(std::move(save));
            return iterator(this, iter.position_);
        }

        push_back(std::move(back()));

        for (size_type ind = size_ - 2; ind > iter.position_; --ind)
        {
            (*this)[ind] = std::move((*this)[ind - 1]);
        }

        (*this)[iter.position_] = std::move(save);

        return iterator(this, iter.position_);
    }

    iterator insert(const_iterator iter, value_type&& value)
    {
        if (iter.container_ != this || iter.position_ < 0 || iter.position_ > size_)
        {
            throw std::out_of_range("wrong insert position");
        }

        value_type save = value;

        if (size_ == capacity_)
        {
            if constexpr (!Extendable)
            {
                throw std::length_error("buffer if full ");
            } else 
            {
                double_memory();
            }
        }

        if (size_ == iter.position_)
        {
            push_back(std::move(save));
            return iterator(this, iter.position_);
        }

        push_back(std::move(back()));

        for (size_type ind = size_ - 2; ind > iter.position_; --ind)
        {
            (*this)[ind] = std::move((*this)[ind - 1]);
        }

        (*this)[iter.position_] = std::move(save);

        return iterator(this, iter.position_);
    }

    iterator insert(const_iterator iter, size_type cnt, const value_type& value)
    {
        if (iter.container_ != this || iter.position_ < 0 || iter.position_ > size_)
        {
            throw std::out_of_range("wrong insert position");
        }

        if (cnt == 0) return iterator(this, iter.position_);

        const size_type new_size = size_ + cnt;

        if constexpr (!Extendable)
        {   
            if (new_size > capacity_)
            {
                throw std::length_error("buffer is full");
            }
        }

        const value_type save = value;

        circular_buffer tmp(capacity_, allocator_);

        for (size_type ind = 0; ind < iter.position_; ++ind)
        {
            tmp.push_back((*this)[ind]);
        }

        for (size_type ind = 0; ind < cnt; ++ind)
        {
            tmp.push_back(save);
        }

        for (size_type ind = iter.position_; ind < size_; ++ind)
        {
            tmp.push_back((*this)[ind]);
        }

        clear_memory();
        take_memory(tmp);

        return iterator(this, iter.position_);
    }

    template<std::input_iterator InputIterator>
    iterator insert(const_iterator iter, InputIterator first, InputIterator last)
    {
        if (iter.container_ != this || iter.position_ < 0 || iter.position_ > size_)
        {
            throw std::out_of_range("wrong insert position");
        }

        if (first == last) return iterator(this, iter.position_);

        circular_buffer tmp(capacity_, allocator_);

        for (size_type ind = 0; ind < iter.position_; ++ind)
        {
            tmp.push_back((*this)[ind]);
        }

        for (; first != last; ++first)
        {
            tmp.push_back(*first);
        }

        for (size_type ind = iter.position_; ind < size_; ++ind)
        {
            tmp.push_back((*this)[ind]);
        }

        clear_memory();
        take_memory(tmp);

        return iterator(this, iter.position_);
    }

    iterator insert(const_iterator iter, std::initializer_list<value_type> list)
    {
        return insert(iter, list.begin(), list.end());
    }

    void reverse() 
    {
        using std::swap;

        for (size_type i = 0; i < size_ / 2; ++i) 
        {
            swap((*this)[i], (*this)[size_ - i - 1]);
        }
    }

    void reserve(size_type new_capacity) {
        if (new_capacity <= capacity_) 
        {
            return;
        }

        if constexpr (!Extendable) 
        {
            throw std::length_error("can`t reserve memory for non-extendable circular_buffer");
        } else 
        {
            if (new_capacity > max_size()) 
            {
                throw std::length_error("circular_buffer::reserve");
            }

            reallocate(new_capacity);
        }
    }

    void swap(circular_buffer& other) noexcept 
    {
        using std::swap;

        swap(data_, other.data_);
        swap(capacity_, other.capacity_);
        swap(size_, other.size_);
        swap(head_, other.head_);
        
        if constexpr (allocator_traits::propagate_on_container_swap::value) 
        {
            swap(allocator_, other.allocator_);
        }
    }

    friend void swap(circular_buffer& left, circular_buffer& right) noexcept 
    {
        left.swap(right);
    }
};

#include "iterators.h"
