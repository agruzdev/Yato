/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ITERATOR_ND_H_
#define _YATO_ITERATOR_ND_H_

#include <iterator>
#include "assertion.h"
#include "types.h"
#include "range.h"

#include "array_proxy.h"

namespace yato
{


    /**
     * Iterator over multidimensional view_nd
     * Wraps ProxyType_ iterating underlying data accordint to proxy shape
     */
    template <typename ProxyType_>
    class iterator_nd
        : private ProxyType_
    {
        using this_type  = iterator_nd<ProxyType_>;
        using proxy_type = ProxyType_;

        using data_value_type     = typename ProxyType_::value_type;

    public:

        using proxy_type::dimensions_number;
        using proxy_type::access_policy;

        // iterator traits
        using size_type         = size_t;
        using value_type        = proxy_type; // dereferencing returns itself for stl compatibility
        using pointer           = std::add_pointer_t<proxy_type>;
        using reference         = std::add_lvalue_reference_t<proxy_type>;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        iterator_nd(const proxy_type & v)
            : proxy_type(v)
        { }

        template <typename OtherProxy_>
        iterator_nd(const iterator_nd<OtherProxy_> & other)
            : proxy_type(other)
        { }

        iterator_nd(const iterator_nd&) = default;
        iterator_nd(iterator_nd&&) YATO_NOEXCEPT_KEYWORD = default;

        iterator_nd& operator=(const iterator_nd&) = default;
        iterator_nd& operator=(iterator_nd&&) YATO_NOEXCEPT_KEYWORD = default;

        ~iterator_nd() = default;

        /**
         * Check that iterators range can have plain access
         */
        YATO_CONSTEXPR_FUNC
        bool makes_plain_range() const
        {
            return sizeof(data_value_type) * proxy_type::total_size() == proxy_type::total_stored();
        }

        /**
         *  Return the wrapped proxy
         */
        YATO_CONSTEXPR_FUNC
        reference operator*() const
        {
            return static_cast<reference>(*const_cast<this_type*>(this));
        }

        /**
         *  Increment iterator
         */
        YATO_CONSTEXPR_FUNC_CXX14
        this_type & operator++() YATO_NOEXCEPT_KEYWORD
        {
            details::advance_bytes(proxy_type::raw_ptr_(), proxy_type::total_stored());
            return *this;
        }

        /**
         *  Increment iterator
         */
        this_type & operator++(int)
        {
            this_type temp(*this);
            ++(*this);
            return temp;
        }

        /**
         *  Decrement iterator
         */
        YATO_CONSTEXPR_FUNC_CXX14
        this_type & operator--() YATO_NOEXCEPT_KEYWORD
        {
            details::advance_bytes(proxy_type::raw_ptr_(), -narrow_cast<difference_type>(proxy_type::total_stored()));
            return *this;
        }

        /**
         *  Decrement iterator
         */
        this_type & operator--(int)
        {
            this_type temp(*this);
            --(*this);
            return temp;
        }

        /**
         *  Add offset
         */
        this_type & operator+=(difference_type offset) YATO_NOEXCEPT_KEYWORD
        {
            details::advance_bytes(proxy_type::raw_ptr_(), offset * proxy_type::total_stored());
            return *this;
        }

        /**
         *  Add offset
         */
        friend 
        this_type operator+(const this_type & iter, difference_type offset)
        {
            this_type temp(iter);
            return (temp += offset);
        }

        /**
         *  Add offset
         */
        friend
        this_type operator+(difference_type offset, const this_type & iter)
        {
            return iter + offset;
        }

        /**
         *  Subtract offset
         */
        this_type & operator-=(difference_type offset) YATO_NOEXCEPT_KEYWORD
        {
            details::advance_bytes(proxy_type::raw_ptr_(), -offset * narrow_cast<difference_type>(proxy_type::total_stored()));
            return *this;
        }

        /**
         *  Subtract offset
         */
        friend 
        this_type operator-(const this_type & iter, difference_type offset)
        {
            this_type temp(iter);
            return (temp -= offset);
        }

        /**
         *  Distance between iterators
         *  Can be computed only between iterators of the same container. Otherwise result is undefined
         */
        friend
        difference_type operator-(const this_type & one, const this_type & another)
        {
            YATO_REQUIRES(one.total_stored() == another.total_stored());
            using byte_pointer = const volatile uint8_t*;
            return (yato::pointer_cast<byte_pointer>(one.raw_ptr_()) - yato::pointer_cast<byte_pointer>(another.raw_ptr_())) / one.total_stored();
        }

        /**
         *  Equal
         */
        friend
        bool operator==(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.proxy_type::raw_ptr_() == another.proxy_type::raw_ptr_();
        }

        /**
         *  Not equal
         */
        friend
        bool operator!=(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.proxy_type::raw_ptr_() != another.proxy_type::raw_ptr_();
        }

        /**
         *  Less
         */
        friend
        bool operator<(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.proxy_type::raw_ptr_() < another.proxy_type::raw_ptr_();
        }

        /**
         *  Greater
         */
        friend
        bool operator>(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.proxy_type::raw_ptr_() > another.proxy_type::raw_ptr_();
        }

        /**
         *  Less or equal
         */
        friend
        bool operator<=(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.proxy_type::raw_ptr_() <= another.proxy_type::raw_ptr_();
        }

        /**
         *  Greater or equal
         */
        friend
        bool operator>=(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.proxy_type::raw_ptr_() >= another.proxy_type::raw_ptr_();
        }


        template<typename>
        friend class iterator_nd;
    };


    template <typename ProxyValue_, typename ProxyDimension_, size_t ProxyDims_, proxy_access_policy ProxyAccess_>
    YATO_CONSTEXPR_FUNC
    auto make_iterator(const proxy_nd<ProxyValue_, ProxyDimension_, ProxyDims_, ProxyAccess_> & p)
    {
        return iterator_nd<proxy_nd<ProxyValue_, ProxyDimension_, ProxyDims_, ProxyAccess_>>(p);
    }

    template <typename ProxyType_>
    YATO_CONSTEXPR_FUNC
    auto make_move_iterator(const iterator_nd<ProxyType_> & it)
    {
        using move_proxy = typename ProxyType_::template rebind_access_t<proxy_access_policy::rvalue_ref>;
        return iterator_nd<move_proxy>(it);
    }

}

#endif //_YATO_ITERATOR_ND_H_
