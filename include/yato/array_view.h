/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_VIEW_H_
#define _YATO_ARRAY_VIEW_H_

#include <array>
#include "assert.h"
#include "not_null.h"

namespace yato
{

    template<typename _DataType>
    class array_view
    {
    public:
        using data_type = _DataType;

    private:
        data_type * m_base_ptr;
        const size_t m_size;

    public:
        YATO_CONSTEXPR_FUNC
        array_view(not_null<data_type*> ptr, size_t size) YATO_NOEXCEPT_KEYWORD
            : m_base_ptr(ptr.get()), m_size(size) 
        { }
        
        YATO_CONSTEXPR_FUNC
        const data_type & operator[](size_t idx) const
        {
#if YATO_DEBUG
            return at(idx);
#else
            return m_base_ptr[idx];
#endif
        }

        data_type & operator[](size_t idx)
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
}


#endif