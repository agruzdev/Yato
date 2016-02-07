/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_VIEW_H_
#define _YATO_ARRAY_VIEW_H_

#include <array>
#include <vector>
#include <initializer_list>

#include "array_proxy.h"

namespace yato
{
#ifdef YATO_MSVC
/*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
 *	MSVC complains about type cast otherwise
 */
#pragma warning(push)
#pragma warning(disable:4702) 
#endif
   
    /**
     *	Non-owning light-weight container for contiguous data 
     */
    template<typename _DataType, size_t _DimsNum>
    class array_view_nd
    {
    public:
        using this_type = array_view_nd<_DataType, _DimsNum>;
        using data_type = _DataType;
        static YATO_CONSTEXPR_VAR size_t dimensions_num = _DimsNum;
        static_assert(dimensions_num > 1, "Dimensions number should be greater than 1");

    private:
        using sizes_array = std::array<size_t, dimensions_num>;
        using const_sizes_iterator = typename sizes_array::const_iterator;
        using sub_view = details::sub_array_proxy<data_type*, const_sizes_iterator, dimensions_num - 1>;
        using const_sub_view = details::sub_array_proxy<const data_type*, const_sizes_iterator, dimensions_num - 1>;

    public:
        using data_iterator = data_type*;
        using const_data_iterator = const data_type*;
        using iterator = sub_view;
        using const_iterator = const_sub_view;

        //-------------------------------------------------------
    private:
        const sizes_array m_sizes;
        /*const*/ sizes_array m_sub_array_sizes; // my size, sub array size, sub sub array size, etc
        data_iterator m_base_ptr;

        sub_view _create_sub_view(size_t offset) YATO_NOEXCEPT_KEYWORD
        {
            return sub_view(std::next(m_base_ptr, offset * m_sub_array_sizes[1]), std::next(m_sizes.cbegin()), std::next(m_sub_array_sizes.cbegin()));
        }

        YATO_CONSTEXPR_FUNC
        const_sub_view _create_const_sub_view(size_t offset) const YATO_NOEXCEPT_KEYWORD
        {
            return const_sub_view(std::next(m_base_ptr, offset * m_sub_array_sizes[1]), std::next(m_sizes.cbegin()), std::next(m_sub_array_sizes.cbegin()));
        }
        //-------------------------------------------------------

    public:
        template<typename... _Sizes>
        array_view_nd(not_null<data_type*> ptr, _Sizes && ...sizes) YATO_NOEXCEPT_IN_RELEASE
            : m_sizes({ yato::narrow_cast<size_t>(std::forward<_Sizes>(sizes))... }),
              m_base_ptr(ptr.get())
        { 
            static_assert(sizeof...(_Sizes) > 1, "Should have at least 2 dimensions");
            m_sub_array_sizes[dimensions_num - 1] = m_sizes[dimensions_num - 1];
            for (size_t i = dimensions_num - 1; i > 0; --i) {
                m_sub_array_sizes[i - 1] = m_sizes[i - 1] * m_sub_array_sizes[i];
            }
        }

        array_view_nd(const this_type & other) YATO_NOEXCEPT_KEYWORD
            : m_sizes(other.m_sizes),
              m_sub_array_sizes(other.m_sub_array_sizes),
              m_base_ptr(other.m_base_ptr)
        { }

        array_view_nd& operator=(const this_type & other) YATO_NOEXCEPT_KEYWORD
        {
            if (this != &other) {
                m_sizes = other.m_sizes;
                m_sub_array_sizes = other.m_sub_array_sizes;
                m_base_ptr = other.m_base_ptr;
            }
            return *this;
        }

        array_view_nd(this_type && other) YATO_NOEXCEPT_KEYWORD
            : m_sizes(std::move(other.m_sizes)),
              m_sub_array_sizes(std::move(other.m_sub_array_sizes)),
              m_base_ptr(std::move(other.m_base_ptr))
        { }

        array_view_nd& operator=(this_type && other) YATO_NOEXCEPT_KEYWORD
        {
            if (this != &other) {
                m_sizes = std::move(other.m_sizes);
                m_sub_array_sizes = std::move(other.m_sub_array_sizes);
                m_base_ptr = std::move(other.m_base_ptr);
            }
            return *this;
        }

        ~array_view_nd()
        { }


        YATO_CONSTEXPR_FUNC
        const_sub_view operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return (idx < m_sizes[0])
                ? _create_const_sub_view(idx)
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd: out of range!"), _create_const_sub_view(0));
#else
            return _create_const_sub_view(idx);
#endif
        }

        sub_view operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return (idx < m_sizes[0])
                ? _create_sub_view(idx)
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd: out of range!"), _create_sub_view(idx));
#else
            return _create_sub_view(idx);
#endif
        }

        template<typename... _IdxTail>
        const data_type & at(size_t idx, _IdxTail... tail) const
        {
            if (idx >= m_sizes[0]) {
                YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd: out of range!");
            }
            return (*this)[idx].at(tail...);
        }

        template<typename... _IdxTail>
        data_type & at(size_t idx, _IdxTail... tail)
        {
            if (idx >= m_sizes[0]) {
                YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd: out of range!");
            }
            return (*this)[idx].at(tail...);
        }

        YATO_CONSTEXPR_FUNC
        size_t total_size() const YATO_NOEXCEPT_KEYWORD
        {
            return m_sub_array_sizes[0];
        }

        YATO_CONSTEXPR_FUNC
        size_t size(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return idx < dimensions_num
                ? return m_sizes[idx]
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd[size]: idx out of range!"), m_sizes[0]);
#else
            return m_sizes[idx];
#endif
        }

        YATO_CONSTEXPR_FUNC
        const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return _create_const_sub_view(0);
        }

        iterator begin() YATO_NOEXCEPT_KEYWORD
        {
            return _create_sub_view(0);
        }

        YATO_CONSTEXPR_FUNC
        const_iterator cend() const YATO_NOEXCEPT_KEYWORD
        {
            return _create_const_sub_view(size(0));
        }

        iterator end() YATO_NOEXCEPT_KEYWORD
        {
            return _create_sub_view(size(0));
        }

        YATO_CONSTEXPR_FUNC
        const_data_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        data_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        YATO_CONSTEXPR_FUNC
        const_data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + total_size();
        }

        data_iterator plain_end() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + total_size();
        }

        YATO_CONSTEXPR_FUNC
        yato::range<const_iterator> crange() const YATO_NOEXCEPT_KEYWORD
        {
            return make_range(cbegin(), cend());
        }

        yato::range<iterator> range() YATO_NOEXCEPT_KEYWORD
        {
            return make_range(begin(), end());
        }

        YATO_CONSTEXPR_FUNC
        yato::range<const_data_iterator> plain_crange() const YATO_NOEXCEPT_KEYWORD
        {
            return make_range(plain_cbegin(), plain_cend());
        }

        yato::range<data_iterator> plain_range() YATO_NOEXCEPT_KEYWORD
        {
            return make_range(plain_begin(), plain_end());
        }
    };

    /**
     *	More effective specialization of 1D array view
     */
    template<typename _DataType>
    class array_view_nd<_DataType, 1>
    {
    public:
        using data_type = _DataType;
        using data_iterator = data_type*;
        using const_data_iterator = const data_type*;
        using iterator = data_iterator;
        using const_iterator = const_data_iterator;
        static YATO_CONSTEXPR_VAR size_t dimensions_num = 1;

    private:
        data_type * m_base_ptr;
        const size_t m_size;

    public:
        YATO_CONSTEXPR_FUNC
        array_view_nd(not_null<data_type*> ptr, size_t size) YATO_NOEXCEPT_KEYWORD
            : m_base_ptr(ptr.get()), m_size(size) 
        { }

        YATO_CONSTEXPR_FUNC
        array_view_nd(const array_view_nd & other) YATO_NOEXCEPT_KEYWORD
            : m_base_ptr(other.m_base_ptr), m_size(other.m_size)
        { }

        YATO_CONSTEXPR_FUNC
        array_view_nd(array_view_nd && other) YATO_NOEXCEPT_KEYWORD
            : m_base_ptr(other.m_base_ptr), m_size(other.m_size)
        { }

        array_view_nd& operator=(const array_view_nd & other) YATO_NOEXCEPT_KEYWORD
        {
            if (this != &other) {
                m_base_ptr = std::move(other.m_base_ptr);
                m_size = std::move(other.m_size);
            }
            return *this;
        }
        
        array_view_nd& operator=(array_view_nd && other) YATO_NOEXCEPT_KEYWORD
        {
            if (this != &other) {
                m_base_ptr = other.m_base_ptr;
                m_size = other.m_size;
            }
            return *this;
        }

        ~array_view_nd()
        { }
        
        YATO_CONSTEXPR_FUNC
        const data_type & operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return at(idx);
#else
            return m_base_ptr[idx];
#endif
        }

        data_type & operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return at(idx);
#else
            return m_base_ptr[idx];
#endif
        }

        YATO_CONSTEXPR_FUNC
        const data_type & at(size_t idx) const
        {
            return idx < m_size
                ? m_base_ptr[idx]
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view: index out of range!"), m_base_ptr[idx]);
        }

        data_type & at(size_t idx)
        {
            return idx < m_size
                ? m_base_ptr[idx]
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view: index out of range!"), m_base_ptr[idx]);
        }

        YATO_CONSTEXPR_FUNC
        const data_type * data() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        data_type * data() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        YATO_CONSTEXPR_FUNC
        size_t total_size() const YATO_NOEXCEPT_KEYWORD
        {
            return m_size;
        }

        YATO_CONSTEXPR_FUNC
        size_t size(size_t idx = 0) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return idx < dimensions_num
                ? m_size
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view[size]: idx is out of range!"), m_size);
#else
            (void)idx;
            return m_size;
#endif
        }

        YATO_CONSTEXPR_FUNC
        const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        iterator begin() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        YATO_CONSTEXPR_FUNC
        const_iterator cend() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size;
        }

        iterator end() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size;
        }

        YATO_CONSTEXPR_FUNC
        const_data_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        data_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        YATO_CONSTEXPR_FUNC
        const_data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size;
        }

        data_iterator plain_end() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size;
        }

        YATO_CONSTEXPR_FUNC
        yato::range<const_iterator> crange() const
        {
            return make_range(cbegin(), cend());
        }

        yato::range<iterator> range()
        {
            return make_range(begin(), end());
        }

        YATO_CONSTEXPR_FUNC
        yato::range<const_data_iterator> plain_crange() const
        {
            return make_range(plain_cbegin(), plain_cend());
        }

        yato::range<data_iterator> plain_range()
        {
            return make_range(plain_begin(), plain_end());
        }
    };
#ifdef YATO_MSVC
#pragma warning(pop)
#endif
    template<typename _DataType>
    using array_view = array_view_nd<_DataType, 1>;

    template<typename _T, size_t _Size>
    YATO_CONSTEXPR_FUNC
    array_view<_T> make_view(_T (& arr)[_Size]) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<_T>(arr, _Size);
    }

    template<typename _T, size_t _Size>
    YATO_CONSTEXPR_FUNC
    array_view<_T> make_view(std::array<_T, _Size> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<_T>(arr.data(), _Size);
    }

    template<typename _T, size_t _Size>
    YATO_CONSTEXPR_FUNC
    array_view<const _T> make_view(const std::array<_T, _Size> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<const _T>(arr.data(), _Size);
    }

    template<typename _T>
    YATO_CONSTEXPR_FUNC
    auto make_view(std::vector<_T> & vec) YATO_NOEXCEPT_KEYWORD 
        -> typename std::enable_if<!std::is_same<_T, bool>::value, array_view<_T> >::type
    {
        return array_view<_T>(vec.data(), vec.size());
    }

    template<typename _T>
    YATO_CONSTEXPR_FUNC
    auto make_view(const std::vector<_T> & vec) YATO_NOEXCEPT_KEYWORD
        -> typename std::enable_if<!std::is_same<_T, bool>::value, array_view<const _T> >::type
    {
        return array_view<const _T>(vec.data(), vec.size());
    }

    template<typename _T, typename _Size, typename... _Sizes>
    YATO_CONSTEXPR_FUNC
    auto make_view(_T* ptr, _Size&& size_1, _Sizes && ...sizes) YATO_NOEXCEPT_IN_RELEASE
        -> array_view_nd<_T, sizeof...(_Sizes) + 1>
    {
        return array_view_nd<_T, sizeof...(_Sizes) + 1>(ptr, std::forward<_Size>(size_1), std::forward<_Sizes>(sizes)...);
    }

    template<typename _T, size_t _Size1, size_t _Size2>
    YATO_CONSTEXPR_FUNC
    array_view_nd<_T, 2> make_view(_T(&arr)[_Size1][_Size2]) YATO_NOEXCEPT_IN_RELEASE
    {
        return array_view_nd<_T, 2>(&arr[0][0], _Size1, _Size2);
    }

    template<typename _T, size_t _Size1, size_t _Size2, size_t _Size3>
    YATO_CONSTEXPR_FUNC
        array_view_nd<_T, 3> make_view(_T(&arr)[_Size1][_Size2][_Size3]) YATO_NOEXCEPT_IN_RELEASE
    {
        return array_view_nd<_T, 3>(&arr[0][0][0], _Size1, _Size2, _Size3);
    }

}

#endif
