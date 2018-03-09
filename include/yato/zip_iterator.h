/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
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
        using difference_type = std::ptrdiff_t;
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

        template <typename _Iterator>
        struct _advance
        {
            void operator()(_Iterator & it, difference_type n) const
            {
                it += n;
            }
        };

        template <typename _Iterator1, typename _Iterator2>
        struct _equal
        {
            YATO_CONSTEXPR_FUNC
            bool operator()(const _Iterator1 & it1, const _Iterator2 & it2) const
            {
                return !(it1 != it2);
            }
        };

        template <typename _Iterator1, typename _Iterator2>
        struct _less
        {
            YATO_CONSTEXPR_FUNC
            bool operator()(const _Iterator1 & it1, const _Iterator2 & it2) const
            {
                return it1 < it2;
            }
        };

        template <typename _Iterator1, typename _Iterator2>
        struct _greater
        {
            YATO_CONSTEXPR_FUNC
            bool operator()(const _Iterator1 & it1, const _Iterator2 & it2) const
            {
                return it1 > it2;
            }
        };
        //-------------------------------------------------------

        iterators_tuple m_iterators;
        //-------------------------------------------------------

    public:
        /**
         *  Create from tuple
         */
        YATO_CONSTEXPR_FUNC
        zip_iterator(const iterators_tuple & iters)
            : m_iterators(iters)
        { }
        
        /**
         *  Create from tuple
         */
        YATO_CONSTEXPR_FUNC
        zip_iterator(iterators_tuple && iters)
            : m_iterators(std::move(iters))
        { }

        /**
         *  Copy
         */
        YATO_CONSTEXPR_FUNC
        zip_iterator(const my_type & other)
            : m_iterators(other.m_iterators)
        { }

        /**
         *  Copy 
         */
        template <typename... _OtherIterators>
        YATO_CONSTEXPR_FUNC
        zip_iterator(const zip_iterator<_OtherIterators...> & other)
            : m_iterators(other.m_iterators)
        { }

        /**
         *  Move
         */
        zip_iterator(my_type && other)
            : m_iterators(std::move(other.m_iterators))
        { }

        /**
         *  Move
         */
        template <typename... _OtherIterators>
        zip_iterator(zip_iterator<_OtherIterators...> && other)
            : m_iterators(std::move(other.m_iterators))
        { }

        /**
         *  Destroy
         */
        ~zip_iterator()
        { }

        /**
         *  Swap all iterators
         */
        void swap(my_type & other)
        {
            if (this != &other){
                using std::swap;
                swap(m_iterators, other.m_iterators);
            }
        }

        /**
         *  Copy all iterators
         */
        my_type & operator = (const my_type & other)
        {
            if (this != &other) {
                my_type copy(other);
                copy.swap(*this);
            }
            return *this;
        }

        /**
         *  Copy all iterators
         */
        template <typename... _OtherIterators>
        my_type & operator = (const zip_iterator<_OtherIterators...> & other)
        {
            my_type copy(other);
            copy.swap(*this);
            return *this;
        }

        /**
         *  Move all iterators
         */
        my_type & operator = (my_type && other)
        {
            if (this != &other) {
                m_iterators = std::move(other.m_iterators);
            }
            return *this;
        }

        /**
         *  Move all iterators
         */
        template <typename... _OtherIterators>
        my_type & operator = (zip_iterator<_OtherIterators...> && other)
        {
            m_iterators = std::move(other.m_iterators);
            return *this;
        }

        /**
         *  Dereference all iterators.
         *  @return a tuple of references
         */
        YATO_CONSTEXPR_FUNC
        const reference operator*() const
        {
            return tuple_transform<_dereference>(m_iterators);
        }

        /**
         *  Dereference all iterators. 
         *  @return a tuple of references
         */
        reference operator*()
        {
            return tuple_transform<_dereference>(m_iterators);
        }

        /**
         *  Increment all iterators
         */
        my_type & operator++ ()
        {
            tuple_for_each<_increment>(m_iterators);
            return *this;
        }

        /**
         *  Increment all iterators
         */
        my_type operator++ (int)
        {
            auto copy(*this);
            ++(*this);
            return copy;
        }

        /**
         *  Decrement all iterators
         */
        template<typename _MyCategory = iterator_category>
        auto operator-- ()
            -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, _MyCategory>::value, my_type &>::type
        {
            tuple_for_each<_decrement>(m_iterators);
            return *this;
        }

        /**
         *  Decrement all iterators
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
         *  Shift all iterators by offset
         */
        template<typename _MyCategory = iterator_category>
        auto operator += (difference_type offset)
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, my_type &>::type
        {
            tuple_for_each<_advance>(m_iterators, offset);
            return *this;
        }

        /**
         *  Shift all iterators by offset
         */
        template<typename _MyCategory = iterator_category>
        auto operator -= (difference_type offset)
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, my_type &>::type
        {
            return this->operator+=(-offset);
        }

        /**
         *  Shift all iterators by offset
         */
        template<typename _MyCategory = iterator_category>
        YATO_CONSTEXPR_FUNC
        auto operator + (difference_type offset) const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, my_type>::type
        {
            auto copy(*this);
            return (copy += offset);
        }

        /**
         *  Get distance between iterators
         *  Is computed using the first iterator in the tuple!
         */
        template<typename _MyCategory = iterator_category>
        auto operator - (const my_type & other)  const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, difference_type>::type
        {
            return std::get<0>(m_iterators) - std::get<0>(other.m_iterators);
        }

        /**
         *  Compare equality
         *  Two zip iterators are equal if all iterators in the iterator tuple are equal 
         */
        YATO_CONSTEXPR_FUNC
        bool operator == (const my_type & other)  const
        {
            return tuple_all_of<_equal>(m_iterators, other.m_iterators);
        }

        /**
         *  Compare inequality
         *  Two zip iterators are not equal if any iterators in the tuple are not equal
         */
        YATO_CONSTEXPR_FUNC
        bool operator != (const my_type & other) const
        {
            return !(*this == other);
        }

        /**
         *  Compare less
         */
        template<typename _MyCategory = iterator_category>
        YATO_CONSTEXPR_FUNC
        auto operator < (const my_type & other) const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, bool>::type
        {
            return tuple_all_of<_less>(m_iterators, other.m_iterators);
        }

        /**
         *  Compare greater
         */
        template<typename _MyCategory = iterator_category>
        YATO_CONSTEXPR_FUNC
        auto operator > (const my_type & other) const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, bool>::type
        {
            return tuple_all_of<_greater>(m_iterators, other.m_iterators);
        }

        /**
         *  Compare less or equal
         */
        template<typename _MyCategory = iterator_category>
        YATO_CONSTEXPR_FUNC
        auto operator <= (const my_type & other) const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, bool>::type
        {
            return !(*this > other);
        }

        /**
         *  Compare greater or equal
         */
        template<typename _MyCategory = iterator_category>
        YATO_CONSTEXPR_FUNC
        auto operator >= (const my_type & other) const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, bool>::type
        {
            return !(*this < other);
        }


        template <typename... _SomeIterators>
        friend class zip_iterator;
    };

    template <typename... _Iterators>
    inline void swap(zip_iterator<_Iterators...> & one, zip_iterator<_Iterators...> & another)
    {
        one.swap(another);
    }

    template<typename... _Iterators>
    YATO_CONSTEXPR_FUNC
    zip_iterator<_Iterators...> make_zip_iterator(const std::tuple<_Iterators...> & tuple)
    {
        return zip_iterator<_Iterators...>(tuple);
    }

    template<typename... _Iterators>
    YATO_CONSTEXPR_FUNC
    zip_iterator<_Iterators...> make_zip_iterator(std::tuple<_Iterators...> && tuple)
    {
        return zip_iterator<_Iterators...>(std::move(tuple));
    }

    template<typename... _Iterators>
    YATO_CONSTEXPR_FUNC
    zip_iterator<_Iterators...> make_zip_iterator(_Iterators && ...iters)
    {
        return zip_iterator<_Iterators...>(std::make_tuple(std::forward<_Iterators>(iters)...));
    }
}

#endif
