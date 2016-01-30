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
#include "assert.h"
#include "not_null.h"
#include "types.h"

namespace yato
{

#pragma warning(push)
/*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
 *	MSVC complains about type cast otherwise
 */
#pragma warning(disable:4702) 

    namespace details
    {
        
        template<typename _SizeIterator, typename _DataType, size_t _DimsNum>
        struct array_sub_view_nd
        {
            using size_iterator = _SizeIterator;
            using data_type = _DataType;
            using data_iterator = data_type*;

            static YATO_CONSTEXPR_VAR size_t dimensions_num = _DimsNum;

            using sub_view = array_sub_view_nd<size_iterator, data_type, dimensions_num - 1>;

            data_iterator m_base_ptr;
            size_iterator m_sizes_iter;
            size_iterator m_offsets_iter;

        public:
            YATO_CONSTEXPR_FUNC
            array_sub_view_nd(const data_iterator & data, const size_iterator & sizes, const size_iterator & offsets) YATO_NOEXCEPT_KEYWORD
                : m_base_ptr(data), m_sizes_iter(sizes), m_offsets_iter(offsets)
            { }

            template<size_t _Dims = dimensions_num>
            YATO_CONSTEXPR_FUNC
            auto operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if<(_Dims > 1), sub_view>::type
            {
                return sub_view{
                    m_base_ptr + idx * (*std::next(m_offsets_iter)), 
                    std::next(m_sizes_iter), 
                    std::next(m_offsets_iter) };
            }


            template<size_t _Dims = dimensions_num>
            YATO_CONSTEXPR_FUNC
            auto operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if <(_Dims == 1), const data_type &>::type
            {
                return *(m_base_ptr + idx);
            }

            template<size_t _Dims = dimensions_num>
            auto operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if < _Dims == 1, data_type &>::type
            {
                return *(m_base_ptr + idx);
            }

            YATO_CONSTEXPR_FUNC
            size_t size() const
            {
                return *m_offsets_iter;
            }
        };

    }

    /**
     *	Non-owning light-weight container for contiguous data 
     */
    template<typename _DataType, size_t _DimsNum>
    class array_view_nd
    {
        static_assert(_DimsNum > 1, "Dimensions number should be greater than 1");
    public:
        using this_type = array_view_nd<_DataType, _DimsNum>;
        using data_type = _DataType;
        static YATO_CONSTEXPR_VAR size_t dimensions_num = _DimsNum;

    private:
        using sizes_array = std::array<size_t, dimensions_num>;
        using sub_view = details::array_sub_view_nd<typename sizes_array::const_iterator, data_type, dimensions_num - 1>;

        const sizes_array m_sizes;
        /*const*/ sizes_array m_sub_array_sizes; // my size, sub array size, sub sub array size, etc
        data_type* m_base_ptr;

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

        array_view_nd(const this_type &) = default;
        array_view_nd(this_type &&) = default;

        array_view_nd& operator=(const this_type &) = default;
        array_view_nd& operator=(this_type &&) = default;

        ~array_view_nd()
        { }


        YATO_CONSTEXPR_FUNC
        sub_view operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
        {
            return sub_view{ 
                m_base_ptr + idx * m_sub_array_sizes[1], 
                std::next(std::begin(m_sizes)), 
                std::next(std::begin(m_sub_array_sizes)) };
        }

        YATO_CONSTEXPR_FUNC
        size_t size() const
        {
            return m_sub_array_sizes[0];
        }
    };


    template<typename _DataType>
    class array_view_nd<_DataType, 1>
    {
    public:
        using data_type = _DataType;

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

        array_view_nd& operator=(const array_view_nd & other) = delete;
        array_view_nd& operator=(array_view_nd && other) = delete;

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
        size_t size() const YATO_NOEXCEPT_KEYWORD
        {
            return m_size;
        }

        YATO_CONSTEXPR_FUNC
        const data_type * cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        data_type * begin() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        YATO_CONSTEXPR_FUNC
        const data_type * cend() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size;
        }

        data_type * end() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size;
        }
    };
#pragma warning(pop)

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