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


    /**
     * Access to private interface of a container implementation
     */
    template <typename ValueType_, size_t Dimensions_, typename Implementation_>
    class container_implementation_access
    {
    public:
        /**
         * Returns size of specified dimension
         */
        template <typename Container_>
        static
        size_t size(const Container_* c, size_t idx)
        {
            return static_cast<const Implementation_*>(c)->size(idx);
        }

        /**
         * Result of access by index on the top dimension. Implements operator[]
         */
        template <typename Container_>
        static
        decltype(auto) subscript(Container_* c, std::ptrdiff_t offset) YATO_NOEXCEPT_KEYWORD
        {
            return static_cast<Implementation_*>(c)->subscript_(offset);
        }

        /**
         * Result of const access by index on the top dimension. Implements operator[] const
         */
        template <typename Container_>
        static
        decltype(auto) csubscript(const Container_* c, std::ptrdiff_t offset) YATO_NOEXCEPT_KEYWORD
        {
            return static_cast<const Implementation_*>(c)->csubscript_(offset);
        }
    };



    template <typename ValueType_, size_t Dimensions_, typename Implementation_>
    class const_container_nd
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType_ cannot be a reference");
        static_assert(Dimensions_ > 0, "Invalid dimensions");

        using this_type = const_container_nd<ValueType_, Dimensions_, Implementation_>;
        using implementation_access = container_implementation_access<ValueType_, Dimensions_, Implementation_>;

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
        //template <typename... Tail_>
        //decltype(auto) at(size_t idx, Tail_... tail) const
        //{
        //    return static_cast<const Implementation_*>(this)->at(idx, tail...);
        //}

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
            //return static_cast<const Implementation_*>(this)->size(idx);
            return implementation_access::size(this, 0);
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



        /**
         *  Element access with a sampler
         */
        template <typename Sampler_, typename... Tail_>
        auto at(const typename Sampler_::index_type& idx, Tail_... tail) const
            -> typename Sampler_::template return_type<value_type>
        {
            const auto size_0 = implementation_access::size(this, 0);
            if (Sampler_::is_valid_index(idx, size_0)) {
                //return static_cast<const Implementation_*>(this)->operator[](Sampler_::wrap_index(idx, size_0)).at<Sampler_>(tail...);
                return implementation_access::csubscript(this, Sampler_::wrap_index(idx, size_0)).template at<Sampler_>(tail...);
            }
            else {
                return Sampler_::template boundary_value<value_type>();
            }
        }

        /**
         *  Element access with bounds check
         */
        template <typename... Idxs_>
        const_reference at(Idxs_&&... indexes) const
        {
            return this->at<yato::sampler_default>(std::forward<Idxs_>(indexes)...);
        }
    };


    template <typename ValueType_, typename Implementation_>
    class const_container_nd<ValueType_, 1, Implementation_>
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType_ cannot be a reference");

        using this_type = const_container_nd<ValueType_, 1, Implementation_>;
        using implementation_access = container_implementation_access<ValueType_, 1, Implementation_>;

    public:
        using value_type = ValueType_;
        using implementation_type = Implementation_;
        using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;

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
        //template <typename... Tail_>
        //decltype(auto) at(size_t idx, Tail_... tail) const
        //{
        //    return static_cast<const Implementation_*>(this)->at(idx, tail...);
        //}

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
            //return static_cast<const Implementation_*>(this)->size(idx);
            return implementation_access::size(this, 0);
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


        /**
         *  Element access with a sampler
         */
        template <typename Sampler_>
        auto at(const typename Sampler_::index_type& idx) const
            -> typename Sampler_::template return_type<value_type>
        {
            const auto size_0 = implementation_access::size(this, 0);
            if (Sampler_::is_valid_index(idx, size_0)) {
                //return static_cast<const Implementation_*>(this)->operator[](Sampler_::wrap_index(idx, size_0));
                return implementation_access::csubscript(this, Sampler_::wrap_index(idx, size_0));
            }
            else {
                return Sampler_::template boundary_value<value_type>();
            }
        }

        /**
         *  Element access with bounds check
         */
        template <typename Idx_>
        const_reference at(Idx_&& indexes) const
        {
            return this->at<yato::sampler_default>(std::forward<Idx_>(indexes));
        }
    };




    template <typename ValueType_, size_t Dimensions_, typename Implementation_>
    class container_nd
        : public const_container_nd<ValueType_, Dimensions_, Implementation_>
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType_ cannot be a reference");
        static_assert(Dimensions_ > 0, "Invalid dimensions");

        using this_type = container_nd<ValueType_, Dimensions_, Implementation_>;
        using implementation_access = container_implementation_access<ValueType_, Dimensions_, Implementation_>;

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

        ///**
        // *  Element access with bounds check
        // */
        //template<typename... Tail_>
        //decltype(auto) at(size_t idx, Tail_... tail) const
        //{
        //    return static_cast<Implementation_*>(const_cast<this_type*>(this))->at(idx, tail...);
        //}

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


        /**
         *  Element access with a sampler
         */
        template <typename... Tail_>
        reference at(size_t idx, Tail_... tail)
        {
            if (idx >= implementation_access::size(this, 0)) {
                throw yato::out_of_range_error("container_nd::at() index is out of bounds.");
            }
            //return static_cast<Implementation_*>(this)->operator[](idx).at(tail...);
            return implementation_access::subscript(this, idx).at(tail...);
        }

    };



    template <typename ValueType_, typename Implementation_>
    class container_nd<ValueType_, 1, Implementation_>
        : public const_container_nd<ValueType_, 1, Implementation_>
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType_ cannot be a reference");

        using this_type = container_nd<ValueType_, 1, Implementation_>;
        using implementation_access = container_implementation_access<ValueType_, 1, Implementation_>;

    public:
        using value_type = ValueType_;
        using implementation_type = Implementation_;
        using reference = std::add_lvalue_reference_t<value_type>;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;

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

        ///**
        // *  Element access with bounds check
        // */
        //template<typename... Tail_>
        //decltype(auto) at(size_t idx, Tail_... tail) const
        //{
        //    return static_cast<Implementation_*>(const_cast<this_type*>(this))->at(idx, tail...);
        //}

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


        /**
         *  Element access with a sampler
         */
        reference at(size_t idx)
        {
            if (idx >= implementation_access::size(this, 0)) {
                throw yato::out_of_range_error("container_nd::at() index is out of bounds.");
            }
            return implementation_access::subscript(this, idx);
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


#define YATO_IMPORT_CONTAINER_ND_INTERFACE(BaseType_) \
    friend class container_implementation_access<typename BaseType_::value_type, BaseType_::dimensions_number, typename BaseType_::implementation_type>; \
    using BaseType_::at;


}

#endif //_YATO_CONTAINER_ND_H_
