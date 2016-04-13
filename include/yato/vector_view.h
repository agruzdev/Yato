/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_VECTOR_VIEW_H_
#define _YATO_VECTOR_VIEW_H_

#include <utility>
#include <iterator>
#include <algorithm>
#include "types.h"

namespace yato
{
    template <typename _IteratorType>
    class vector_view
    {
        using my_type = vector_view<_IteratorType>;

        using iterator = _IteratorType;
        using value_type = typename std::iterator_traits<iterator>::value_type;
        using size_type = std::size_t;
        using difference_type = typename std::iterator_traits<iterator>::difference_type;
        using reference = typename std::iterator_traits<iterator>::reference;
        using pointer = typename std::iterator_traits<iterator>::pointer;
        using const_reference = const reference;
        using const_pointer = const pointer;
        using const_iterator = const iterator;
        //-------------------------------------------------------

    private:
        iterator m_begin;
        iterator m_end;
        size_type m_max_size;
        //-------------------------------------------------------

        void _resize(size_type count)
        {
#if YATO_DEBUG
            if (count > m_max_size) {
                throw yato::out_of_range_error("vector_view[_resize]: required size is bigger than the maximal size of the view");
            }
#endif
            m_end = std::next(m_begin, count);
        }

        iterator _prepare_insert(const_iterator c_pos, size_type count)
        {
#if YATO_DEBUG
            if (std::distance(cbegin(), c_pos) > yato::narrow_cast<difference_type>(size())) {
                throw yato::assertion_error("yato::vectr_view[_prepare_insert]: bad position!");
            }
#endif
            const size_type old_size{ size() };
            const iterator pos{ std::next(begin(), std::distance(cbegin(), c_pos)) };
            if (count > 0) {
                _resize(old_size + count);
                std::move(pos, end(), std::next(pos, count));
            }
            return pos;
        }

        iterator _erase_n(const_iterator c_pos, size_type count)
        {
#if YATO_DEBUG
            if (std::distance(cbegin(), c_pos) > yato::narrow_cast<difference_type>(size())) {
                throw yato::assertion_error("yato::vectr_view[_erase_n]: bad position!");
            }
#endif
            const iterator pos{ std::next(begin(), std::distance(cbegin(), c_pos)) };
            if (count > 0) {
                std::move_backward(std::next(pos, count), end(), std::prev(end(), count));
                _resize(size() - count);
            }
            return pos;
        }

        //-------------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        vector_view(iterator data, size_type max_size)
            : m_begin(data), m_end(data), m_max_size(max_size)
        { }

        YATO_CONSTEXPR_FUNC
        vector_view(iterator begin, iterator end, size_type max_size)
            : m_begin(begin), m_end(end), m_max_size(max_size)
        { }

        YATO_CONSTEXPR_FUNC
        vector_view(const my_type & other)
            : m_begin(other.m_begin), m_end(other.m_end), m_max_size(other.m_max_size)
        { }

        template <typename _AnotherIterator>
        YATO_CONSTEXPR_FUNC
        vector_view(const vector_view<_AnotherIterator> & other)
            : m_begin(other.m_begin), m_end(other.m_end), m_max_size(other.m_max_size)
        { }

        YATO_CONSTEXPR_FUNC
        vector_view(my_type && other)
            : m_begin(std::move(other.m_begin)), m_end(std::move(other.m_end)), m_max_size(std::move(other.m_max_size))
        { }

        ~vector_view()
        { }

        void swap(my_type & other) YATO_NOEXCEPT_KEYWORD
        {
            if (this != &other) {
                using std::swap;
                swap(m_begin, other.m_begin);
                swap(m_end, other.m_end);
                swap(m_max_size, other.m_max_size);
            }
        }

        my_type & operator = (const my_type & other)
        {
            if (this != &other) {
                my_type tmp{ other };
                tmp.swap(*this);
            }
            return *this;
        }

        template <typename _AnotherIterator>
        my_type & operator = (const vector_view<_AnotherIterator> & other)
        {
            my_type tmp{ other };
            tmp.swap(*this);
            return *this;
        }

        my_type & operator = (my_type && other)
        {
            if (this != &other) {
                m_begin = std::move(other.m_begin);
                m_end = std::move(other.m_end);
                m_max_size = std::move(other.m_max_size);
            }
            return *this;
        }

        /**
         *  Get maximal available size
         */
        YATO_CONSTEXPR_FUNC
        size_type max_size() const
        {
            return m_max_size;
        }

        /**
         *  Same to max_size
         */
        size_type capacity() const
        {
            return m_max_size;
        }

        YATO_CONSTEXPR_FUNC
        size_type size() const
        {
            return std::distance(m_begin, m_end);
        }

        YATO_CONSTEXPR_FUNC
        bool empty() const
        {
            return m_begin == m_end;
        }

        /**
         *  Resizes vector_view to the first count elements and assigns all values
         */
        void assign(size_type count, const value_type & value)
        {
            _resize(count);
            std::fill(m_begin, m_end, value);
        }

        /**
         *  Resizes vector_view to the first count elements
         */
        void resize(size_type count)
        {
            if (count != size()) {
                _resize(count);
            }
        }

        /**
         *  Resizes vector_view to the first count elements and assigns new values 
         */
        void resize(size_type count, const value_type & value)
        {
            if (count < size()) {
                _resize(count);
            }
            else if (count > size()) {
                iterator prev{ m_end };
                _resize(count);
                if (std::distance(prev, m_end) > 0) {
                    std::fill(prev, m_end, value);
                }
            }
        }

        YATO_CONSTEXPR_FUNC
        const_iterator cbegin() const
        {
            return m_begin;
        }

        iterator begin()
        {
            return m_begin;
        }

        YATO_CONSTEXPR_FUNC
        const_iterator cend() const
        {
            return m_end;
        }

        iterator end()
        {
            return m_end;
        }

        YATO_CONSTEXPR_FUNC
        const_reference operator[] (size_type idx) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return (idx >= size())
                ? (throw yato::out_of_range_error("vector_view[operator[]]: out of range"), *m_begin)
                : *std::next(m_begin, idx);
#else
            return *std::next(m_begin, idx);
#endif
        }

        reference operator[] (size_type idx) YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            if (idx >= size()) {
                throw yato::out_of_range_error("vector_view[operator[]]: out of range");
            }
#endif
            return *std::next(m_begin, idx);
        }

        YATO_CONSTEXPR_FUNC
        const_reference at(size_type idx) const
        {
            return (idx >= size()) 
                ? (throw yato::out_of_range_error("vector_view[at]: out of range"), *m_begin)
                : (*this)[idx];
        }

        reference at(size_type idx)
        {
            if (idx >= size()) {
                throw yato::out_of_range_error("vector_view[at]: out of range");
            }
            return (*this)[idx];
        }

        reference front()
        {
            return *begin();
        }

        YATO_CONSTEXPR_FUNC
        const_reference front() const
        {
            return const_cast<my_type*>(this)->front();
        }

        reference back()
        {
            return *std::prev(end());
        }

        YATO_CONSTEXPR_FUNC
        const_reference back() const
        {
            return const_cast<my_type*>(this)->back();
        }

        /**
         *  Make view empty. Doesn't change data
         */
        void clear()
        {
            m_end = m_begin;
        }

        void push_back(const value_type & val)
        {
#if YATO_DEBUG
            if (size() >= max_size()) {
                throw yato::out_of_range_error("yato::vector_view[push_back]: max size is reached!");
            }
#endif
            *m_end++ = val;
        }

        void push_back(value_type && val)
        {
#if YATO_DEBUG
            if (size() >= max_size()) {
                throw yato::out_of_range_error("yato::vector_view[push_back]: max size is reached!");
            }
#endif
            *m_end++ = std::move(val);
        }

        /**
         *  Reduce view's size, doesn't change values
         */
        void pop_back()
        {
#if YATO_DEBUG
            if (empty()) {
                throw yato::out_of_range_error("yato::vector_view[pop_back]: view is empty!");
            }
#endif
            --m_end;
        }

        /**
         *  Insert value to the position
         *  If pos doesn't belong the range [cbegin, cend) then behavior is undefined
         */
        iterator insert(const_iterator pos, const value_type & val)
        {
            iterator it{ _prepare_insert(pos, 1) };
            *it = val;
            return it;
        }

        /**
         *  Insert value to the position
         *  If pos doesn't belong the range [cbegin, cend) then behavior is undefined
         */
        iterator insert(const_iterator pos, value_type && val)
        {
            iterator it{ _prepare_insert(pos, 1) };
            *it = std::move(val);
            return it;
        }

        /**
         *  Insert value to the position
         *  If pos doesn't belong the range [cbegin, cend) then behavior is undefined
         */
        template <class _InputIterator>
        iterator insert(const_iterator pos, _InputIterator first, _InputIterator last)
        {
            const typename std::iterator_traits<_InputIterator>::difference_type count{ std::distance(first, last) };
#if YATO_DEBUG
            if (count < 0) {
                throw yato::assertion_error("yato::vector_view[insert]: bad pair of the first and last iterators!");
            }
#endif
            iterator it{ _prepare_insert(pos, count) };
            std::copy(first, last, it);
            return it;
        }

        /**
         *  Insert value to the position
         *  If pos doesn't belong the range [cbegin, cend) then behavior is undefined
         */
        iterator insert(const_iterator pos, std::initializer_list<value_type> ilist)
        {
            return insert(pos, ilist.begin(), ilist.end());
        }

        /**
         *  Erase element from the position 
         *  If pos doesn't belong the range [cbegin, cend) then behavior is undefined
         */
        iterator erase(const_iterator pos)
        {
#if YATO_DEBUG
            if (empty()) {
                throw yato::assertion_error("yato::vector_view[insert]: view is empty!");
            }
#endif
            return _erase_n(pos, 1);
        }

        /**
         *  Erase interval [first, last) of elements
         *  If first or last doesn't belong the range [cbegin, cend) then behavior is undefined
         */
        iterator erase(const_iterator first, const_iterator last)
        {
            const auto count{ std::distance(first, last) };
#if YATO_DEBUG
            if (count < 0 || count > yato::narrow_cast<decltype(count)>(size())) {
                throw yato::assertion_error("yato::vector_view[insert]: bad pair of the first and last iterators!");
            }
#endif
            return _erase_n(first, count);
        }

        //-------------------------------------------------------
        template <typename _SomeIterator>
        friend class vector_view;
    };

}

#endif
