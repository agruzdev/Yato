/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
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

#if 0
    /**
     * Access to private interface of a container implementation
     */
    struct container_implementation_access
    {
        /**
         * Returns size of specified dimension
         * If the container is empty ( empty() returns true ) then calling for size(idx) returns 0 for idx = 0; Return value for any idx > 0 is undefined
         */
        template <typename Container_>
        static YATO_CONSTEXPR_FUNC_CXX14
        size_t size(const Container_* c, size_t idx) YATO_NOEXCEPT_KEYWORD
        {
            return c->size_(idx);
        }

        /**
         * Result of access by index on the top dimension. Implements operator[]
         */
        template <typename Container_>
        static YATO_CONSTEXPR_FUNC_CXX14
        decltype(auto) subscript(Container_* c, size_t offset) YATO_NOEXCEPT_KEYWORD
        {
            return c->subscript_(offset);
        }

        /**
         * Result of const access by index on the top dimension. Implements operator[] const
         */
        template <typename Container_>
        static YATO_CONSTEXPR_FUNC_CXX14
        decltype(auto) csubscript(const Container_* c, size_t offset) YATO_NOEXCEPT_KEYWORD
        {
            return c->csubscript_(offset);
        }
    };



    template <typename ValueType_, size_t Dimensions_>
    struct sampler_ops
    {

        template <size_t Depth_ = 0, typename Container_, typename Sampler_, typename... Tail_>
        auto cfetch(const Container_* c, Sampler_&& sampler, const typename yato::remove_cvref_t<Sampler_>::index_type& idx, Tail_&&... tail) const
            -> std::enable_if_t<(sizeof...(Tail_) + 1 == Dimensions_), typename yato::remove_cvref_t<Sampler_>::template const_return_type<ValueType_>>
        {
            size_t effective_idx;
            if (sampler.template transform_index<Depth_>(idx, container_implementation_access::size(c, 0), effective_idx)) {
                return container_implementation_access::csubscript(c, effective_idx).template cfetch<Depth_ + 1>(c, std::forward<Sampler_>(sampler), std::forward<Tail_>(tail)...);
            }
            else {
                return sampler.template boundary_cvalue<ValueType_>();
            }
        }

        template <size_t Depth_ = 0, typename Container_, typename Sampler_, typename... Tail_>
        auto fetch(Container_* c, Sampler_&& sampler, const typename yato::remove_cvref_t<Sampler_>::index_type& idx, Tail_&&... tail) const
            -> std::enable_if_t<(sizeof...(Tail_) + 1 == Dimensions_), typename yato::remove_cvref_t<Sampler_>::template return_type<ValueType_>>
        {
            size_t effective_idx;
            if (sampler.template transform_index<Depth_>(idx, container_implementation_access::size(c, 0), effective_idx)) {
                return container_implementation_access::subscript(c, effective_idx).template fetch<Depth_ + 1>(c, std::forward<Sampler_>(sampler), std::forward<Tail_>(tail)...);
            }
            else {
                throw yato::out_of_range_error("index is out of bounds");
            }
        }

    };

    template <typename ValueType_>
    struct sampler_ops<ValueType_, 1>
    {
        template <size_t Depth_ = 0, typename Container_, typename Sampler_>
        auto cfetch(const Container_* c, Sampler_&& sampler, const typename yato::remove_cvref_t<Sampler_>::index_type& idx) const
            -> typename yato::remove_cvref_t<Sampler_>::template const_return_type<ValueType_>
        {
            size_t effective_idx;
            if (sampler.template transform_index<Depth_>(idx, container_implementation_access::size(c, 0), effective_idx)) {
                return container_implementation_access::csubscript(c, effective_idx);
            }
            else {
                return sampler.template boundary_cvalue<ValueType_>();
            }
        }

        template <size_t Depth_ = 0, typename Container_, typename Sampler_>
        auto fetch(Container_* c, Sampler_&& sampler, const typename yato::remove_cvref_t<Sampler_>::index_type& idx) const
            -> typename yato::remove_cvref_t<Sampler_>::template return_type<ValueType_>
        {
            size_t effective_idx;
            if (sampler.template transform_index<Depth_>(idx, container_implementation_access::size(c, 0), effective_idx)) {
                return container_implementation_access::subscript(c, effective_idx);
            }
            else {
                throw yato::out_of_range_error("index is out of bounds");
            }
        }
    };
#endif

    template <typename ValueType_, size_t Dimensions_, typename Implementation_>
    class const_container_nd
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType_ cannot be a reference");
        static_assert(Dimensions_ > 0, "Invalid dimensions");

        using this_type = const_container_nd<ValueType_, Dimensions_, Implementation_>;

    public:
        using value_type = ValueType_;
        using implementation_type = Implementation_;
        using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
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
        template<typename... Indexes_>
        decltype(auto) at(Indexes_&&... indexes) const
        {
            return static_cast<const Implementation_*>(this)->at(std::forward<Indexes_>(indexes)...);
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
        using implementation_type = Implementation_;
        using reference = std::add_lvalue_reference_t<value_type>;
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
        template<typename... Indexes_>
        decltype(auto) at(Indexes_&&... indexes) const
        {
            return static_cast<Implementation_*>(const_cast<this_type*>(this))->at(std::forward<Indexes_>(indexes)...);
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
