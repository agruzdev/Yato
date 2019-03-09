/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/

#ifndef _YATO_CONTAINER_ND_H_
#define _YATO_CONTAINER_ND_H_

#include <iterator>
#include <type_traits>
#include "assertion.h"
#include "types.h"
#include "range.h"

#include "container_base.h"

namespace yato
{

    template <typename ValueType_, size_t Dimensions_, typename Implementation_>
    class const_container_nd
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType_ cannot be a reference");
        static_assert(Dimensions_ > 0, "Invalid dimensions");

        using this_type = const_container_nd<ValueType_, Dimensions_, Implementation_>;
    public:
        using value_type = ValueType_;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = Dimensions_;

        explicit
        operator const Implementation_&() const
        {
            return *this;
        }

        /**
         * Access sub-element
         */
        decltype(auto) operator[](size_t idx) const
        {
            return static_cast<const Implementation_*>(this)->operator[](idx);
        }

        /**
         *  Element access with bounds check
         */
        template<typename... Tail_>
        decltype(auto) at(size_t idx, Tail_... tail) const
        {
            return static_cast<const Implementation_*>(this)->at(idx, tail...);
        }

        /**
         * Chech is view data has no gaps, i.e. strides are equal to sizes
         * Plain iterators are valid only if view is continuous
         */
        bool continuous() const
        {
            return static_cast<const Implementation_*>(this)->continuous();
        }

        /**
         *  Get size of specified dimension
         */
        size_t size(size_t idx) const
        {
            return static_cast<const Implementation_*>(this)->size(idx);
        }

        /**
         *  Get stride of specified dimension. 
         *  Dimensions number for strides is less by one.
         */
        size_t stride(size_t idx) const
        {
            return static_cast<const Implementation_*>(this)->stride(idx);
        }

        /**
         * Get total number of elements
         */
        size_t total_size() const
        {
            return static_cast<const Implementation_*>(this)->total_size();
        }

        /**
         * Get total number of bytes counting strides
         */
        size_t total_stored() const
        {
            return static_cast<const Implementation_*>(this)->total_stored();
        }

        /**
         * Get dimensions array
         */
        decltype(auto) dimensions() const
        {
            return static_cast<const Implementation_*>(this)->dimensions();
        }

        /**
         * Get dimensions array
         */
        decltype(auto) strides() const
        {
            return static_cast<const Implementation_*>(this)->strides();
        }

        /**
         * Get dimensions range
         */
        decltype(auto) dimensions_range() const
        {
            return static_cast<const Implementation_*>(this)->dimensions_range();
        }

        /**
         * Get strides range
         */
        decltype(auto) strides_range() const
        {
            return static_cast<const Implementation_*>(this)->strides_range();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) cbegin() const
        {
            return static_cast<const Implementation_*>(this)->cbegin();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) cend() const
        {
            return static_cast<const Implementation_*>(this)->cend();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) plain_cbegin() const
        {
            return static_cast<const Implementation_*>(this)->plain_cbegin();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) plain_cend() const
        {
            return static_cast<const Implementation_*>(this)->plain_cend();
        }

        /**
         * Raw pointer to underlying data
         */
        decltype(auto) cdata() const
        {
            return static_cast<const Implementation_*>(this)->cdata();
        }
    };


    template <typename ValueType_, size_t Dimensions_, typename Implementation_>
    class container_nd
        : public const_container_nd<ValueType_, Dimensions_, Implementation_>
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType_ cannot be a reference");
        static_assert(Dimensions_ > 0, "Invalid dimensions");

        using this_type = container_nd<ValueType_, Dimensions_, Implementation_>;
    public:
        using value_type = ValueType_;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = Dimensions_;

        explicit
        operator Implementation_&() const
        {
            return *this;
        }

        /**
         * Access sub-element
         */
        decltype(auto) operator[](size_t idx) const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->operator[](idx);
        }

        /**
         *  Element access with bounds check
         */
        template<typename... Tail_>
        decltype(auto) at(size_t idx, Tail_... tail) const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->at(idx, tail...);
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) begin() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->begin();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) end() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->end();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) plain_begin() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->plain_begin();
        }

        /**
         * Iterator along the top dimension
         */
        decltype(auto) plain_end() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->plain_end();
        }

        /**
         * Raw pointer to underlying data
         */
        decltype(auto) data() const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->data();
        }

    };


    namespace details
    {
        template <typename ValueType_, size_t Dimensions_, typename Implementation_>
        using choose_container_interface_t = std::conditional_t<std::is_const<ValueType_>::value,
            yato::const_container_nd<std::remove_const_t<ValueType_>, Dimensions_, Implementation_>,
            yato::container_nd<ValueType_, Dimensions_, Implementation_>
        >;

    } // namespace details


    template <typename ValueType_, typename Implementation_>
    using const_container_1d = const_container_nd<ValueType_, 1, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using const_container_2d = const_container_nd<ValueType_, 2, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using const_container_3d = const_container_nd<ValueType_, 3, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using container_1d = container_nd<ValueType_, 1, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using container_2d = container_nd<ValueType_, 2, Implementation_>;

    template <typename ValueType_, typename Implementation_>
    using container_3d = container_nd<ValueType_, 3, Implementation_>;
}

#endif //_YATO_CONTAINER_ND_H_
