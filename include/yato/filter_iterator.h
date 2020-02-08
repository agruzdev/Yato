/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_FILTER_ITERATOR_H_
#define _YATO_FILTER_ITERATOR_H_

#include "type_traits.h"
#include "storage.h"

#ifndef YATO_FILTER_ITER_SIZE
# define YATO_FILTER_ITER_SIZE (64)
#endif

#ifndef YATO_FILTER_ITER_CACHE_SIZE
# define YATO_FILTER_ITER_CACHE_SIZE (64)
#endif

namespace yato
{
 

    /**
     *  Save value after dereferencing to avoid multiple dereferencing of iterator
     */
    class dereference_policy_caching
    { };

    /**
     *  Multiple dereferencing
     */
    class dereference_policy_no_caching
    { };

    namespace details
    {
        template <typename _T, typename _Policy, typename _Enable = void>
        struct filter_cache_base
        {
            static YATO_CONSTEXPR_VAR bool _base_cache = false;
            static YATO_CONSTEXPR_VAR size_t cache_max_size = 0;

            filter_cache_base() 
            { }

            template <typename _AnotherType, typename _AnotherPolicy, typename _AnotherEnable>
            filter_cache_base(const filter_cache_base<_AnotherType, _AnotherPolicy, _AnotherEnable> &)
            { }

            template <typename _AnotherType, typename _AnotherPolicy, typename _AnotherEnable>
            filter_cache_base(filter_cache_base<_AnotherType, _AnotherPolicy, _AnotherEnable> &&)
            { }

            template <typename _AnotherType, typename _AnotherPolicy, typename _AnotherEnable>
            void swap(filter_cache_base<_AnotherType, _AnotherPolicy, _AnotherEnable> &)
            { }

            template <typename _AnotherType, typename _AnotherPolicy, typename _AnotherEnable>
            void _move_impl(filter_cache_base<_AnotherType, _AnotherPolicy, _AnotherEnable> &&)
            { }
        };

        template <typename _T, typename _Policy>
        struct filter_cache_base<_T, _Policy, typename std::enable_if<std::is_same<_Policy, dereference_policy_caching>::value>::type>
        {
            static YATO_CONSTEXPR_VAR bool _base_cache = true;
            static YATO_CONSTEXPR_VAR size_t cache_max_size = YATO_FILTER_ITER_CACHE_SIZE;
            storage<_T, cache_max_size> m_cached;

            filter_cache_base()
            { }

            template <typename _AnotherType, typename _AnotherPolicy, typename _AnotherEnable>
            filter_cache_base(const filter_cache_base<_AnotherType, _AnotherPolicy, _AnotherEnable> &)
            { }

            template <typename _AnotherType, typename _AnotherEnable>
            filter_cache_base(const filter_cache_base<_AnotherType, dereference_policy_caching, _AnotherEnable> & other)
                : m_cached(other.m_cached)
            { }

            template <typename _AnotherType, typename _AnotherPolicy, typename _AnotherEnable>
            filter_cache_base(filter_cache_base<_AnotherType, _AnotherPolicy, _AnotherEnable> &&)
            { }

            template <typename _AnotherType, typename _AnotherEnable>
            filter_cache_base(filter_cache_base<_AnotherType, dereference_policy_caching, _AnotherEnable> && other)
                : m_cached(std::move(other.m_cached))
            { }

            template <typename _AnotherType, typename _AnotherPolicy, typename _AnotherEnable>
            void swap(filter_cache_base<_AnotherType, _AnotherPolicy, _AnotherEnable> &)
            { }

            template <typename _AnotherType, typename _AnotherEnable>
            void swap(filter_cache_base<_AnotherType, dereference_policy_caching, _AnotherEnable> & other)
            {
                m_cached.swap(other.m_cached);
            }

            template <typename _AnotherType, typename _AnotherPolicy, typename _AnotherEnable>
            void _move_impl(filter_cache_base<_AnotherType, _AnotherPolicy, _AnotherEnable> &&)
            { }

            template <typename _AnotherType, typename _AnotherEnable>
            void _move_impl(filter_cache_base<_AnotherType, dereference_policy_caching, _AnotherEnable> && other)
            { 
                m_cached = std::move(other.m_cached);
            }

            void _save_cache(const _T & obj)
            {
                m_cached = obj;
            }

            _T & _get_cache()
            {
                return *m_cached;
            }
        };
    }


    template <typename _Predicate, typename _Iterator, typename _DereferencePolicy = dereference_policy_no_caching, bool _HasEnd = true>
    class filter_iterator 
        : private details::filter_cache_base<typename std::iterator_traits<_Iterator>::value_type, _DereferencePolicy>
    {
        using _base = details::filter_cache_base<typename std::iterator_traits<_Iterator>::value_type, _DereferencePolicy>;
    public:
        using predicate_type = _Predicate;
        using iterator_type = _Iterator;
        using dereference_policy = _DereferencePolicy;
        using my_type = filter_iterator<predicate_type, iterator_type, dereference_policy, true>;

        static_assert(std::is_convertible<typename yato::callable_trait<_Predicate>::result_type, bool>::value, "Predicate return type should be convertible to boolean");

        using underlying_iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;

        static YATO_CONSTEXPR_VAR bool uses_cache = _base::_base_cache;
        static YATO_CONSTEXPR_VAR size_t _predicate_storage_max_size = YATO_FILTER_ITER_SIZE;
        using predicate_storage = storage<predicate_type, _predicate_storage_max_size>;

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
        predicate_storage m_predicate;

        template <typename _MyPolicy = dereference_policy>
        auto _skip_forward()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_caching>::value>::type
        {
            while (m_iterator != m_end) {
                _base::_save_cache(*m_iterator);
                if ((*m_predicate)(_base::_get_cache())) {
                    break;
                }
                ++m_iterator;
            }
        }
        
        template <typename _MyPolicy = dereference_policy>
        auto _skip_forward()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_no_caching>::value>::type
        {
            while (m_iterator != m_end && !(*m_predicate)(*m_iterator)) {
                ++m_iterator;
            }
        }

        template <typename _MyPolicy = dereference_policy>
        auto _skip_backward()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_caching>::value>::type
        {
            for (;;) {
                _base::_save_cache(*m_iterator);
                if ((*m_predicate)(_base::_get_cache())) {
                    break;
                }
                --m_iterator;
            }
        }

        template <typename _MyPolicy = dereference_policy>
        auto _skip_backward()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_no_caching>::value>::type
        {
            while (!(*m_predicate)(*m_iterator)) {
                --m_iterator;
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
            :  m_iterator(std::forward<_IteratorReference>(iter)), m_end(std::forward<_IteratorReference>(end)), m_predicate(std::forward<_PredicateReference>(predicate))
        {
            _skip_forward();
        }

        /**
         *  Copy
         */
        filter_iterator(const my_type & other)
            : _base(other), m_iterator(other.m_iterator), m_end(other.m_end), m_predicate(other.m_predicate)
        { }

        /**
         *  Copy
         */
        template<typename _AnotherIterator, typename _AnotherPredicate, typename _AnotherPolicy>
        filter_iterator(const filter_iterator<_AnotherPredicate, _AnotherIterator, _AnotherPolicy> & other,
            typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value)>::type* = nullptr)
            : _base(other), m_iterator(other.m_iterator), m_end(other.m_end), m_predicate(predicate_type(*other.m_predicate))
        { }

        /**
         *  Move
         */
        filter_iterator(my_type && other)
            : _base(std::move(other)), m_iterator(std::move(other.m_iterator)), m_end(std::move(other.m_end)), m_predicate(std::move(other.m_predicate))
        { }

        /**
         *  Move
         */
        template<typename _AnotherIterator, typename _AnotherPredicate, typename _AnotherPolicy>
        filter_iterator(filter_iterator<_AnotherPredicate, _AnotherIterator, _AnotherPolicy> && other,
            typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value)>::type* = nullptr)
            : _base(std::move(other)), m_iterator(std::move(other.m_iterator)), m_end(std::move(other.m_end)), m_predicate(std::move(predicate_type(*other.m_predicate)))
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
                _base::swap(other);
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
        template<typename _AnotherPredicate, typename _AnotherIterator, typename _AnotherPolicy>
        auto operator = (const filter_iterator<_AnotherPredicate, _AnotherIterator, _AnotherPolicy> & other)
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
                _base::_move_impl(std::move(other));
                m_iterator = std::move(other.m_iterator);
                m_end = std::move(other.m_end);
                m_predicate = std::move(other.m_predicate);
            }
            return *this;
        }

        /**
         *  Move
         */
        template<typename _AnotherPredicate, typename _AnotherIterator, typename _AnotherPolicy>
        auto operator = (filter_iterator<_AnotherPredicate, _AnotherIterator, _AnotherPolicy> && other)
            -> typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value), my_type &>::type
        {
            _base::_move_impl(std::move(other));
            m_iterator = std::move(other.m_iterator);
            m_end = std::move(other.m_end);
            m_predicate = std::move(predicate_type(*other.m_predicate));
            return *this;
        }

        /**
         *  Dereference
         */
        template <typename _MyPolicy = dereference_policy>
        auto operator*()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_caching>::value, reference>::type
        {
            return _base::_get_cache();
        }

        /**
        *  Dereference
        */
        template <typename _MyPolicy = dereference_policy>
        auto operator*()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_no_caching>::value, reference>::type
        {
            return *m_iterator;
        }

        /**
          *  Dereference
        */
        const reference operator*() const
        {
            return *(const_cast<my_type*>(this));
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
         *  Decrement and skip all values not satisfying the predicate
         */
        template<typename _MyCategory = iterator_category>
        auto operator-- ()
            -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, _MyCategory>::value, my_type &>::type
        {
            --m_iterator;
            _skip_backward();
            return *this;
        }

        /**
         *  Decrement and skip all values not satisfying the predicate
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
        template <typename _SomePredicate, typename _SomeIterator, typename _SomePolicy, bool _SomeEnd>
        friend class filter_iterator;
    };

    /**
     *  Filter iterator without end limit
     */
    template <typename _Predicate, typename _Iterator, typename _DereferencePolicy>
    class filter_iterator <_Predicate, _Iterator, _DereferencePolicy, false>
        : private details::filter_cache_base<typename std::remove_reference<typename std::iterator_traits<_Iterator>::reference>::type, _DereferencePolicy>
    {
        using _base = details::filter_cache_base<typename std::remove_reference<typename std::iterator_traits<_Iterator>::reference>::type, _DereferencePolicy>;
    public:
        using predicate_type = _Predicate;
        using iterator_type = _Iterator;
        using dereference_policy = _DereferencePolicy;
        using my_type = filter_iterator<predicate_type, iterator_type, dereference_policy, false>;

        static_assert(std::is_convertible<typename yato::callable_trait<_Predicate>::result_type, bool>::value, "Predicate return type should be convertible to boolean");

        using underlying_iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;

        static YATO_CONSTEXPR_VAR bool uses_cache = _base::_base_cache;
        static YATO_CONSTEXPR_VAR size_t _predicate_storage_max_size = YATO_FILTER_ITER_SIZE;
        using predicate_storage = storage<predicate_type, _predicate_storage_max_size>;

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
        predicate_storage m_predicate;

        template <typename _MyPolicy = dereference_policy>
        auto _skip_forward()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_caching>::value>::type
        {
            for (;;) {
                _base::_save_cache(*m_iterator);
                if ((*m_predicate)(_base::_get_cache())) {
                    break;
                }
                ++m_iterator;
            }
        }

        template <typename _MyPolicy = dereference_policy>
        auto _skip_forward()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_no_caching>::value>::type
        {
            while (!(*m_predicate)(*m_iterator)) {
                ++m_iterator;
            }
        }

        template <typename _MyPolicy = dereference_policy>
        auto _skip_backward()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_caching>::value>::type
        {
            for (;;) {
                _base::_save_cache(*m_iterator);
                if ((*m_predicate)(_base::_get_cache())) {
                    break;
                }
                --m_iterator;
            }
        }

        template <typename _MyPolicy = dereference_policy>
        auto _skip_backward()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_no_caching>::value>::type
        {
            while (!(*m_predicate)(*m_iterator)) {
                --m_iterator;
            }
        }

        //-------------------------------------------------------

    public:
        /**
        *  Create from iterators and predicate
        *  Immediately moves to the first correct position
        */
        template <typename _IteratorReference, typename _PredicateReference>
        filter_iterator(_IteratorReference && iter, _PredicateReference && predicate,
            typename std::enable_if<(std::is_convertible<typename std::remove_reference<_IteratorReference>::type, iterator_type>::value && std::is_convertible<typename std::remove_reference<_PredicateReference>::type, predicate_type>::value)>::type* = nullptr)
            : m_iterator(std::forward<_IteratorReference>(iter)), m_predicate(std::forward<_PredicateReference>(predicate))
        {
            _skip_forward();
        }

        /**
        *  Copy
        */
        filter_iterator(const my_type & other)
            : _base(other), m_iterator(other.m_iterator), m_predicate(other.m_predicate)
        {
        }

        /**
        *  Copy
        */
        template<typename _AnotherIterator, typename _AnotherPredicate, typename _AnotherPolicy>
        filter_iterator(const filter_iterator<_AnotherPredicate, _AnotherIterator, _AnotherPolicy> & other,
            typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value)>::type* = nullptr)
            : _base(other), m_iterator(other.m_iterator), m_predicate(predicate_type(*other.m_predicate))
        {
        }

        /**
        *  Move
        */
        filter_iterator(my_type && other)
            : _base(std::move(other)), m_iterator(std::move(other.m_iterator)), m_predicate(std::move(other.m_predicate))
        {
        }

        /**
        *  Move
        */
        template<typename _AnotherIterator, typename _AnotherPredicate, typename _AnotherPolicy>
        filter_iterator(filter_iterator<_AnotherPredicate, _AnotherIterator, _AnotherPolicy> && other,
            typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value)>::type* = nullptr)
            : _base(std::move(other)), m_iterator(std::move(other.m_iterator)), m_predicate(std::move(predicate_type(*other.m_predicate)))
        {
        }


        /**
        *  Destroy
        */
        ~filter_iterator()
        {
        }

        /**
        *  Swap iterators
        */
        void swap(my_type & other) YATO_NOEXCEPT_KEYWORD
        {
            using std::swap;
            if (this != &other) {
                _base::swap(other);
                swap(m_iterator, other.m_iterator);
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
        template<typename _AnotherPredicate, typename _AnotherIterator, typename _AnotherPolicy>
        auto operator = (const filter_iterator<_AnotherPredicate, _AnotherIterator, _AnotherPolicy> & other)
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
                _base::_move_impl(std::move(other));
                m_iterator = std::move(other.m_iterator);
                m_predicate = std::move(other.m_predicate);
            }
            return *this;
        }

        /**
        *  Move
        */
        template<typename _AnotherPredicate, typename _AnotherIterator, typename _AnotherPolicy>
        auto operator = (filter_iterator<_AnotherPredicate, _AnotherIterator, _AnotherPolicy> && other)
            -> typename std::enable_if<(std::is_convertible<_AnotherPredicate, predicate_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value), my_type &>::type
        {
            _base::_move_impl(std::move(other));
            m_iterator = std::move(other.m_iterator);
            m_predicate = std::move(predicate_type(*other.m_predicate));
            return *this;
        }

        /**
        *  Dereference
        */
        template <typename _MyPolicy = dereference_policy>
        auto operator*()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_caching>::value, reference>::type
        {
            return _base::_get_cache();
        }

        /**
        *  Dereference
        */
        template <typename _MyPolicy = dereference_policy>
        auto operator*()
            -> typename std::enable_if<std::is_same<_MyPolicy, dereference_policy_no_caching>::value, reference>::type
        {
            return *m_iterator;
        }

        /**
        *  Dereference
        */
        const reference operator*() const
        {
            return *(const_cast<my_type*>(this));
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
        *  Decrement and skip all values not satisfying the predicate
        */
        template<typename _MyCategory = iterator_category>
        auto operator-- ()
            -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, _MyCategory>::value, my_type &>::type
        {
            --m_iterator;
            _skip_backward();
            return *this;
        }

        /**
        *  Decrement and skip all values not satisfying the predicate
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
        template <typename _SomePredicate, typename _SomeIterator, typename _SomePolicy, bool _SomeEnd>
        friend class filter_iterator;
    };


    template <typename Predicate_, typename Iterator_, typename SomePolicy_, bool HasEnd_>
    void swap(filter_iterator<Predicate_, Iterator_, SomePolicy_, HasEnd_> & one, filter_iterator<Predicate_, Iterator_, SomePolicy_, HasEnd_> & another) YATO_NOEXCEPT_KEYWORD
    {
        one.swap(another);
    }

    template<typename DereferencePolicy_ = dereference_policy_no_caching, typename Predicate_, typename Iterator_>
    auto make_filter_iterator(Iterator_ && iterator, Iterator_ && end, Predicate_ && predicate)
        -> filter_iterator<yato::remove_cvref_t<Predicate_>, yato::remove_cvref_t<Iterator_>, DereferencePolicy_>
    {
        return filter_iterator<yato::remove_cvref_t<Predicate_>, yato::remove_cvref_t<Iterator_>, DereferencePolicy_>(std::forward<Iterator_>(iterator), std::forward<Iterator_>(end), std::forward<Predicate_>(predicate));
    }

    template<typename DereferencePolicy_ = dereference_policy_no_caching, typename Predicate_, typename Iterator_>
    auto make_filter_iterator(Iterator_ && iterator, Predicate_ && predicate)
        -> filter_iterator<yato::remove_cvref_t<Predicate_>, yato::remove_cvref_t<Iterator_>, DereferencePolicy_, false>
    {
        return filter_iterator<yato::remove_cvref_t<Predicate_>, yato::remove_cvref_t<Iterator_>, DereferencePolicy_, false>(std::forward<Iterator_>(iterator), std::forward<Predicate_>(predicate));
    }
}

#endif
