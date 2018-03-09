/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_RANGE_H_
#define _YATO_RANGE_H_

#include <numeric>
#include <vector>
#include <list>
#include <set>

#include "numeric_iterator.h"
#include "transform_iterator.h"
#include "filter_iterator.h"
#include "zip_iterator.h"

namespace yato
{
#ifdef YATO_MSVC_2013
    namespace details
    {
        template <typename _RangesList, typename... _Iterators>
        struct ranges_to_zip_iterator_impl
        {
            using head_range = typename _RangesList::head;
            using type = typename ranges_to_zip_iterator_impl<typename _RangesList::tail, _Iterators..., typename head_range::iterator_type>::type;
        };

        template <typename... _Iterators>
        struct ranges_to_zip_iterator_impl<meta::null_list, _Iterators...>
        {
            using type = zip_iterator<_Iterators...>;
        };

        template <typename... _Ranges>
        struct ranges_to_zip_iterator
        {
            using type = typename ranges_to_zip_iterator_impl<meta::list<_Ranges...>>::type;
        };
    }
#endif

    /**
     *  Immutable object aggregating two iterators
     *  Helps to express a range of one container
     */
    template<typename _IteratorType>
    class range
    {
    public:
        using iterator_type = _IteratorType;
        using reverse_iterator_type = std::reverse_iterator<iterator_type>;
        static_assert(is_iterator<iterator_type>::value, "yato::range can be used only for iterators");
        using my_type = range<iterator_type>;

        using difference_type = typename std::iterator_traits<iterator_type>::difference_type;
        //-------------------------------------------------------

    private:
        const iterator_type m_begin;
        const iterator_type m_end;
        //-------------------------------------------------------

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
        reverse_iterator_type rbegin() const
        {
            return reverse_iterator_type(end());
        }

        YATO_CONSTEXPR_FUNC
        reverse_iterator_type rend() const
        {
            return reverse_iterator_type(begin());
        }

        YATO_CONSTEXPR_FUNC
        range<reverse_iterator_type> reverse() const
        {
            return range<reverse_iterator_type>(rbegin(), rend());
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

        /**
         *  Apply lazy transformation to all elements
         */
        template <typename _Callable>
        YATO_CONSTEXPR_FUNC
        auto map(const _Callable & callable) const
            -> range<transform_iterator<_Callable, iterator_type>>
        {
            using _transformed_iterator_type = transform_iterator<_Callable, iterator_type>;
            return range<_transformed_iterator_type>(_transformed_iterator_type(begin(), callable), _transformed_iterator_type(end(), callable));
        }

        /**
         *  Apply lazy filtering 
         */
        template <typename _DereferencePolicy = dereference_policy_no_caching, typename _Predicate>
        YATO_CONSTEXPR_FUNC
        auto filter(const _Predicate & predicate) const
            -> range<filter_iterator<_Predicate, iterator_type, _DereferencePolicy>>
        {
            using _filtered_iterator_type = filter_iterator<_Predicate, iterator_type, _DereferencePolicy>;
            return range<_filtered_iterator_type>(_filtered_iterator_type(begin(), end(), predicate), _filtered_iterator_type(end(), end(), predicate));
        }

        /**
         *  Join two ranges into range of tuples
         */
        template <typename... _Ranges>
        YATO_CONSTEXPR_FUNC
        auto zip(const _Ranges & ...ranges) const
            -> range<zip_iterator<iterator_type, typename _Ranges::iterator_type...>> 
        {
#ifdef YATO_MSVC_2013
            using _zipped_iterator_type = typename details::ranges_to_zip_iterator<my_type, _Ranges...>::type;
#else
            using _zipped_iterator_type = zip_iterator<iterator_type, typename _Ranges::iterator_type...>;
#endif
            return range<_zipped_iterator_type>(_zipped_iterator_type(std::make_tuple(begin(), ranges.begin()...)), _zipped_iterator_type(std::make_tuple(end(), ranges.end()...)));
        }

        /**
         *  Accumulate range values from left to right
         */
        template <typename _BinaryFunction, typename _ValueType>
        YATO_CONSTEXPR_FUNC
        _ValueType fold_left(_BinaryFunction && function, const _ValueType & initialValue) const
        {
            return std::accumulate(begin(), end(), initialValue, std::forward<_BinaryFunction>(function));
        }

        /**
         *  Accumulate range values from right to left
         */
        template <typename _BinaryFunction, typename _ValueType>
        YATO_CONSTEXPR_FUNC
        _ValueType fold_right(_BinaryFunction && function, const _ValueType & initialValue) const
        {
            return std::accumulate(rbegin(), rend(), initialValue, std::forward<_BinaryFunction>(function));
        }

        /**
         *  Convert range to vector via copying
         */
        template <typename _Allocator = std::allocator<typename std::iterator_traits<iterator_type>::value_type>>
        std::vector<typename std::iterator_traits<iterator_type>::value_type, _Allocator> to_vector(const _Allocator & allocator = _Allocator()) const
        {
            return std::vector<typename std::iterator_traits<iterator_type>::value_type, _Allocator>(begin(), end(), allocator);
        }

        /**
         *  Convert range to list via copying
         */
        template <typename _Allocator = std::allocator<typename std::iterator_traits<iterator_type>::value_type>>
        std::list<typename std::iterator_traits<iterator_type>::value_type, _Allocator> to_list(const _Allocator & allocator = _Allocator()) const
        {
            return std::list<typename std::iterator_traits<iterator_type>::value_type, _Allocator>(begin(), end(), allocator);
        }

        /**
         *  Convert range to set via copying
         */
        template <typename _Comparator = std::less<typename std::iterator_traits<iterator_type>::value_type>, typename _Allocator = std::allocator<typename std::iterator_traits<iterator_type>::value_type>>
        std::set<typename std::iterator_traits<iterator_type>::value_type, _Comparator, _Allocator> to_set(const _Comparator & comparator = _Comparator(), const _Allocator & allocator = _Allocator()) const
        {
            return std::set<typename std::iterator_traits<iterator_type>::value_type, _Comparator, _Allocator>(begin(), end(), comparator, allocator);
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
    typename std::enable_if < std::is_integral<typename std::decay<_T>::type>::value, range<numeric_iterator<typename std::decay<_T>::type>> >::type
        make_range(_T && right)
    {
        return make_range(static_cast<typename std::decay<_T>::type>(0), std::forward<_T>(right));
    }

    /**
     *  Generic version. Calls begin() and end()
     */
    template <typename _T>
    YATO_CONSTEXPR_FUNC
    auto make_range(_T && object)
        -> typename std::enable_if<std::is_class<typename std::decay<_T>::type>::value, range<decltype(object.begin())> >::type
    {
        return range<decltype(object.begin())>(object.begin(), object.end());
    }

    /**
     *  Generic version. Calls cbegin() and cend()
     */
    template <typename _T>
    YATO_CONSTEXPR_FUNC
    auto make_crange(const _T & object)
        -> typename std::enable_if<std::is_class<typename std::decay<_T>::type>::value, range<decltype(object.cbegin())> >::type
    {
        return range<decltype(object.cbegin())>(object.cbegin(), object.cend());
    }
}

#endif
