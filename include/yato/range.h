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
     *  Immutable object aggregating two iterators
     *  Helps to express a range of one container
     */
    template<typename _IteratorType>
    class range
    {
        using iterator_type = _IteratorType;
        static_assert(is_iterator<iterator_type>::value, "yato::range can be used only for iterators");

        using difference_type = typename std::iterator_traits<iterator_type>::difference_type;

        const iterator_type m_begin;
        const iterator_type m_end;
    public:
        YATO_CONSTEXPR_FUNC 
        range(const iterator_type & begin, const iterator_type & end) YATO_NOEXCEPT_KEYWORD
            : m_begin(begin), m_end(end)
        { }

        YATO_CONSTEXPR_FUNC 
        range(iterator_type && begin, iterator_type && end) YATO_NOEXCEPT_KEYWORD
            : m_begin(std::move(begin)), m_end(std::move(end))
        { }

        YATO_CONSTEXPR_FUNC 
        range(const range<iterator_type> & other) YATO_NOEXCEPT_KEYWORD
            : m_begin(other.begin()), m_end(other.end())
        { }

        range(range<iterator_type> && other) YATO_NOEXCEPT_KEYWORD
            : m_begin(std::move(other.m_begin)), m_end(std::move(other.m_end))
        { }

        range<iterator_type>& operator=(const range<iterator_type> & other) YATO_NOEXCEPT_KEYWORD
        {
            m_begin = other.m_begin;
            m_end = other.m_end;
            return *this;
        }

        range<iterator_type>& operator=(range<iterator_type> && other) YATO_NOEXCEPT_KEYWORD
        {
            m_begin = std::move(other.m_begin);
            m_end = std::move(other.m_end);
            return *this;
        }

        ~range() 
        { }

        YATO_CONSTEXPR_FUNC 
        const iterator_type & begin() const YATO_NOEXCEPT_KEYWORD
        {
            return m_begin;
        }

        YATO_CONSTEXPR_FUNC 
        const iterator_type & end() const YATO_NOEXCEPT_KEYWORD
        {
            return m_end;
        }

        YATO_CONSTEXPR_FUNC 
        difference_type distance() const
        {
            return std::distance(m_begin, m_end);
        }

        YATO_CONSTEXPR_FUNC 
        bool empty() const 
        {
            return !(m_begin != m_end);
        }

        YATO_CONSTEXPR_FUNC 
        const iterator_type & head() const YATO_NOEXCEPT_KEYWORD
        {
            return m_begin;
        }

        YATO_CONSTEXPR_FUNC 
        range<iterator_type> tail() const
        {
            return range<iterator_type>(std::next(m_begin), m_end);
        }
    };

    /**
    *    Helper functions to make range from a couple of iterators with auto type deduction 
    */
    template<typename _IteratorType>
    YATO_CONSTEXPR_FUNC 
    typename std::enable_if< is_iterator< typename std::decay<_IteratorType>::type >::value, range< typename std::decay<_IteratorType>::type > >::type
        make_range(_IteratorType && begin, _IteratorType && end)
    {
        return range<typename std::decay<_IteratorType>::type>(std::forward<_IteratorType>(begin), std::forward<_IteratorType>(end));
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

    /**
     *    Helper functions to make numeric ranges (e.g. for ranged for-loops)
     */
    template <typename _T>
    YATO_CONSTEXPR_FUNC
    typename std::enable_if < std::is_unsigned<typename std::decay<_T>::type>::value, range<numeric_iterator<typename std::decay<_T>::type>> >::type
        make_range(_T && right)
    {
        return make_range(static_cast<typename std::decay<_T>::type>(0), std::forward<_T>(right));
    }

    /**
     *  Generic version. Trying to call method range
     */
    template <typename _T>
    YATO_CONSTEXPR_FUNC
    auto make_range(_T && object)
        -> typename std::enable_if<std::is_class<typename std::decay<_T>::type>::value, decltype(object.range())>::type
    {
        return object.range();
    }

    /**
     *  Generic version. Trying to call method range
     */
    template <typename _T>
    YATO_CONSTEXPR_FUNC
    auto make_range(const _T & object)
        -> typename std::enable_if<std::is_class<typename std::decay<_T>::type>::value, decltype(object.crange())>::type
    {
        return object.crange();
    }
}

#endif
