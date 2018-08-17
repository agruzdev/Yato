/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_TRANSFORM_ITERATOR_H_
#define _YATO_TRANSFORM_ITERATOR_H_

#include "type_traits.h"
#include "storage.h"

#ifndef YATO_TRANSFORM_ITER_SIZE
    #define YATO_TRANSFORM_ITER_SIZE (64)
#endif

namespace yato
{

    template <typename _UnaryFunction, typename _Iterator>
    class transform_iterator
    {
        static YATO_CONSTEXPR_VAR size_t _function_storage_size = YATO_TRANSFORM_ITER_SIZE;
    public:
        using unary_function_type = _UnaryFunction;
        using iterator_type = _Iterator;
        using my_type = transform_iterator<unary_function_type, iterator_type>;
        using unary_function_storage = storage<unary_function_type, _function_storage_size>;

        //-------------------------------------------------------
        // Definitions for iterator_traits
        /**
        * Category is same as iterator's category
        */
        using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;
        /**
         * Dereferencing the iterator yields the same type as calling the function
         */
        //using reference = typename yato::function_trait<unary_function_type>::result_type;
        using reference = typename std::conditional<!std::is_same<iterator_category, std::output_iterator_tag>::value,
            decltype((std::declval<unary_function_type>())(*(std::declval<iterator_type>()))), decltype(*(std::declval<iterator_type>()))>::type;
        /**
         * Value type preserves cv-qualifiers 
         */
        using value_type = typename std::remove_reference<reference>::type;
        /**
         * Pointer to value
         */
        using pointer = value_type*;
        /**
         * Difference type is same as iterator's difference type
         * Using std::ptrdiff_t for output_iterator_tag to avoid compilation error for operator +=
         */
        using difference_type = typename std::conditional<!std::is_same<iterator_category, std::output_iterator_tag>::value, 
            typename std::iterator_traits<iterator_type>::difference_type, std::ptrdiff_t>::type;
        //-------------------------------------------------------

    private:
        iterator_type m_iterator;
        unary_function_storage m_function;
        //-------------------------------------------------------

    public:
        /**
         *  Create from iterator and function
         */
        transform_iterator(const iterator_type & iter, const unary_function_type & function)
            : m_iterator(iter), m_function(function)
        { }
        
        /**
         *  Create from iterator and function
         */
        transform_iterator(iterator_type && iter, const unary_function_type & function)
            : m_iterator(std::move(iter)), m_function(function)
        { }

        /**
         *  Create from iterator and function
         */
        transform_iterator(const iterator_type & iter, unary_function_type && function)
            : m_iterator(iter), m_function(std::move(function))
        { }

        /**
         *  Create from iterator and function
         */
        transform_iterator(iterator_type && iter, unary_function_type && function)
            : m_iterator(std::move(iter)), m_function(std::move(function))
        { }

        /**
         *  Copy 
         */
        template<typename _AnotherUnaryFunction, typename _AnotherIterator>
        YATO_CONSTEXPR_FUNC
        transform_iterator(const transform_iterator<_AnotherUnaryFunction, _AnotherIterator> & other,
            typename std::enable_if<(std::is_convertible<_AnotherUnaryFunction, unary_function_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value)>::type* = nullptr)
            : m_iterator(other.m_iterator), m_function(unary_function_type(*other.m_function))
        { }
        
        YATO_CONSTEXPR_FUNC
        transform_iterator(const my_type & other)
            : m_iterator(other.m_iterator), m_function(other.m_function)
        { }

        /**
         *  Move
         */
        transform_iterator(my_type && other)
            : m_iterator(std::move(other.m_iterator)), m_function(std::move(other.m_function))
        { }

        /**
         *  Move
         */
        template<typename _AnotherUnaryFunction, typename _AnotherIterator>
        transform_iterator(transform_iterator<_AnotherUnaryFunction, _AnotherIterator> && other,
            typename std::enable_if<(std::is_convertible<_AnotherUnaryFunction, unary_function_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value)>::type* = nullptr)
            : m_iterator(std::move(other.m_iterator)), m_function(std::move(unary_function_type(*other.m_function)))
        { }

        /**
         *  Destroy
         */
        ~transform_iterator()
        { }

        /**
         *  Copy
         */
        template<typename _AnotherUnaryFunction, typename _AnotherIterator>
        auto operator = (const transform_iterator<_AnotherUnaryFunction, _AnotherIterator> & other)
            -> typename std::enable_if<(std::is_convertible<_AnotherUnaryFunction, unary_function_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value), my_type &>::type
        {
            my_type tmp(other);
            tmp.swap(*this);
            return *this;
        }

        my_type & operator = (const my_type & other)
        {
            if (this != &other) {
                my_type tmp(other);
                tmp.swap(*this);
            }
            return *this;
        }

        /**
         *  Swap iterators
         */
        void swap(my_type & other) YATO_NOEXCEPT_KEYWORD
        {
            using std::swap;
            if (this != &other) {
                swap(m_iterator, other.m_iterator);
                swap(m_function, other.m_function);
            }
        }

        /**
         *  Move
         */
        template<typename _AnotherUnaryFunction, typename _AnotherIterator>
        auto operator = (transform_iterator<_AnotherUnaryFunction, _AnotherIterator> && other)
            -> typename std::enable_if<(std::is_convertible<_AnotherUnaryFunction, unary_function_type>::value && std::is_convertible<_AnotherIterator, iterator_type>::value), my_type &>::type
        {
            m_iterator = std::move(other.m_iterator);
            m_function = std::move(unary_function_type(*other.m_function));
            return *this;
        }

        /**
         *  Move
         */
        my_type & operator = (my_type && other)
        {
            if (this != &other) {
                m_iterator = std::move(other.m_iterator);
                m_function = std::move(other.m_function);
            }
            return *this;
        }

        /**
         *  Dereference 
         */
        YATO_CONSTEXPR_FUNC
        const reference operator*() const
        {
            return (*m_function)(*m_iterator);
        }

        /**
         *  Dereference
         */
        reference operator*()
        {
            return (*m_function)(*m_iterator);
        }

        /**
         *  Increment
         */
        my_type & operator++ ()
        {
            ++m_iterator;
            return *this;
        }

        /**
         *  Increment
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
         *  Shift by offset
         */
        template<typename _MyCategory = iterator_category>
        auto operator += (difference_type offset)
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, my_type &>::type
        {
            m_iterator += offset;
            return *this;
        }

        /**
         *  Shift by offset
         */
        template<typename _MyCategory = iterator_category>
        auto operator -= (difference_type offset)
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, my_type &>::type
        {
            return this->operator+=(-offset);
        }

        /**
         *  Shift by offset
         */
        template<typename _MyCategory = iterator_category>
        auto operator + (difference_type offset) const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, my_type>::type
        {
            auto copy(*this);
            return (copy += offset);
        }

        /**
         *  Get distance 
         */
        template<typename _MyCategory = iterator_category>
        auto operator - (const my_type & other)  const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, difference_type>::type
        {
            return m_iterator - other.m_iterator;
        }

        /**
         *  Compare equality
         */
        YATO_CONSTEXPR_FUNC
        bool operator == (const my_type & other)  const
        {
            return m_iterator == other.m_iterator;
        }

        /**
         *  Compare inequality
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
            return m_iterator < other.m_iterator;
        }

        /**
         *  Compare greater
         */
        template<typename _MyCategory = iterator_category>
        YATO_CONSTEXPR_FUNC
        auto operator > (const my_type & other) const
            -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, _MyCategory>::value, bool>::type
        {
            return m_iterator > other.m_iterator;
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

        // Make any other iterator friend to access fields in copy operator
        template <typename _SomeFunction, typename _SomeIterator>
        friend class transform_iterator;
     };

    template<typename _UnaryFunction, typename _Iterator>
    inline void swap(transform_iterator<_UnaryFunction, _Iterator> & one, transform_iterator<_UnaryFunction, _Iterator> & another)
    {
        one.swap(another);
    }

    template<typename _UnaryFunction, typename _Iterator>
    YATO_CONSTEXPR_FUNC
    auto make_transform_iterator(_Iterator && iterator, _UnaryFunction && function)
        -> transform_iterator<typename std::remove_reference<_UnaryFunction>::type, typename std::remove_reference<_Iterator>::type>
    {
        return transform_iterator<typename std::remove_reference<_UnaryFunction>::type, typename std::remove_reference<_Iterator>::type>(std::forward<_Iterator>(iterator), std::forward<_UnaryFunction>(function));
    }

    template<typename _UnaryFunction, typename _Iterator>
    YATO_CONSTEXPR_FUNC
    auto make_transform_iterator(_Iterator && iterator)
        -> typename std::enable_if< std::is_constructible<_UnaryFunction>::value, transform_iterator<_UnaryFunction, typename std::remove_reference<_Iterator>::type> >::type
    {
        return transform_iterator<_UnaryFunction, typename std::remove_reference<_Iterator>::type>(std::forward<_Iterator>(iterator), _UnaryFunction());
    }

}

#endif
