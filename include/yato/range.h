/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_RANGE_H_
#define _YATO_RANGE_H_

#include "numeric_iterator.h"

namespace yato
{
    /**
     *    An immutable object aggregating two iterators
     *  Helps to express a range of one container
     */
    template<typename IteratorType>
    class range
    {
        static_assert(is_iterator<IteratorType>::value, "yato::range can be used only for iterators");

        const IteratorType m_begin;
        const IteratorType m_end;
    public:
        YATO_CONSTEXPR_FUNC 
        range(const IteratorType & begin, const IteratorType & end) YATO_NOEXCEPT_KEYWORD
            : m_begin(begin), m_end(end)
        { }

        YATO_CONSTEXPR_FUNC 
        range(IteratorType && begin, IteratorType && end) YATO_NOEXCEPT_KEYWORD
            : m_begin(std::move(begin)), m_end(std::move(end))
        { }

        YATO_CONSTEXPR_FUNC 
        range(const range<IteratorType>& other) YATO_NOEXCEPT_KEYWORD
            : m_begin(other._begin), m_end(other._end)
        { }

        range(range<IteratorType> && other) YATO_NOEXCEPT_KEYWORD
            : m_begin(std::move(other.m_begin)), m_end(std::move(other.m_end))
        { }

        range<IteratorType>& operator=(const range<IteratorType> & other) YATO_NOEXCEPT_KEYWORD
        {
            m_begin = other.m_begin;
            m_end = other.m_end;
            return *this;
        }

        range<IteratorType>& operator=(range<IteratorType> && other) YATO_NOEXCEPT_KEYWORD
        {
            m_begin = std::move(other.m_begin);
            m_end = std::move(other.m_end);
            return *this;
        }

        ~range() 
        { }

        YATO_CONSTEXPR_FUNC 
        const IteratorType & begin() const YATO_NOEXCEPT_KEYWORD 
        {
            return m_begin;
        }

        YATO_CONSTEXPR_FUNC 
        const IteratorType & end() const YATO_NOEXCEPT_KEYWORD
        {
            return m_end;
        }

        YATO_CONSTEXPR_FUNC 
        typename IteratorType::difference_type size() const
        {
            return std::distance(m_begin, m_end);
        }

        YATO_CONSTEXPR_FUNC 
        bool empty() const 
        {
            return !(m_begin != m_end);
        }

        YATO_CONSTEXPR_FUNC 
        const IteratorType & head() const YATO_NOEXCEPT_KEYWORD
        {
            return m_begin;
        }

        YATO_CONSTEXPR_FUNC 
        range<IteratorType> tail() const
        {
            return range<IteratorType>(std::next(m_begin), m_end);
        }
    };

    /**
    *    Helper functions to make range from a couple of iterators with auto type deduction 
    */
    template<typename IteratorType>
    YATO_CONSTEXPR_FUNC 
    typename std::enable_if< is_iterator< typename std::decay<IteratorType>::type >::value, range< typename std::decay<IteratorType>::type > >::type
        make_range(IteratorType && begin, IteratorType && end)
    {
        return range<typename std::decay<IteratorType>::type>(std::forward<IteratorType>(begin), std::forward<IteratorType>(end));
    }

    /**
     *    Helper functions to make numeric ranges (e.g. for ranged for-loops)
     */
    template <typename _T>
    YATO_CONSTEXPR_FUNC
    typename std::enable_if < std::is_integral<typename std::decay<_T>::type>::value, range<numeric_iterator<typename std::decay<_T>::type>> >::type
        make_range(_T && left, _T && right)
    {
        return make_range(numeric_iterator<typename std::decay<_T>::type>(std::forward<_T>(left)), numeric_iterator<typename std::decay<_T>::type>(std::forward<_T>(right)));
    }

    template <typename _T>
    YATO_CONSTEXPR_FUNC
    typename std::enable_if < std::is_unsigned<typename std::decay<_T>::type>::value, range<numeric_iterator<typename std::decay<_T>::type>> >::type
        make_range(_T && right)
    {
        return make_range(static_cast<typename std::decay<_T>::type>(0), std::forward<_T>(right));
    }

}

#endif
