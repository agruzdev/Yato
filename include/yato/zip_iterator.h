/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ZIP_ITERATOR_H_
#define _YATO_ZIP_ITERATOR_H_

#include "meta.h"
#include "tuple.h"

namespace yato
{

    template<typename... _Iterators>
    class zip_iterator
    {
    public:
        using my_type = zip_iterator<_Iterators...>;
        using iterators_list = meta::list<_Iterators...>;
        using iterators_tuple = std::tuple<_Iterators...>;

        static YATO_CONSTEXPR_VAR size_t iterators_num = sizeof...(_Iterators);
        //-------------------------------------------------------
        // Definitions for iterator_traits
        /**
        * Reference type is the type of the tuple obtained from the iterators' reference types.
        */
        using reference = std::tuple<typename std::iterator_traits<_Iterators>::reference...>;
        /**
        * Pointer type is the type of the tuple obtained from the iterators' pointer types.
        */
        using pointer = std::tuple<typename std::iterator_traits<_Iterators>::pointer...>;
        /**
        * Value type is the same as reference type
        */
        using value_type = reference;
        /**
        * Difference type is the first iterator's difference type
        */
        using difference_type = typename std::iterator_traits<typename meta::list<_Iterators...>::head>::difference_type;
        /**
        * Category is the most common category for all of iterators
        */
        using iterator_category = typename std::common_type<typename std::iterator_traits<_Iterators>::iterator_category ...>::type;
        //-------------------------------------------------------

    private:
        template <typename _Iterator>
        struct _increment
        {
            void operator()(_Iterator & it) const
            {
                it++;
            }
        };

        template <typename _Iterator>
        struct _decrement
        {
            void operator()(_Iterator & it) const
            {
                it--;
            }
        };

        template <typename _Iterator>
        struct _dereference
        {
            YATO_CONSTEXPR_FUNC
            auto operator()(_Iterator & it) const
                -> typename std::iterator_traits<_Iterator>::reference
            {
                return *it;
            }
        };
        //-------------------------------------------------------

        iterators_tuple m_iterators;
        //-------------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        zip_iterator(const iterators_tuple & iters)
            : m_iterators(iters)
        { }

        YATO_CONSTEXPR_FUNC
        zip_iterator(iterators_tuple && iters)
            : m_iterators(std::move(iters))
        { }

        template <typename... _Args>
        YATO_CONSTEXPR_FUNC
        zip_iterator(_Args && ...args)
            : m_iterators({std::forward<_Args>(args)...})
        { }

        YATO_CONSTEXPR_FUNC
        zip_iterator(const my_type & other)
            : m_iterators(other.m_iterators)
        { }

        YATO_CONSTEXPR_FUNC
        zip_iterator(my_type && other)
            : m_iterators(std::move(other.m_iterators))
        { }

        ~zip_iterator()
        { }

        my_type & operator = (const my_type & other)
        {
            if (this != &other) {
                m_iterators = other.m_iterators;
            }
        }

        my_type & operator = (my_type && other)
        {
            if (this != &other) {
                m_iterators = std::move(other.m_iterators);
            }
        }

        YATO_CONSTEXPR_FUNC
        const reference operator*() const
        {
            return tuple_transform<_dereference>(m_iterators);
        }

        reference operator*()
        {
            return tuple_transform<_dereference>(m_iterators);
        }

        my_type & operator++ ()
        {
            tuple_for_each<_increment>(m_iterators);
            return *this;
        }

        my_type operator++ (int)
        {
            auto copy(*this);
            ++(*this);
            return copy;
        }

        template<typename _MyCategory = iterator_category>
        auto operator-- ()
            -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, _MyCategory>::value, my_type &>::type
        {
            tuple_for_each<_decrement>(m_iterators);
            return *this;
        }

        auto operator-- (int)
        {
            auto copy(*this);
            --(*this);
            return copy;
        }
    };
}

#endif
