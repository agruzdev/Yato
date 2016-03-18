/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_FILTER_ITERATOR_H_
#define _YATO_FILTER_ITERATOR_H_

#include "type_traits.h"

namespace yato
{
 
    template <typename _Predicate, typename _Iterator>
    class filter_iterator
    {
    public:
        using predicate_type = _Predicate;
        using iterator_type = _Iterator;
        using my_type = filter_iterator<predicate_type, iterator_type>;

        static_assert(std::is_convertible<typename yato::callable_trait<_Predicate>::result_type, bool>::value, "Predicate return type should be convertible to boolean");

        using underlying_iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;

        //-------------------------------------------------------
        // Definitions for iterator_traits
        /**
            * Category is not greater than bidirectional
            */
        using iterator_category = typename std::conditional<!std::is_same<underlying_iterator_category, std::random_access_iterator_tag>::value,
            underlying_iterator_category,
            std::bidirectional_iterator_tag >::type;
        /**
            * Dereferencing the iterator yields the same type as base iterator
            */
        using reference = typename std::iterator_traits<iterator_type>::reference;
        /**
            * Value type preserves cv-qualifiers
            */
        using value_type = typename std::iterator_traits<iterator_type>::value_type;
        /**
            * Pointer to value
            */
        using pointer = typename std::iterator_traits<iterator_type>::pointer;
        /**
            * Difference type is same as iterator's difference type
            */
        using difference_type = typename std::iterator_traits<iterator_type>::difference_type;
        //-------------------------------------------------------

    private:
        iterator_type m_iterator;
        iterator_type m_end;
        predicate_type m_predicate;

        void _skip_forward()
        {
            while (m_iterator != m_end && !m_predicate(*m_iterator)) {
                ++m_iterator;
            }
        }
        //-------------------------------------------------------

    public:
        /**
            *  Create from iterators and predicate
            *  Immediately moves to the first correct position
            */
        template <typename _IteratorReference, typename _PredicateReference>
        filter_iterator(_IteratorReference && iter, _IteratorReference && end, _PredicateReference && predicate,
            typename std::enable_if<(std::is_convertible<typename std::remove_reference<_IteratorReference>::type, iterator_type>::value && std::is_convertible<typename std::remove_reference<_PredicateReference>::type, predicate_type>::value)>::type* = nullptr)
            : m_iterator(std::forward<_IteratorReference>(iter)), m_end(std::forward<_IteratorReference>(end)), m_predicate(std::forward<_PredicateReference>(predicate))
        {
            _skip_forward();
        }

        /**
        *  Copy
        */
        filter_iterator(const my_type & other)
            : m_iterator(other.m_iterator), m_end(other.m_end), m_predicate(other.m_predicate)
        { }

        /**
            *  Copy
            */
        template<typename _AnotherIterator, typename _AnotherPredicate>
        filter_iterator(const filter_iterator<_AnotherPredicate, _AnotherIterator> & other,
            typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value)>::type* = nullptr)
            : m_iterator(other.m_iterator), m_end(other.m_end), m_predicate(other.m_predicate)
        { }

        /**
            *  Move
            */
        filter_iterator(my_type && other)
            : m_iterator(std::move(other.m_iterator)), m_end(std::move(other.m_end)), m_predicate(std::move(other.m_predicate))
        { }

        /**
            *  Move
            */
        template<typename _AnotherIterator, typename _AnotherPredicate>
        filter_iterator(filter_iterator<_AnotherPredicate, _AnotherIterator> && other,
            typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value)>::type* = nullptr)
            : m_iterator(std::move(other.m_iterator)), m_end(std::move(other.m_end)), m_predicate(std::move(other.m_predicate))
        { }

        /**
            *  Destroy
            */
        ~filter_iterator()
        { }

        /**
            *  Swap iterators
            */
        void swap(my_type & other) YATO_NOEXCEPT_KEYWORD
        {
            using std::swap;
            if (this != &other) {
                swap(m_iterator, other.m_iterator);
                swap(m_end, other.m_end);
                swap(m_predicate, other.m_predicate);
            }
        }

        /**
            *  Copy
            */
        my_type & operator = (const my_type & other)
        {
            if (this != &other) {
                my_type tmp(other);
                tmp.swap(*this);
            }
            return *this;
        }

        /**
            *  Copy
            */
        template<typename _AnotherPredicate, typename _AnotherIterator>
        auto operator = (const filter_iterator<_AnotherPredicate, _AnotherIterator> & other)
            -> typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value), my_type &>::type
        {
            my_type tmp(other);
            tmp.swap(*this);
            return *this;
        }

        /**
            *  Move
            */
        my_type & operator = (my_type && other)
        {
            if (this != &other) {
                m_iterator = std::move(other.m_iterator);
                m_end = std::move(other.m_end);
                m_predicate = std::move(other.m_predicate);
            }
            return *this;
        }

        /**
            *  Move
            */
        template<typename _AnotherPredicate, typename _AnotherIterator>
        auto operator = (filter_iterator<_AnotherPredicate, _AnotherIterator> && other)
            -> typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value), my_type &>::type
        {
            m_iterator = std::move(other.m_iterator);
            m_end = std::move(other.m_end);
            m_predicate = std::move(other.m_predicate);
            return *this;
        }

        /**
            *  Dereference
            */
        const reference operator*() const
        {
            return *m_iterator;
        }

        /**
            *  Dereference
            */
        reference operator*()
        {
            return *m_iterator;
        }

        /**
            *  Increment and skip all values not satisfying the predicate
            */
        my_type & operator++ ()
        {
            ++m_iterator;
            _skip_forward();
            return *this;
        }

        /**
            *  Increment and skip all values not satisfying the predicate
            */
        my_type operator++ (int)
        {
            auto copy(*this);
            ++(*this);
            return copy;
        }

        /**
            *  Decrement
            */
        template<typename _MyCategory = iterator_category>
        auto operator-- ()
            -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, _MyCategory>::value, my_type &>::type
        {
            --m_iterator;
            while (!m_predicate(*m_iterator)) {
                --m_iterator;
            }
            return *this;
        }

        /**
            *  Decrement
            */
        template<typename _MyCategory = iterator_category>
        auto operator-- (int)
            -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, _MyCategory>::value, my_type>::type
        {
            auto copy(*this);
            --(*this);
            return copy;
        }

        /**
            *  Compare inequality
            */
        bool operator != (const my_type & other)  const
        {
            return m_iterator != other.m_iterator;
        }

        /**
            *  Compare inequality
            */
        bool operator != (const iterator_type & other)  const
        {
            return m_iterator != other;
        }

        /**
            *  Compare equality
            */
        bool operator == (const my_type & other) const
        {
            return !(*this != other);
        }

        /**
            *  Compare equality
            */
        bool operator == (const iterator_type & other) const
        {
            return !(*this != other);
        }

        // Make any other iterator friend to access fields in copy operator
        template <typename _SomePredicate, typename _SomeIterator>
        friend class filter_iterator;
    };


    template<typename _Predicate, typename _Iterator>
    inline void swap(filter_iterator<_Predicate, _Iterator> & one, filter_iterator<_Predicate, _Iterator> & another)
    {
        one.swap(another);
    }

    template<typename _Predicate, typename _Iterator>
    inline auto make_filter_iterator(_Iterator && iterator, _Iterator && end, _Predicate && predicate)
        -> filter_iterator<typename callable_trait<typename std::remove_reference<_Predicate>::type>::function_type, typename std::remove_reference<_Iterator>::type>
    {
        return filter_iterator<typename callable_trait<typename std::remove_reference<_Predicate>::type>::function_type, typename std::remove_reference<_Iterator>::type>(std::forward<_Iterator>(iterator), std::forward<_Iterator>(end), std::forward<_Predicate>(predicate));
    }


}

#endif
