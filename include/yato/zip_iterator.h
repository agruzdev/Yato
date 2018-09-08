/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ZIP_ITERATOR_H_
#define _YATO_ZIP_ITERATOR_H_

#include "assert.h"
#include "meta.h"
#include "tuple.h"
#include "type_traits.h"

namespace yato
{

    template<typename... Iterators_>
    class zip_iterator
    {
    public:
        using this_type       = zip_iterator<Iterators_...>;
        using iterators_list  = meta::list<Iterators_...>;
        using iterators_tuple = std::tuple<Iterators_...>;

        static YATO_CONSTEXPR_VAR size_t iterators_num = sizeof...(Iterators_);
        //-------------------------------------------------------
        // Definitions for iterator_traits
        /**
        * Reference type is the type of the tuple obtained from the iterators' reference types.
        */
        using reference = std::tuple<typename std::iterator_traits<Iterators_>::reference...>;
        /**
        * Pointer type is the type of the tuple obtained from the iterators' pointer types.
        */
        using pointer = std::tuple<typename std::iterator_traits<Iterators_>::pointer...>;
        /**
        * Value type is the same as reference type
        */
        using value_type = std::tuple<typename std::iterator_traits<Iterators_>::value_type...>;
        /**
        * Difference type is the first iterator's difference type
        */
        using difference_type = std::ptrdiff_t;
        /**
        * Category is the most common category for all of iterators
        */
        using iterator_category = typename std::common_type<typename std::iterator_traits<Iterators_>::iterator_category ...>::type;
        //-------------------------------------------------------

    private:
        template <typename Iterator_>
        struct increment_op_
        {
            void operator()(Iterator_ & it) const
            {
                ++it;
            }
        };

        template <typename Iterator_>
        struct decrement_op_
        {
            void operator()(Iterator_ & it) const
            {
                --it;
            }
        };

        template <typename Iterator_>
        struct dereference_op_
        {
            YATO_CONSTEXPR_FUNC
            auto operator()(Iterator_ & it) const
                -> typename std::iterator_traits<Iterator_>::reference
            {
                return *it;
            }
        };

        template <typename Iterator_>
        struct advance_op_
        {
            void operator()(Iterator_ & it, difference_type n) const
            {
                it += n;
            }
        };

        template <typename Iterator1_, typename Iterator2_>
        struct notequal_op_
        {
            YATO_CONSTEXPR_FUNC
            bool operator()(const Iterator1_ & it1, const Iterator2_ & it2) const
            {
                return it1 != it2;
            }
        };

        template <typename Iterator1_, typename Iterator2_>
        struct less_op_
        {
            YATO_CONSTEXPR_FUNC
            bool operator()(const Iterator1_ & it1, const Iterator2_ & it2) const
            {
                return it1 < it2;
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
        zip_iterator(const zip_iterator & other)
            : m_iterators(other.m_iterators)
        { }

        /**
         *  Copy 
         */
        template <typename... OtherIterators_>
        YATO_CONSTEXPR_FUNC
        zip_iterator(const zip_iterator<OtherIterators_...> & other)
            : m_iterators(other.m_iterators)
        { }

        /**
         *  Move
         */
        zip_iterator(zip_iterator && other)
            : m_iterators(std::move(other.m_iterators))
        { }

        /**
         *  Move
         */
        template <typename... OtherIterators_>
        zip_iterator(zip_iterator<OtherIterators_...> && other)
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
        void swap(this_type & other) YATO_NOEXCEPT_KEYWORD
        {
            YATO_REQUIRES(this != &other);
            using std::swap;
            swap(m_iterators, other.m_iterators);
        }

        /**
         *  Copy all iterators
         */
        zip_iterator& operator= (const zip_iterator & other)
        {
            YATO_REQUIRES(this != &other);
            this_type copy(other);
            copy.swap(*this);
            return *this;
        }

        /**
         *  Copy all iterators
         */
        template <typename... OtherIterators_>
        zip_iterator & operator = (const zip_iterator<OtherIterators_...> & other)
        {
            this_type copy(other);
            copy.swap(*this);
            return *this;
        }

        /**
         *  Move all iterators
         */
        zip_iterator & operator = (zip_iterator && other)
        {
            YATO_REQUIRES(this != &other);
            m_iterators = std::move(other.m_iterators);
            return *this;
        }

        /**
         *  Move all iterators
         */
        template <typename... OtherIterators_>
        zip_iterator & operator = (zip_iterator<OtherIterators_...> && other)
        {
            m_iterators = std::move(other.m_iterators);
            return *this;
        }

        /**
         *  Dereference all iterators.
         *  @return a tuple of references
         */
        reference operator*() const
        {
            return tuple_transform<dereference_op_>(const_cast<this_type*>(this)->m_iterators);
        }

        /**
         *  Increment all iterators
         */
        this_type & operator++ ()
        {
            tuple_for_each<increment_op_>(m_iterators);
            return *this;
        }

        /**
         *  Increment all iterators
         */
        this_type operator++ (int)
        {
            auto copy(*this);
            tuple_for_each<increment_op_>(m_iterators);
            return copy;
        }

        /**
         *  Decrement all iterators
         */
        template<typename MyCategory_ = iterator_category>
        auto operator-- ()
            -> std::enable_if_t<std::is_base_of<std::bidirectional_iterator_tag, MyCategory_>::value, this_type&>
        {
            tuple_for_each<decrement_op_>(m_iterators);
            return *this;
        }

        /**
         *  Decrement all iterators
         */
        template<typename MyCategory_ = iterator_category>
        auto operator-- (int)
            -> std::enable_if_t<std::is_base_of<std::bidirectional_iterator_tag, MyCategory_>::value, this_type>
        {
            auto copy(*this);
            tuple_for_each<decrement_op_>(m_iterators);
            return copy;
        }

        /**
         *  Shift all iterators by offset
         */
        template<typename MyCategory_ = iterator_category>
        auto operator += (difference_type offset)
            -> std::enable_if_t<std::is_base_of<std::random_access_iterator_tag, MyCategory_>::value, this_type&>
        {
            tuple_for_each<advance_op_>(m_iterators, offset);
            return *this;
        }

        /**
         *  Shift all iterators by offset
         */
        template<typename MyCategory_ = iterator_category>
        auto operator -= (difference_type offset)
            -> std::enable_if_t<std::is_base_of<std::random_access_iterator_tag, MyCategory_>::value, this_type&>
        {
            tuple_for_each<advance_op_>(m_iterators, -offset);
            return *this;
        }

        /**
         *  Shift all iterators by offset
         */
        template<typename MyCategory_ = iterator_category>
        auto operator + (difference_type offset) const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, MyCategory_>::value, this_type>::type
        {
            this_type copy(*this);
            return (copy += offset);
        }

        /**
         *  Get distance between iterators
         *  Is computed using the first iterator in the tuple!
         */
        template<typename MyCategory_ = iterator_category>
        auto operator - (const this_type & other)  const
            -> std::enable_if_t<std::is_base_of<std::random_access_iterator_tag, MyCategory_>::value, difference_type>
        {
            return std::get<0>(m_iterators) - std::get<0>(other.m_iterators);
        }

        /**
         *  Compare equality
         *  Two zip iterators are equal if all iterators in the iterator tuple are equal 
         */
        bool operator == (const this_type & other) const
        {
            return !tuple_any_of<notequal_op_>(m_iterators, other.m_iterators);
        }

        /**
         *  Compare inequality
         *  Two zip iterators are not equal if any iterators in the tuple are not equal
         */
        bool operator != (const this_type & other) const
        {
            return tuple_any_of<notequal_op_>(m_iterators, other.m_iterators);
        }

        /**
         *  Compare less
         */
        template<typename MyCategory_ = iterator_category>
        auto operator < (const this_type & other) const
            -> std::enable_if_t<std::is_base_of<std::random_access_iterator_tag, MyCategory_>::value, bool>
        {
            return tuple_all_of<less_op_>(m_iterators, other.m_iterators);
        }

        /**
         *  Compare greater
         */
        template<typename MyCategory_ = iterator_category>
        auto operator > (const this_type & other) const
            -> std::enable_if_t<std::is_base_of<std::random_access_iterator_tag, MyCategory_>::value, bool>
        {
            return tuple_all_of<less_op_>(other.m_iterators, m_iterators);
        }

        /**
         *  Compare less or equal
         */
        template<typename MyCategory_ = iterator_category>
        auto operator <= (const this_type & other) const
            -> std::enable_if<std::is_base_of<std::random_access_iterator_tag, MyCategory_>::value, bool>
        {
            return !tuple_all_of<less_op_>(other.m_iterators, m_iterators);
        }

        /**
         *  Compare greater or equal
         */
        template<typename MyCategory_ = iterator_category>
        YATO_CONSTEXPR_FUNC
        auto operator >= (const this_type & other) const
            -> std::enable_if_t<std::is_base_of<std::random_access_iterator_tag, MyCategory_>::value, bool>
        {
            return !tuple_all_of<less_op_>(m_iterators, other.m_iterators);
        }


        template <typename... _SomeIterators>
        friend class zip_iterator;
    };

    template <typename... Iterators_>
    void swap(zip_iterator<Iterators_...> & one, zip_iterator<Iterators_...> & another) YATO_NOEXCEPT_KEYWORD
    {
        one.swap(another);
    }

    template<typename... Iterators_>
    YATO_CONSTEXPR_FUNC
    zip_iterator<Iterators_...> make_zip_iterator(const std::tuple<Iterators_...> & tuple)
    {
        return zip_iterator<Iterators_...>(tuple);
    }

    template<typename... Iterators_>
    YATO_CONSTEXPR_FUNC
    zip_iterator<Iterators_...> make_zip_iterator(std::tuple<Iterators_...> && tuple)
    {
        return zip_iterator<Iterators_...>(std::move(tuple));
    }

    template<typename... Iterators_>
    YATO_CONSTEXPR_FUNC
    zip_iterator<yato::remove_cvref_t<Iterators_>...> make_zip_iterator(Iterators_ && ... iters)
    {
        return zip_iterator<yato::remove_cvref_t<Iterators_>...>(std::make_tuple(std::forward<Iterators_>(iters)...));
    }
}

#endif //_YATO_ZIP_ITERATOR_H_
