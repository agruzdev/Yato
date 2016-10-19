/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_DYNAMIC_ARRAY_H_
#define _YATO_DYNAMIC_ARRAY_H_

#include <utility>
#include <limits>

#include "type_traits.h"
#include "assert.h"

namespace yato
{

    /**
     *  Container for types without copy- and move-constructors, which can't be hold by std::vector
     */
    template <typename _T, typename _Alloc = std::allocator<_T>>
    class dynamic_array
    {
    public:
        using my_type = dynamic_array<_T, _Alloc>;
        using value_type = _T;
        using allocator_type = _Alloc;
        using reference = _T&;
        using const_reference = const _T&;
        using pointer = _T*;
        using const_pointer = const _T*;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        //-------------------------------------------------------

    private:
        allocator_type m_allocator;
        size_type m_size = static_cast<size_type>(0);
        pointer m_data = nullptr;
        //-------------------------------------------------------

        void _allocate(size_type size)
        {
            YATO_ASSERT(m_data == nullptr, "dynamic_array[_allocate]: Previous memory was not deallocated!");
            m_size = size;
            m_data = m_allocator.allocate(m_size);
            YATO_ASSERT(m_data != nullptr, "dynamic_array[_allocate]: failed to allocate memory!");
        }

        template <typename... _Args>
        void _construct(_Args && ... args)
        {
            for (pointer ptr = m_data; ptr != m_data + m_size; ++ptr) {
                m_allocator.construct(ptr, std::forward<_Args>(args)...);
            }
        }

        template <typename _Iter>
        void _copy_construct(_Iter src)
        {
            for (pointer ptr = m_data; ptr != m_data + m_size; ++ptr, ++src) {
                m_allocator.construct(ptr, *src);
            }
        }

        void _destroy() YATO_NOEXCEPT_KEYWORD
        {
            if (m_data != nullptr) {
                for (pointer ptr = m_data + m_size - 1; ptr != m_data - 1; --ptr) {
                    m_allocator.destroy(ptr);
                }
                m_allocator.deallocate(m_data, m_size);
                m_data = nullptr;
                m_size = 0;
            }
        }

YATO_PRAGMA_WARNING_PUSH
YATO_MSCV_WARNING_IGNORE(4127) // Condition expression is constant
        void _move_assign(my_type && other)
        {
            if (allocator_type::propagate_on_container_move_assignment::value || (m_allocator == other.m_allocator)) {
                m_allocator = other.m_allocator;
                m_size = std::move(other.m_size);
                m_data = std::move(other.m_data);
                other.m_size = 0;
                other.m_data = nullptr;
            }
            else {
                throw yato::runtime_error("dynamic_array[move]: Allocator can't be propagated");
            }
        }
YATO_PRAGMA_WARNING_POP

    public:
        explicit dynamic_array(const allocator_type & allocator)
            : m_allocator(allocator)
        { }

        dynamic_array()
            : dynamic_array(allocator_type())
        { }

        explicit dynamic_array(size_type size, const allocator_type & allocator = allocator_type())
            : m_allocator(allocator)
        {
            _allocate(size);
            _construct();
        }

        dynamic_array(size_type size, const_reference value, const allocator_type & allocator = allocator_type())
            : m_allocator(allocator)
        {
            _allocate(size);
            _construct(value);
        }

        dynamic_array(std::initializer_list<value_type> init, const allocator_type & allocator = allocator_type())
            : m_allocator(allocator)
        {
            _allocate(init.size());
            _copy_construct(init.begin());
        }

        ~dynamic_array()
        {
            _destroy();
        }

        dynamic_array(const my_type &) = delete;

        dynamic_array(my_type && other)
        {
            _move_assign(std::move(other));
        }

        template <typename _MyAlloc = allocator_type>
        void swap(typename std::enable_if<_MyAlloc::propagate_on_container_swap::value, my_type &>::type other) YATO_NOEXCEPT_KEYWORD
        {
            if (this != &other) {
                using std::swap;
                std::swap(m_allocator, other.m_allocator);
                std::swap(m_size, other.m_size);
                std::swap(m_data, other.m_data);
            }
        }
        
        my_type & operator=(const my_type &) = delete;

        my_type & operator=(my_type && other)
        {
            if (this != &other) {
                _move_assign(std::move(other));
            }
            return *this;
        }

        my_type & operator=(std::initializer_list<value_type> init)
        {
            _destroy();
            _allocate(init.size());
            _copy_construct(init.begin());
        }

        reference operator[](size_type idx)
        {
            YATO_REQUIRES(idx >= 0 && idx < m_size);
            YATO_REQUIRES(m_data != nullptr);
            return m_data[idx];
        }

        const_reference operator[](size_type idx) const 
        {
            return const_cast<my_type*>(this)->operator[](idx);
        }

        reference at(size_type idx)
        {
            if (!(idx >= 0 && idx < m_size)) {
                throw yato::out_of_range_error("dynamic_array[at]: out of range");
            }
            return (*this)[idx];
        }

        const_reference at(size_type idx) const
        {
            if (!(idx >= 0 && idx < m_size)) {
                throw yato::out_of_range_error("dynamic_array[at]: out of range");
            }
            return (*this)[idx];
        }

        allocator_type get_allocator() const
        {
            return m_allocator;
        }

        size_type size() const
        {
            return m_size;
        }

        size_type max_size() const
        {
            return std::numeric_limits<size_type>::max();
        }

        bool empty() const
        {
            return (0 == size());
        }

        iterator begin()
        {
            return m_data;
        }

        const_iterator cbegin() const
        {
            return m_data;
        }

        iterator end()
        {
            return m_data + m_size;
        }

        const_iterator cend()
        {
            return m_data + m_size;
        }

        reference front()
        {
            YATO_REQUIRES(!empty());
            YATO_REQUIRES(m_data != nullptr);
            return *begin();
        }

        const_reference front() const
        {
            return const_cast<my_type*>(this)->front();
        }

        reference back()
        {
            YATO_REQUIRES(!empty());
            YATO_REQUIRES(m_data != nullptr);
            return *(m_data + m_size - 1);
        }

        const_reference back() const
        {
            return const_cast<my_type*>(this)->back();
        }

        /**
         *  Destroy values and deallocate memory
         */
        void clear()
        {
            _destroy();
        }

        /**
         *  Destroy old values and memory, allocate new array and init with value
         */
        void assign(size_type size, const_reference value)
        {
            clear();
            _allocate(size);
            _construct(value);
        }

        /**
         *  Destroy old values and memory, allocate new array and init with value
         */
        void assign(size_type size, std::initializer_list<value_type> init)
        {
            clear();
            _allocate(size);
            _construct(init.begin());
        }

        /**
         *  Destroy the old array and create a new array with values constructed by default
         */
        void resize(size_type size)
        {
            clear();
            _allocate(size);
            _construct();
        }
    };

    template <typename _T, typename _Alloc>
    inline
    auto swap(dynamic_array<_T, _Alloc> & one, dynamic_array<_T, _Alloc> & another)
        -> typename std::enable_if<_Alloc::propagate_on_container_swap::value, void>::type
    {
        one.swap(another);
    }

}

#endif
