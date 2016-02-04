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
#include "range.h"

namespace yato
{
#ifdef YATO_MSVC
/*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
 *	MSVC complains about type cast otherwise
 */
#pragma warning(push)
#pragma warning(disable:4702) 
#endif
    namespace details
    {
        
        template<typename _DataIterator, typename _SizeIterator, size_t _DimsNum>
        struct sub_array_proxy
        {
            using size_iterator = _SizeIterator;
            using data_iterator = _DataIterator;
            using const_data_iterator = const data_iterator;
            using reference = decltype(*std::declval<data_iterator>());

            static YATO_CONSTEXPR_VAR size_t dimensions_num = _DimsNum;

            using sub_view = sub_array_proxy<data_iterator, size_iterator, dimensions_num - 1>;

            data_iterator m_data_iter;
            size_iterator m_sizes_iter;
            size_iterator m_offsets_iter;

        public:
            YATO_CONSTEXPR_FUNC
            sub_array_proxy(const data_iterator & data, const size_iterator & sizes, const size_iterator & offsets) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(data), m_sizes_iter(sizes), m_offsets_iter(offsets)
            { }

            sub_array_proxy(const sub_array_proxy&) = default;

            sub_array_proxy(sub_array_proxy && other) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(std::move(other.m_data_iter)), m_sizes_iter(std::move(other.m_sizes_iter)), m_offsets_iter(std::move(other.m_offsets_iter))
            { }

            sub_array_proxy & operator= (const sub_array_proxy&) = default;

            sub_array_proxy & operator= (sub_array_proxy && other) YATO_NOEXCEPT_KEYWORD
            {
                if (this != &other) {
                    m_data_iter = std::move(other.m_data_iter);
                    m_sizes_iter = std::move(other.m_sizes_iter);
                    m_offsets_iter = std::move(other.m_offsets_iter);
                }
                return *this;
            }

            ~sub_array_proxy()
            { }

            template<size_t _Dims = dimensions_num>
            YATO_CONSTEXPR_FUNC
            auto operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if<(_Dims > 1), sub_view>::type
            {
#if YATO_DEBUG
                return (idx < *m_sizes_iter) 
                    ? sub_view{ m_data_iter + idx * (*std::next(m_offsets_iter)), std::next(m_sizes_iter), std::next(m_offsets_iter) }
                    : (YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!"), sub_view{ m_data_iter , m_sizes_iter, m_offsets_iter });
#else
                return sub_view{
                    m_data_iter + idx * (*std::next(m_offsets_iter)),
                    std::next(m_sizes_iter), 
                    std::next(m_offsets_iter) };
#endif
            }


            template<size_t _Dims = dimensions_num>
            YATO_CONSTEXPR_FUNC
            auto operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if <(_Dims == 1), const reference>::type
            {
#if YATO_DEBUG
                return (idx < *m_sizes_iter)
                    ? *(m_data_iter + idx) :
                    (YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!"), *(m_data_iter));
#else
                return *(m_data_iter + idx);
#endif
            }

            template<size_t _Dims = dimensions_num>
            auto operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if < _Dims == 1, reference>::type
            {
#if YATO_DEBUG
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
#endif
                return *(m_data_iter + idx);
            }


            template<size_t _Dims = dimensions_num, typename... _IdxTail>
            auto at(size_t idx, _IdxTail... tail) const
                -> typename std::enable_if < (_Dims > 1), const reference>::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx].at(tail...);
            }

            template<size_t _Dims = dimensions_num, typename... _IdxTail>
            auto at(size_t idx, _IdxTail... tail)
                -> typename std::enable_if < (_Dims > 1), reference> ::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx].at(tail...);
            }

            template<size_t _Dims = dimensions_num>
            auto at(size_t idx) const 
                -> typename std::enable_if < _Dims == 1, const reference>::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return *(m_data_iter + idx);
            }

            template<size_t _Dims = dimensions_num>
            auto at(size_t idx) 
                -> typename std::enable_if < _Dims == 1, reference>::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return *(m_data_iter + idx);
            }

            YATO_CONSTEXPR_FUNC
            size_t size() const
            {
                return *m_offsets_iter;
            }

            YATO_CONSTEXPR_FUNC
            const_data_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter;
            }

            data_iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter;
            }

            YATO_CONSTEXPR_FUNC
            const_data_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter + size();
            }

            data_iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter + size();
            }

            YATO_CONSTEXPR_FUNC
            range<const_data_iterator> crange() const YATO_NOEXCEPT_KEYWORD
            {
                return make_range(cbegin(), cend());
            }

            range<data_iterator> range() YATO_NOEXCEPT_KEYWORD
            {
                return make_range(begin(), end());
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
        using sub_view = details::sub_array_proxy<data_type*, typename sizes_array::const_iterator, dimensions_num - 1>;
        using const_sub_view = details::sub_array_proxy<const data_type*, typename sizes_array::const_iterator, dimensions_num - 1>;

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
                ? sub_view{ m_base_ptr + idx * m_sub_array_sizes[1], std::next(std::begin(m_sizes)), std::next(std::begin(m_sub_array_sizes)) }
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd: out of range!"), sub_view{ m_base_ptr , std::begin(m_sizes) , std::begin(m_sub_array_sizes) });
#else
            return sub_view{ 
                m_base_ptr + idx * m_sub_array_sizes[1], 
                std::next(std::begin(m_sizes)), 
                std::next(std::begin(m_sub_array_sizes)) };
#endif
        }

        sub_view operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return (idx < m_sizes[0])
                ? sub_view{ m_base_ptr + idx * m_sub_array_sizes[1], std::next(std::begin(m_sizes)), std::next(std::begin(m_sub_array_sizes)) }
            : (YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd: out of range!"), sub_view{ m_base_ptr , std::begin(m_sizes) , std::begin(m_sub_array_sizes) });
#else
            return sub_view{
                m_base_ptr + idx * m_sub_array_sizes[1],
                std::next(std::begin(m_sizes)),
                std::next(std::begin(m_sub_array_sizes)) };
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
        size_t size() const YATO_NOEXCEPT_KEYWORD
        {
            return m_sub_array_sizes[0];
        }

        YATO_CONSTEXPR_FUNC
        const data_type* cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        data_type* begin() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        YATO_CONSTEXPR_FUNC
        const data_type* cend() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + size();
        }

        data_type* end() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + size();
        }

        YATO_CONSTEXPR_FUNC
        range<const data_type*> crange() const YATO_NOEXCEPT_KEYWORD
        {
            return make_range(cbegin(), cend());
        }

        range<data_type*> range() YATO_NOEXCEPT_KEYWORD
        {
            return make_range(begin(), end());
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

        YATO_CONSTEXPR_FUNC
        range<const data_type*> crange() const
        {
            return make_range(cbegin(), cend());
        }

        range<data_type*> range()
        {
            return make_range(begin(), end());
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