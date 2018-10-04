/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ITERATOR_ND_H_
#define _YATO_ITERATOR_ND_H_

#include <iterator>
#include "assert.h"
#include "types.h"
#include "range.h"

#include "array_proxy.h"

namespace yato
{


    /**
     * Iterator over multidimensional view_nd
     */
    template <typename ValueType_, typename DimensionDescriptor_, size_t DimsNum_, proxy_access_policy AccessPolicy_>
    class iterator_nd
        : private proxy_nd<ValueType_, DimensionDescriptor_, DimsNum_, AccessPolicy_>
    {
        using this_type = iterator_nd<ValueType_, DimensionDescriptor_, DimsNum_, AccessPolicy_>;
        using view_type = proxy_nd<ValueType_, DimensionDescriptor_, DimsNum_, AccessPolicy_>;

        using data_value_type     = ValueType_;
        using data_pointer_type   = std::add_pointer_t<ValueType_>;
        using data_reference_type = typename proxy_access_traits<ValueType_, AccessPolicy_>::reference;

    public:

        using view_type::dimensions_number;
        using view_type::access_policy;

        // iterator traits
        using size_type         = size_t;
        using value_type        = view_type; // dereferencing returns itself for stl compatibility
        using pointer           = std::add_pointer_t<view_type>;
        using reference         = std::add_lvalue_reference_t<view_type>;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        iterator_nd(const view_type & v)
            : view_type(v)
        { }

        template <typename OtherTy_ = ValueType_, typename =
            std::enable_if_t<details::convertible_view_type<OtherTy_, data_value_type>::value>
        >
        iterator_nd(const iterator_nd<OtherTy_, typename view_type::dim_descriptor, view_type::dimensions_number, view_type::access_policy> & other)
            : view_type(other)
        { }

        template <proxy_access_policy ProxyAccess_, typename =
            std::enable_if_t<ProxyAccess_ != view_type::access_policy>
        >
        explicit
        iterator_nd(const iterator_nd<typename view_type::value_type, typename view_type::dim_descriptor, view_type::dimensions_number, ProxyAccess_> & other)
            : view_type(other)
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
            return sizeof(data_value_type) * view_type::total_size() == view_type::total_stored();
        }

        /**
         *  Return the current proxy
         *  Is necessary for supporting ranged 'for' 
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
            details::advance_bytes(view_type::raw_ptr_(), view_type::total_stored());
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
            details::advance_bytes(view_type::raw_ptr_(), -narrow_cast<difference_type>(view_type::total_stored()));
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
            details::advance_bytes(view_type::raw_ptr_(), offset * view_type::total_stored());
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
            details::advance_bytes(view_type::raw_ptr_(), -offset * narrow_cast<difference_type>(view_type::total_stored()));
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
            return one.view_type::raw_ptr_() == another.view_type::raw_ptr_();
        }

        /**
         *  Not equal
         */
        friend
        bool operator!=(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.view_type::raw_ptr_() != another.view_type::raw_ptr_();
        }

        /**
         *  Less
         */
        friend
        bool operator<(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.view_type::raw_ptr_() < another.view_type::raw_ptr_();
        }

        /**
         *  Greater
         */
        friend
        bool operator>(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.view_type::raw_ptr_() > another.view_type::raw_ptr_();
        }

        /**
         *  Less or equal
         */
        friend
        bool operator<=(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.view_type::raw_ptr_() <= another.view_type::raw_ptr_();
        }

        /**
         *  Greater or equal
         */
        friend
        bool operator>=(const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
        {
            return one.view_type::raw_ptr_() >= another.view_type::raw_ptr_();
        }


        template<typename, typename, size_t, proxy_access_policy>
        friend class iterator_nd;
    };


    template <typename ProxyValue_, typename ProxyDimension_, size_t ProxyDims_, proxy_access_policy ProxyAccess_>
    YATO_CONSTEXPR_FUNC
    auto make_move_iterator(const iterator_nd<ProxyValue_, ProxyDimension_, ProxyDims_, ProxyAccess_> & it)
        -> iterator_nd<ProxyValue_, ProxyDimension_, ProxyDims_, proxy_access_policy::rvalue_ref>
    {
        return iterator_nd<ProxyValue_, ProxyDimension_, ProxyDims_, proxy_access_policy::rvalue_ref>(it);
    }

}

#endif //_YATO_ITERATOR_ND_H_
