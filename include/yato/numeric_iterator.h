/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_NUMERIC_ITERATOR_H_
#define _YATO_NUMERIC_ITERATOR_H_

#include "assertion.h"
#include "types.h"
#include "type_traits.h"

namespace yato
{
    
    /**
     *  Iterator for passing through a 'virtual' integer sequence
     *  Enumerates without pointing to any container
     */
    template <typename ValueType_>
    class numeric_iterator
    {
    public:
        using this_type = numeric_iterator<ValueType_>;

        using value_type        = ValueType_;
        using difference_type   = std::ptrdiff_t;
        using pointer           = std::add_pointer_t<std::add_const_t<ValueType_>>;
        using reference         = std::add_lvalue_reference_t<std::add_const_t<ValueType_>>;
        using iterator_category = std::random_access_iterator_tag;

    private:
        value_type m_value;

    public:
        YATO_CONSTEXPR_FUNC
        explicit
        numeric_iterator(value_type value) YATO_NOEXCEPT_OPERATOR(std::is_nothrow_move_constructible<value_type>::value)
            : m_value(std::move(value))
        { }

        numeric_iterator(const numeric_iterator&) = default;

        numeric_iterator(numeric_iterator&&) YATO_NOEXCEPT_OPERATOR(std::is_nothrow_move_constructible<value_type>::value) = default;

        numeric_iterator& operator=(const numeric_iterator&) = default;

        numeric_iterator& operator=(numeric_iterator&&) YATO_NOEXCEPT_OPERATOR(std::is_nothrow_move_assignable<value_type>::value) = default;

        ~numeric_iterator() = default;

        YATO_CONSTEXPR_FUNC
        reference operator*() const YATO_NOEXCEPT_KEYWORD
        {
            return m_value;
        }

        YATO_CONSTEXPR_FUNC
        pointer operator->() const YATO_NOEXCEPT_KEYWORD
        {
            return &m_value;
        }

        this_type & operator++()
        {
            // ToDo (a.gruzdev): Make max/min assert only for integral types
            YATO_ASSERT(m_value < std::numeric_limits<value_type>::max(), "yato::numeric_iterator is out of range");
            ++m_value;
            return *this;
        }

        this_type operator++(int)
        {
            YATO_ASSERT(m_value < std::numeric_limits<value_type>::max(), "yato::numeric_iterator is out of range");
            auto temp = *this;
            ++m_value;
            return temp;
        }

        this_type & operator--()
        {
            YATO_ASSERT(m_value > std::numeric_limits<value_type>::min(), "yato::numeric_iterator is out of range");
            --m_value;
            return *this;
        }

        this_type operator--(int)
        {
            YATO_ASSERT(m_value > std::numeric_limits<value_type>::min(), "yato::numeric_iterator is out of range");
            auto temp = *this;
            --m_value;
            return temp;
        }

        this_type & operator+=(difference_type offset)
        {
            YATO_ASSERT(m_value <= std::numeric_limits<value_type>::max() - yato::narrow_cast<value_type>(offset), "yato::numeric_iterator is out of range");
            m_value += yato::narrow_cast<value_type>(offset);
            return *this;
        }

        this_type operator+(difference_type offset) const
        {
            this_type tmp = *this;
            return (tmp += offset);
        }

        this_type & operator-=(difference_type offset)
        {
            YATO_ASSERT(m_value >= std::numeric_limits<value_type>::min() + yato::narrow_cast<value_type>(offset), "yato::numeric_iterator is out of range");
            m_value -= yato::narrow_cast<value_type>(offset);
            return *this;
        }

        YATO_CONSTEXPR_FUNC this_type operator-(difference_type offset) const
        {
            this_type tmp = *this;
            return (tmp -= offset);
        }

        YATO_CONSTEXPR_FUNC difference_type operator-(const this_type & right) const
        {
            return m_value - right.m_value;
        }

        YATO_CONSTEXPR_FUNC reference operator[](difference_type offset) const
        {
            return (*(*this + offset));
        }

        YATO_CONSTEXPR_FUNC bool operator!=(const this_type & other) const YATO_NOEXCEPT_KEYWORD
        {
            return m_value != other.m_value;
        }

        YATO_CONSTEXPR_FUNC bool operator==(const this_type & other) const YATO_NOEXCEPT_KEYWORD
        {
            return m_value == other.m_value;
        }

        YATO_CONSTEXPR_FUNC bool operator<(const this_type & right) const YATO_NOEXCEPT_KEYWORD
        {
            return m_value < right.m_value;
        }

        YATO_CONSTEXPR_FUNC bool operator>(const this_type & right) const YATO_NOEXCEPT_KEYWORD
        {
            return m_value > right.m_value;
        }

        YATO_CONSTEXPR_FUNC bool operator<=(const this_type & right) const YATO_NOEXCEPT_KEYWORD
        {
            return m_value <= right.m_value;
        }

        YATO_CONSTEXPR_FUNC bool operator>=(const this_type & right) const YATO_NOEXCEPT_KEYWORD
        {
            return m_value >= right.m_value;
        }
    };


}

#endif
