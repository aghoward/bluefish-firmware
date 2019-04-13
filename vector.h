#pragma once

#include <stdio.h>
#include <type_traits.h>

template <typename T>
class vector
{
    public:
        typedef T value_type;
        typedef unsigned int size_type;
        typedef std::add_lvalue_reference_t<T> reference;
        typedef std::add_const_t<reference> const_reference;
        typedef T* iterator;
        typedef std::add_const_t<iterator> const_iterator;

    private:
        T* _data;
        size_type _size;
        size_type _capacity;

        void copy(iterator from_begin, iterator from_end, iterator to_begin)
        {
            iterator out = to_begin;
            for (auto it = from_begin; it != from_end; it++)
                *out = std::move(*it);
        }

    public:
        vector()
            : _data(nullptr),
              _size(0u),
              _capacity(0u)
        {}

        vector(size_type size)
        {
            _data = new T[size];
            _size = 0u;
            _capacity = size;
        }

        vector(vector&& other)
        {
            auto* tmp_data = _data;
            _data = other._data;
            _size = other._size;
            _capacity = other._capacity;
            other._data = tmp_data;
        }

        vector& operator=(vector&& other)
        {
            auto* tmp_data = _data;
            _data = other._data;
            _size = other._size;
            _capacity = other._capacity;
            other._data = tmp_data;
            return *this;
        }

        ~vector()
        {
            if (_data != nullptr)
                delete[] _data;
        }

        reference operator[](size_type position)
        {
            return *(_data + position);
        }

        const_reference operator[](size_type position) const
        {
            return *(_data + position);
        }

        iterator begin()
        {
            return _data;
        }

        const_iterator begin() const
        {
            return _data;
        }

        const_iterator cbegin() const
        {
            return _data;
        }

        iterator end()
        {
            return (_size > 0) ? (_data + _size) : (_data + 1);
        }

        const_iterator end() const
        {
            return (_size > 0) ? (_data + _size) : (_data + 1);
        }

        const_iterator cend() const
        {
            return (_size > 0) ? (_data + _size) : (_data + 1);
        }

        bool empty() const
        {
            return _size == 0;
        }

        size_type size() const
        {
            return _size;
        }

        size_type capacity() const
        {
            return _capacity;
        }

        void reserve(size_type size)
        {
            if (_capacity >= size)
                return;
            auto* new_data = new T[size];
            copy(begin(), end(), new_data);
            delete[] _data;
            _data = new_data;
            _capacity = size;
        }

        void clear()
        {
            delete[] _data;
            _data = nullptr;
            _size = 0u;
            _capacity = 0u;
        }

        iterator insert(iterator position, T&& value)
        {
            iterator item = position - 1;
            *item = std::move(value);
            return item;
        }

        template <typename U = T, typename = std::enable_if_t<std::is_copy_constructible_v<U>>>
        iterator insert(iterator position, const T& value)
        {
            iterator item = position - 1;
            *item = value;
            return item;
        }

        template <typename ... TArgs>
        iterator emplace(iterator position, TArgs&&... args)
        {
            iterator item = position - 1;
            *item = T(std::move(args)...);
            return item;
        }

        void push_back(T&& value)
        {
            if (++_size >= _capacity)
                reserve(_size);
            insert(end(), std::move(value));
        }

        template <typename U = T, typename = std::enable_if_t<std::is_copy_constructible_v<U>>>
        void push_back(const T& value)
        {
            if (++_size >= _capacity)
                reserve(_size);
            insert(end(), value);
        }

        template <typename ... TArgs>
        reference emplace_back(TArgs&&... args)
        {
            push_back(T(std::forward<TArgs...>(args...)));
            return *end();
        }

        void pop_back()
        {
            _size--;
        }

        void resize(size_type count)
        {
            if (_size == count)
                return;
            
            auto* new_data = new T[count];
            if (_size > count)
                copy(begin(), begin() + count, new_data);
            if (_size < count)
                copy(begin(), end(), new_data);
           
            delete[] _data;
            _data = new_data;
           
            _capacity = count;
            if (_size > count)
                _size = count;
        }
};
