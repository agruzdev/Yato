/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_RANGE_H_
#define _YATO_RANGE_H_

#include <numeric>
#include <vector>
#include <list>
#include <set>

#include "type_traits.h"
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
        iterator_type m_begin;
        iterator_type m_end;
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
            m_end   = other.m_end;
            return *this;
        }

        range<iterator_type>& operator=(range<iterator_type> && other) YATO_NOEXCEPT_KEYWORD
        {
            m_begin = std::move(other.m_begin);
            m_end   = std::move(other.m_end);
            return *this;
        }

        ~range() = default;

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
        template <typename Callable_>
        YATO_CONSTEXPR_FUNC
        auto map(Callable_ && callable) const
            -> range<transform_iterator<Callable_, iterator_type>>
        {
            using transformed_iterator_type = transform_iterator<Callable_, iterator_type>;
            return range<transformed_iterator_type>(transformed_iterator_type(begin(), callable), transformed_iterator_type(end(), callable));
        }

        /**
         *  Apply lazy filtering 
         */
        template <typename DereferencePolicy_ = dereference_policy_no_caching, typename Predicate_>
        YATO_CONSTEXPR_FUNC
        auto filter(const Predicate_ & predicate) const
            -> range<filter_iterator<Predicate_, iterator_type, DereferencePolicy_>>
        {
            using filtered_iterator_type = filter_iterator<Predicate_, iterator_type, DereferencePolicy_>;
            return range<filtered_iterator_type>(filtered_iterator_type(begin(), end(), predicate), filtered_iterator_type(end(), end(), predicate));
        }

        /**
         *  Join two ranges into range of tuples
         */
        template <typename... Ranges_>
        YATO_CONSTEXPR_FUNC
        auto zip(const Ranges_ & ... ranges) const
            -> range<zip_iterator<iterator_type, typename Ranges_::iterator_type...>> 
        {
#ifdef YATO_MSVC_2013
            using zipped_iterator_type = typename details::ranges_to_zip_iterator<my_type, Ranges_...>::type;
#else
            using zipped_iterator_type = zip_iterator<iterator_type, typename Ranges_::iterator_type...>;
#endif
            return range<zipped_iterator_type>(zipped_iterator_type(std::make_tuple(begin(), ranges.begin()...)), zipped_iterator_type(std::make_tuple(end(), ranges.end()...)));
        }

        /**
         *  Accumulate range values from left to right
         */
        template <typename BinaryFunction_, typename ValueType_>
        YATO_CONSTEXPR_FUNC
        ValueType_ fold_left(BinaryFunction_ && function, ValueType_ && initialValue) const
        {
            return std::accumulate(begin(), end(), std::forward<ValueType_>(initialValue), std::forward<BinaryFunction_>(function));
        }

        /**
         *  Accumulate range values from right to left
         */
        template <typename BinaryFunction_, typename ValueType_>
        YATO_CONSTEXPR_FUNC
        ValueType_ fold_right(BinaryFunction_ && function, ValueType_ && initialValue) const
        {
            return std::accumulate(rbegin(), rend(), std::forward<ValueType_>(initialValue), std::forward<BinaryFunction_>(function));
        }

        /**
         *  Convert range to vector via copying
         */
        template <typename Allocator_ = std::allocator<typename std::iterator_traits<iterator_type>::value_type>>
        auto to_vector(const Allocator_ & allocator = Allocator_()) const
            -> std::vector<typename std::iterator_traits<iterator_type>::value_type, Allocator_>
        {
            return std::vector<typename std::iterator_traits<iterator_type>::value_type, Allocator_>(begin(), end(), allocator);
        }
            
        /**
         *  Convert range to list via copying
         */
        template <typename Allocator_ = std::allocator<typename std::iterator_traits<iterator_type>::value_type>>
        auto to_list(const Allocator_ & allocator = Allocator_()) const
            -> std::list<typename std::iterator_traits<iterator_type>::value_type, Allocator_> 
        {
            return std::list<typename std::iterator_traits<iterator_type>::value_type, Allocator_>(begin(), end(), allocator);
        }

        /**
         *  Convert range to set via copying
         */
        template <typename Comparator_ = std::less<typename std::iterator_traits<iterator_type>::value_type>, typename Allocator_ = std::allocator<typename std::iterator_traits<iterator_type>::value_type>>
        auto to_set(const Comparator_ & comparator = Comparator_(), const Allocator_ & allocator = Allocator_()) const
            -> std::set<typename std::iterator_traits<iterator_type>::value_type, Comparator_, Allocator_>
        {
            return std::set<typename std::iterator_traits<iterator_type>::value_type, Comparator_, Allocator_>(begin(), end(), comparator, allocator);
        }
    };
    
    /**
    *    Helper functions to make range from a couple of iterators with auto type deduction 
    */
    template<typename _IteratorType>
    YATO_CONSTEXPR_FUNC 
    auto make_range(_IteratorType && begin, _IteratorType && end)
        -> typename std::enable_if< is_iterator< typename yato::remove_cvref<_IteratorType>::type >::value, range< typename yato::remove_cvref<_IteratorType>::type > >::type
    {
        return range<typename yato::remove_cvref<_IteratorType>::type>(std::forward<_IteratorType>(begin), std::forward<_IteratorType>(end));
    }

    /**
     *    Helper functions to make numeric ranges (e.g. for ranged for-loops)
     */
    template <typename Ty_>
    YATO_CONSTEXPR_FUNC
    auto make_range(Ty_ && left, Ty_ && right)
        -> typename std::enable_if < std::is_integral<typename yato::remove_cvref<Ty_>::type>::value, range<numeric_iterator<typename yato::remove_cvref<Ty_>::type>> >::type
    {
        return make_range(numeric_iterator<typename yato::remove_cvref<Ty_>::type>(std::forward<Ty_>(left)), numeric_iterator<typename yato::remove_cvref<Ty_>::type>(std::forward<Ty_>(right)));
    }

    /**
     *    Helper functions to make numeric ranges (e.g. for ranged for-loops)
     */
    template <typename Ty_>
    YATO_CONSTEXPR_FUNC
    auto make_range(Ty_ && right)
        -> typename std::enable_if < std::is_integral<typename yato::remove_cvref<Ty_>::type>::value, range<numeric_iterator<typename yato::remove_cvref<Ty_>::type>> >::type
    {
        return make_range(static_cast<typename yato::remove_cvref<Ty_>::type>(0), std::forward<Ty_>(right));
    }

    /**
     *  Generic version. Calls begin() and end()
     */
    template <typename Ty_>
    YATO_CONSTEXPR_FUNC
    auto make_range(Ty_ && object)
        -> typename std::enable_if<std::is_class<typename yato::remove_cvref<Ty_>::type>::value, range<decltype(object.begin())> >::type
    {
        return range<decltype(object.begin())>(object.begin(), object.end());
    }

    /**
     *  Generic version. Calls cbegin() and cend()
     */
    template <typename Ty_>
    YATO_CONSTEXPR_FUNC
    auto make_crange(const Ty_ & object)
        -> typename std::enable_if<std::is_class<typename yato::remove_cvref<Ty_>::type>::value, range<decltype(object.cbegin())> >::type
    {
        return range<decltype(object.cbegin())>(object.cbegin(), object.cend());
    }
}

#endif
