/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
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

    /**
     *  Immutable object aggregating two iterators
     *  Helps to express a range of one container
     */
    template<typename IteratorType_, typename EndType_ = IteratorType_>
    class range
    {
        using this_type = range<IteratorType_, EndType_>;
    public:
        using iterator_type = IteratorType_;
        using end_type      = EndType_;
        using reverse_iterator_type = std::reverse_iterator<iterator_type>;
        static_assert(is_iterator<iterator_type>::value, "yato::range can be used only for iterators");

        using difference_type = typename std::iterator_traits<iterator_type>::difference_type;

        static YATO_CONSTEXPR_VAR bool is_uniform = std::is_same<IteratorType_, EndType_>::value;
        //-------------------------------------------------------

    private:
        iterator_type m_begin;
        end_type      m_end;
        //-------------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC 
        range(iterator_type begin, end_type end)
            : m_begin(std::move(begin)), m_end(std::move(end))
        { }

        range(const range&) = default;
        range(range&&) = default;

        range& operator=(const range&) = default;
        range& operator=(range&&) = default;

        ~range() = default;

        YATO_CONSTEXPR_FUNC 
        const iterator_type & begin() const
        {
            return m_begin;
        }

        YATO_CONSTEXPR_FUNC 
        const end_type & end() const
        {
            return m_end;
        }

        template <bool UniformRange_ = is_uniform>
        YATO_CONSTEXPR_FUNC
        auto rbegin() const
            -> std::enable_if_t<UniformRange_, reverse_iterator_type>
        {
            return reverse_iterator_type(end());
        }

        template <bool UniformRange_ = is_uniform>
        YATO_CONSTEXPR_FUNC
        auto rend() const
            -> std::enable_if_t<UniformRange_, reverse_iterator_type>
        {
            return reverse_iterator_type(begin());
        }

        template <bool UniformRange_ = is_uniform>
        YATO_CONSTEXPR_FUNC
        auto reverse() const
            -> std::enable_if_t<UniformRange_, range<reverse_iterator_type>>
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
            using zipped_iterator_type = zip_iterator<iterator_type, typename Ranges_::iterator_type...>;
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
    template <typename IteratorType_, typename EndType_>
    YATO_CONSTEXPR_FUNC 
    auto make_range(IteratorType_ && begin, EndType_ && end)
    {
        return range<yato::remove_cvref_t<IteratorType_>, yato::remove_cvref_t<EndType_>>(std::forward<IteratorType_>(begin), std::forward<EndType_>(end));
    }

    /**
     *  Generic version. Calls std::begin() and std::end()
     */
    template <typename Ty_>
    YATO_CONSTEXPR_FUNC
    auto make_range(Ty_& object)
    {
        return make_range(std::begin(object), std::end(object));
    }

    /**
     *  Generic version. Calls std::cbegin() and std::cend()
     */
    template <typename Ty_>
    YATO_CONSTEXPR_FUNC
    auto make_crange(const Ty_& object)
    {
        return make_range(std::cbegin(object), std::cend(object));
    }


    /**
     *  Helper functions to make numeric ranges (e.g. for ranged for-loops)
     */
    template <typename Ty_>
    YATO_CONSTEXPR_FUNC
    auto numeric_range(Ty_&& last)
    {
        using iter_type = numeric_iterator<yato::remove_cvref_t<Ty_>>;
        return make_range(iter_type(static_cast<yato::remove_cvref_t<Ty_>>(0)), iter_type(std::forward<Ty_>(last)));
    }

    /**
     *  Helper functions to make numeric ranges (e.g. for ranged for-loops)
     */
    template <typename Ty1_, typename Ty2_>
    YATO_CONSTEXPR_FUNC
    auto numeric_range(Ty1_&& first, Ty2_&& last)
    {
        using iter1_type = numeric_iterator<yato::remove_cvref_t<Ty1_>>;
        using iter2_type = numeric_iterator<yato::remove_cvref_t<Ty2_>>;
        return make_range(iter1_type(std::forward<Ty1_>(first)), iter2_type(std::forward<Ty2_>(last)));
    }
}

#endif
