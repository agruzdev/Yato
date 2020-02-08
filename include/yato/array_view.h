/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_VIEW_H_
#define _YATO_ARRAY_VIEW_H_

#include <array>
#include <initializer_list>
#include <vector>

#include "array_proxy.h"
#include "iterator_nd.h"
#include "tuple.h"

namespace yato
{
    namespace details
    {

        // Multidimensional case
        template<typename ValueType, size_t DimsNum>
        class array_view_base
        {
            using this_type = array_view_base<ValueType, DimsNum>;
        public:

            using value_type        = ValueType;
            using size_type         = size_t;
            using dimensions_type   = dimensionality<DimsNum, size_t>;

            using element_strides_type  = dimensionality<DimsNum - 1, size_t>;
            using byte_strides_type     = strides_array<DimsNum - 1, size_t>;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = DimsNum;
            static_assert(dimensions_number > 1, "Dimensions number should be greater than 1");

            using value_iterator  = std::add_pointer_t<value_type>;
            using value_reference = std::add_lvalue_reference_t<value_type>;

            using dim_descriptor = dimension_descriptor_strided<size_type>; // size, stride, offset

            using sub_view       = proxy_nd<value_type, dim_descriptor, dimensions_number - 1>;

            using reference       = sub_view;
            using const_reference = sub_view;
            using iterator        = iterator_nd<sub_view>;
            using const_iterator  = iterator_nd<sub_view>;

            using plain_iterator        = std::add_pointer_t<value_type>;
            using const_plain_iterator  = std::add_pointer_t<std::add_const_t<value_type>>;
            //--------------------------------------------------------------------

            value_iterator m_base_ptr;
            std::array<dim_descriptor::type, dimensions_number> m_descriptors;
            //--------------------------------------------------------------------

            array_view_base(value_type* ptr, const dimensions_type & extents, const element_strides_type & strides)
                : m_base_ptr(ptr)
            {
                YATO_REQUIRES(extents[dimensions_number - 1] <= strides[dimensions_number - 2]);
                m_descriptors[dimensions_number - 1] = std::make_tuple(extents[dimensions_number - 1], extents[dimensions_number - 1], strides[dimensions_number - 2] * sizeof(value_type));
                for (size_t i = dimensions_number - 1; i > 0; --i) {
                    YATO_REQUIRES((i > 1) ? extents[i - 1] <= strides[i - 2] : true);
                    m_descriptors[i - 1] = std::make_tuple(
                        extents[i - 1],
                        extents[i - 1] * std::get<dim_descriptor::idx_total>(m_descriptors[i]),
                        (i > 1 ? strides[i - 2] : extents[0]) * std::get<dim_descriptor::idx_offset>(m_descriptors[i]));
                }
            }

            array_view_base(value_type* ptr, const dimensions_type & extents, const byte_strides_type & strides)
                : m_base_ptr(ptr)
            {
                YATO_REQUIRES(extents[dimensions_number - 1] <= strides[dimensions_number - 2]);
                m_descriptors[dimensions_number - 1] = std::make_tuple(extents[dimensions_number - 1], extents[dimensions_number - 1], strides[dimensions_number - 2]);
                for (size_t i = dimensions_number - 1; i > 0; --i) {
                    YATO_REQUIRES((i > 1) ? extents[i - 1] <= strides[i - 2] : true);
                    m_descriptors[i - 1] = std::make_tuple(
                        extents[i - 1],
                        extents[i - 1] * std::get<dim_descriptor::idx_total>(m_descriptors[i]),
                        (i > 1 ? strides[i - 2] : (extents[0] * std::get<dim_descriptor::idx_offset>(m_descriptors[i]))));
                }
            }

            template <typename ProxyValue_, typename ProxyDescriptor_>
            array_view_base(const proxy_nd<ProxyValue_, ProxyDescriptor_, dimensions_number> & proxy)
                : m_base_ptr(proxy.data())
            {
                using proxy_descriptor_type = typename yato::remove_cvref_t<decltype(proxy)>::dim_descriptor;
                using proxy_data_type = typename yato::remove_cvref_t<decltype(proxy)>::value_type;
                YATO_REQUIRES(proxy.descriptors_range_().distance() == dimensions_number);
                auto desc_it = proxy.descriptors_range_().begin();
                for (size_t i = 0; i < dimensions_number; ++i, ++desc_it) {
                    m_descriptors[i] = std::make_tuple(
                        std::get<proxy_descriptor_type::idx_size>(*desc_it),
                        std::get<proxy_descriptor_type::idx_total>(*desc_it),
                        proxy_descriptor_type::template offset_to_bytes<proxy_data_type>(std::get<proxy_descriptor_type::idx_offset>(*desc_it))
                    );
                }
            }

            ~array_view_base() = default;
            
            array_view_base(const this_type&) = default;
            array_view_base& operator= (const this_type&) = default;

            array_view_base(this_type&&) = default;
            array_view_base& operator= (this_type&&) = default;

            template <typename OtherTy_, typename = 
                std::enable_if_t<convertible_view_type<OtherTy_, value_type>::value>>
            array_view_base(const array_view_base<OtherTy_, dimensions_number> & other)
                : m_base_ptr(other.m_base_ptr), m_descriptors(other.m_descriptors)
            { }

            template <typename OtherTy_, typename = 
                std::enable_if_t<convertible_view_type<OtherTy_, value_type>::value>>
            array_view_base& operator=(const array_view_base<OtherTy_, dimensions_number> & other)
            {
                m_base_ptr    = other.m_base_ptr;
                m_descriptors = other.m_descriptors;
                return *this;
            }

            auto to_view_() const
            {
                return proxy_nd<std::add_const_t<value_type>, dim_descriptor, dimensions_number>(m_base_ptr, &m_descriptors[0]);
            }

            auto to_view_()
            {
                return proxy_nd<value_type, dim_descriptor, dimensions_number>(m_base_ptr, &m_descriptors[0]);
            }

            std::add_pointer_t<value_type> get_pointer_() const
            {
                return m_base_ptr;
            }

            void set_pointer_(std::add_pointer_t<value_type> ptr)
            {
                m_base_ptr = ptr;
            }

            size_type get_size_(size_t idx) const
            {
                YATO_REQUIRES(idx < dimensions_number);
                return std::get<dim_descriptor::idx_size>(m_descriptors[idx]);
            }

            size_type get_stride_(size_t idx) const
            {
                YATO_REQUIRES(idx < dimensions_number - 1);
                return std::get<dim_descriptor::idx_offset>(m_descriptors[idx + 1]);
            }

            size_type get_total_size_() const
            {
                return std::get<dim_descriptor::idx_total>(m_descriptors[0]);
            }

            size_type get_total_stored_() const
            {
                return std::get<dim_descriptor::idx_offset>(m_descriptors[0]);
            }

            auto get_dims_range_() const
            {
                return yato::make_range(m_descriptors).map(tuple_cgetter<dim_descriptor::idx_size>());
            }

            auto get_strides_range_() const
            {
                return yato::make_range(m_descriptors).tail().map([](const typename dim_descriptor::type & d) {
                    return dim_descriptor::template offset_to_bytes<value_type>(std::get<dim_descriptor::idx_offset>(d));
                });
            }

            sub_view get_sub_view_(size_t idx) const
            {
                value_iterator sub_view_ptr{ m_base_ptr };
                details::advance_bytes(sub_view_ptr, idx * dim_descriptor::offset_to_bytes<value_type>(std::get<dim_descriptor::idx_offset>(m_descriptors[1])));
                return sub_view(sub_view_ptr, &(m_descriptors[1]));
            }

            iterator get_iterator_(size_t idx) const
            {
                return static_cast<iterator>(get_sub_view_(idx));
            }

            const_iterator get_const_iterator_(size_t idx) const
            {
                return static_cast<const_iterator>(get_sub_view_(idx));
            }

            template<typename... IdxTail>
            value_reference at_impl_(size_t idx, IdxTail && ... tail) const
            {
                static_assert(sizeof...(IdxTail) + 1 == dimensions_number, "Wrong indexes number");
                if (idx >= get_size_(0)) {
                    throw yato::out_of_range_error("yato::array_view_nd[at]: out of range!");
                }
                return get_sub_view_(idx).at(std::forward<IdxTail>(tail)...);
            }

            bool is_continuous_() const
            {
                return get_total_size_() * sizeof(value_type) == get_total_stored_();
            }
        };

        // Single dimension case
        template<typename ValueType>
        class array_view_base<ValueType, 1>
        {
            using this_type = array_view_base<ValueType, 1>;

        public:
            using value_type        = ValueType;
            using size_type         = size_t;
            using dimensions_type   = dimensionality<1, size_t>;

            using element_strides_type  = dimensionality<0, size_t>;
            using byte_strides_type     = strides_array<0, size_t>;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;

            using dim_descriptor = dimension_descriptor_strided<size_type>; // size, stride, offset

            using value_iterator  = std::add_pointer_t<value_type>;
            using value_reference = std::add_lvalue_reference_t<value_type>;

            using iterator        = std::add_pointer_t<value_type>;
            using const_iterator  = std::add_pointer_t<std::add_const_t<value_type>>;
            using reference       = std::add_lvalue_reference_t<value_type>;
            using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;

            using plain_iterator        = std::add_pointer_t<value_type>;
            using const_plain_iterator  = std::add_pointer_t<std::add_const_t<value_type>>;
            //--------------------------------------------------------------------

            value_iterator  m_base_ptr;
            dimensions_type m_size;
            //--------------------------------------------------------------------

            array_view_base(value_type* ptr, const dimensions_type & extents, const element_strides_type &)
                : m_base_ptr(ptr), m_size(extents)
            { }

            array_view_base(value_type* ptr, const dimensions_type & extents, const byte_strides_type &)
                : m_base_ptr(ptr), m_size(extents)
            { }

            template <typename ProxyValue_, typename ProxyDescriptor_>
            array_view_base(const proxy_nd<ProxyValue_, ProxyDescriptor_, dimensions_number> & proxy)
                : m_base_ptr(proxy.data()), m_size(proxy.size(0))
            { }

            ~array_view_base() = default;

            array_view_base(const this_type&) = default;
            array_view_base& operator= (const this_type&) = default;

            array_view_base(this_type&&) = default;
            array_view_base& operator= (this_type&&) = default;

            template <typename OtherTy_, typename = 
                std::enable_if_t<convertible_view_type<OtherTy_, value_type>::value>>
            array_view_base(const array_view_base<OtherTy_, dimensions_number> & other)
                : m_base_ptr(other.m_base_ptr), m_size(other.m_size)
            { }

            template <typename OtherTy_, typename = 
                std::enable_if_t<convertible_view_type<OtherTy_, value_type>::value>>
            array_view_base& operator=(const array_view_base<OtherTy_, dimensions_number> & other)
            {
                m_base_ptr = other.m_base_ptr;
                m_size     = other.m_size;
                return *this;
            }

            auto to_view_() const
            {
                return proxy_nd<std::add_const_t<value_type>, dim_descriptor, dimensions_number>(m_base_ptr, &m_size, &m_size);
            }

            auto to_view_()
            {
                return proxy_nd<value_type, dim_descriptor, dimensions_number>(m_base_ptr, &m_size, &m_size);
            }

            std::add_pointer_t<value_type> get_pointer_() const
            {
                return m_base_ptr;
            }

            void set_pointer_(std::add_pointer_t<value_type> ptr)
            {
                m_base_ptr = ptr;
            }

            size_type get_size_(size_t idx) const
            {
                YATO_MAYBE_UNUSED(idx);
                YATO_REQUIRES(idx == 0);
                return m_size[0];
            }

            size_type get_stride_(size_t idx) const
            {
                YATO_MAYBE_UNUSED(idx);
                YATO_REQUIRES(false && "no strides for 1D");
                return 0;
            }

            size_type get_total_size_() const
            {
                return m_size[0];
            }

            size_type get_total_stored_() const
            {
                return m_size[0] * sizeof(value_type);
            }

            auto get_dims_range_() const
            {
                return yato::make_range(m_size.cbegin(), m_size.cend());
            }

            // returns empty range
            auto get_strides_range_() const
            {
                return yato::range<const size_type*>(nullptr, nullptr);
            }

            value_reference get_sub_view_(size_t idx) const
            {
                return *std::next(m_base_ptr, idx);
            }

            iterator get_iterator_(size_t idx) const
            {
                return std::next(m_base_ptr, idx);
            }

            const_iterator get_const_iterator_(size_t idx) const
            {
                return std::next(m_base_ptr, idx);
            }

            value_reference at_impl_(size_t idx) const
            {
                if (idx >= m_size[0]) {
                    throw yato::out_of_range_error("yato::array_view_nd[at]: out of range!");
                }
                return get_sub_view_(idx);
            }

            bool is_continuous_() const
            {
                return true;
            }
        };

        // Dimensionality should be positive
        template<typename ValueType>
        class array_view_base<ValueType, 0>
        { };
    }

    /**
     *  Non-owning light-weight container for contiguous data 
     */
    template<typename ValueType_, size_t DimsNum>
    class array_view_nd
        : public details::choose_container_interface_t<ValueType_, DimsNum, array_view_nd<ValueType_, DimsNum>>
        , private details::array_view_base<ValueType_, DimsNum>
    {
    public:
        using this_type = array_view_nd<ValueType_, DimsNum>;
        using base_type = details::array_view_base<ValueType_, DimsNum>;

        using value_type = ValueType_;
        using pointer_type = std::add_pointer_t<ValueType_>;
        using const_pointer_type = std::add_pointer_t<std::add_const_t<ValueType_>>;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = DimsNum;

        using typename base_type::dimensions_type;
        using typename base_type::dim_descriptor;
        using typename base_type::element_strides_type;
        using typename base_type::byte_strides_type;
        using typename base_type::size_type;
        using typename base_type::value_iterator;
        using typename base_type::value_reference;
        using typename base_type::reference;
        using typename base_type::const_reference;
        using typename base_type::iterator;
        using typename base_type::const_iterator;
        using typename base_type::plain_iterator;
        using typename base_type::const_plain_iterator;
        //-------------------------------------------------------

        array_view_nd()
            : base_type(nullptr, dimensions_type{}, element_strides_type{})
        { }

        array_view_nd(pointer_type ptr, const dimensions_type & extents)
            : base_type(ptr, extents, extents.sub_dims())
        { }

        array_view_nd(pointer_type ptr, const dimensions_type & extents, const element_strides_type & element_strides)
            : base_type(ptr, extents, element_strides)
        { }

        array_view_nd(pointer_type ptr, const dimensions_type & extents, const byte_strides_type & byte_strides)
            : base_type(ptr, extents, byte_strides)
        { }

        array_view_nd(const this_type & other) = default;
        array_view_nd& operator=(const this_type &) = default;

        array_view_nd(this_type &&) = default;
        array_view_nd& operator=(this_type && other) = default;

        ~array_view_nd() = default;

        /**
         * Convert view<T> to view<const T>
         */
        template <typename OtherTy_, typename = 
            std::enable_if_t<details::convertible_view_type<OtherTy_, value_type>::value>>
        array_view_nd(const array_view_nd<OtherTy_, dimensions_number> & other)
            : base_type(other)
        { }

        /**
         * Assign view<T> to view<const T>
         */
        template <typename OtherTy_, typename = 
            std::enable_if_t<details::convertible_view_type<OtherTy_, value_type>::value>>
        array_view_nd& operator=(const array_view_nd<OtherTy_, dimensions_number> & other)
        {
            base_type::operator=(other);
            return *this;
        }

        /**
         *  Create from proxy
         */
        template<typename ProxyValue_, typename ProxyDescriptor_, typename = 
            std::enable_if_t<std::is_same<ProxyValue_, value_type>::value>>
        array_view_nd(const proxy_nd<ProxyValue_, ProxyDescriptor_, dimensions_number> & proxy)
            : base_type(proxy)
        { }

        /**
         *  Create from proxy
         */
        template<typename ProxyValue_, typename ProxyDescriptor_, typename = 
            std::enable_if_t<std::is_same<ProxyValue_, value_type>::value>>
        array_view_nd& operator=(const proxy_nd<ProxyValue_, ProxyDescriptor_, dimensions_number> & proxy)
        {
            base_type::operator=(proxy);
            return *this;
        }

        /**
         * Create a new array view on the same data but with another shape
         * Total size should be unchanged
         * Only continuous view can be reshaped
         */
        template <size_t NewDimsNum>
        auto reshape(const dimensionality<NewDimsNum, size_type> & extents) const
            -> array_view_nd<value_type, NewDimsNum>
        {
            YATO_REQUIRES(continuous());
            YATO_REQUIRES(extents.total_size() == base_type::get_total_size_());
            return array_view_nd<value_type, NewDimsNum>(base_type::get_pointer_(), extents);
        }

        /**
         * Convert to view for the full array
         */
        operator proxy_nd<value_type, dim_descriptor, dimensions_number>() const
        {
            return base_type::to_view_();
        }

        reference operator[](size_t idx) const
        {
            YATO_REQUIRES(idx < base_type::get_size_(0));
            return base_type::get_sub_view_(idx);
        }

        template <typename... Indexes>
        auto at(Indexes &&... indexes) const
            -> typename std::enable_if<(sizeof...(Indexes) == dimensions_number), value_reference>::type
        {
            return base_type::at_impl_(std::forward<Indexes>(indexes)...);
        }

        size_type total_size() const
        {
            return base_type::get_total_size_();
        }

        /** 
         * returns size along specified dimension
         */
        size_type size(size_t idx) const
        {
            return base_type::get_size_(idx);
        }

        /**
         * Returns top dimension size
         */
        size_type size() const
        {
            return base_type::get_size_(0);
        }

        /**
         * Returns total view memory range including strides
         */
        YATO_DEPRECATED("Legacy naming. Use print_size()")
        size_type total_stored() const
        {
            return base_type::get_total_stored_();
        }

        /**
         * Returns total view memory range including strides
         */
        size_type print_size() const
        {
            return base_type::get_total_stored_();
        }

        /**
         * Get byte offset till next sub-view
         * Returns size in bytes for 1D view
         */
        size_type stride(size_t idx) const
        {
            return base_type::get_stride_(idx);
        }

        /**
         * Get dimensions
         */
        dimensions_type dimensions() const
        {
            return dimensions_type(base_type::get_dims_range_());
        }

        /**
         * Get dimensions
         */
        strides_array<dimensions_number - 1, size_type> strides() const
        {
            return byte_strides_type(base_type::get_strides_range_());
        }

        /**
         * Get number of dimensions
         */
        size_t dimensions_num() const
        {
            return dimensions_number;
        }
      
        /**
         *  Get dimensions range
         */
        auto dimensions_range() const
        {
            return base_type::get_dims_range_();
        }

        /**
         *  Get dimensions range
         */
        auto strides_range() const
        {
            return base_type::get_strides_range_();
        }

        bool empty() const
        {
            return (base_type::get_pointer_() == nullptr) || (base_type::get_total_size_() == 0);
        }

        const_iterator cbegin() const
        {
            return base_type::get_const_iterator_(0);
        }

        iterator begin() const
        {
            return base_type::get_iterator_(0);
        }

        const_iterator cend() const
        {
            return base_type::get_const_iterator_(size(0));
        }

        iterator end() const
        {
            return base_type::get_iterator_(size(0));
        }

        const_plain_iterator plain_cbegin() const
        {
            YATO_REQUIRES(continuous());
            return base_type::get_pointer_();
        }

        plain_iterator plain_begin() const
        {
            YATO_REQUIRES(continuous());
            return base_type::get_pointer_();
        }

        const_plain_iterator plain_cend() const
        {
            YATO_REQUIRES(continuous());
            return base_type::get_pointer_() + base_type::get_total_size_();
        }

        plain_iterator plain_end() const
        {
            YATO_REQUIRES(continuous());
            return base_type::get_pointer_() + base_type::get_total_size_();
        }

        yato::range<const_iterator> crange() const
        {
            return make_range(cbegin(), cend());
        }

        yato::range<iterator> range() const
        {
            return make_range(begin(), end());
        }

        yato::range<const_plain_iterator> plain_crange() const
        {
            YATO_REQUIRES(continuous());
            return make_range(plain_cbegin(), plain_cend());
        }

        yato::range<plain_iterator> plain_range() const
        {
            YATO_REQUIRES(continuous());
            return make_range(plain_begin(), plain_end());
        }

        /**
         * Chech is view data has no gaps, i.e. strides are equal to sizes
         * Plain iterators are valid only if view is continuous
         */
        bool continuous() const
        {
            return base_type::is_continuous_();
        }

        /**
         * Changes base pointer and returnes the old one not changing view extents
         */
        pointer_type rebind(pointer_type new_ptr)
        {
            auto tmp = base_type::get_pointer_();
            base_type::set_pointer_(new_ptr);
            return tmp;
        }

        /**
         * Get raw pointer to underlying data
         */
        pointer_type data() const
        {
            return base_type::get_pointer_();
        }

        /**
         * Get raw pointer to underlying data
         */
        const_pointer_type cdata() const
        {
            return base_type::get_pointer_();
        }

        // For view<T> to view<const/volatile T> conversion
        template <typename OtherTy_, size_t DimsNum_>
        friend class array_view_nd;
    };


    template<typename _DataType>
    using array_view = array_view_nd<_DataType, 1>;

    template<typename _DataType>
    using array_view_1d = array_view_nd<_DataType, 1>;

    template<typename _DataType>
    using array_view_2d = array_view_nd<_DataType, 2>;

    template<typename _DataType>
    using array_view_3d = array_view_nd<_DataType, 3>;

    template<typename _DataType>
    using array_view_4d = array_view_nd<_DataType, 4>;


    template<typename Ty_, size_t Size_>
    auto view(Ty_ (& arr)[Size_])
    {
        return array_view<Ty_>(arr, yato::dims(Size_));
    }

    template<typename Ty_, size_t Size_>
    auto view(std::array<Ty_, Size_> & arr)
    {
        return array_view<Ty_>(arr.data(), yato::dims(Size_));
    }

    template<typename Ty_>
    auto view(std::vector<Ty_> & vec) 
        -> std::enable_if_t<!std::is_same<Ty_, bool>::value, array_view<Ty_>>
    {
        return array_view<Ty_>(vec.data(), yato::dims(vec.size()));
    }

    template<typename Ty_, size_t Size1_, size_t Size2_>
    auto view(Ty_(&arr)[Size1_][Size2_])
    {
        return array_view_nd<Ty_, 2>(&arr[0][0], yato::dims(Size1_, Size2_));
    }

    template<typename Ty_, size_t Size1_, size_t Size2_, size_t Size3_>
    auto view(Ty_(&arr)[Size1_][Size2_][Size3_])
    {
        return array_view_nd<Ty_, 3>(&arr[0][0][0], yato::dims(Size1_, Size2_, Size3_));
    }

    template<typename Ty_, size_t Dims_ , typename Impl_>
    auto view(const container_nd<Ty_, Dims_, Impl_> & c)
    {
        return array_view_nd<Ty_, Dims_>(c.data(), c.dimensions(), c.strides());
    }

    template <typename Ty_, size_t Dims_>
    array_view_nd<Ty_, Dims_> & view(array_view_nd<Ty_, Dims_> & v)
    {
        return v;
    }


    template<typename Ty_, size_t Size_>
    auto cview(Ty_ (& arr)[Size_]) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<std::add_const_t<Ty_>>(arr, yato::dims(Size_));
    }

    template<typename Ty_, size_t Size_>
    auto cview(const std::array<Ty_, Size_> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<std::add_const_t<Ty_>>(arr.data(), yato::dims(Size_));
    }

    template<typename Ty_>
    auto cview(const std::vector<Ty_> & vec) YATO_NOEXCEPT_KEYWORD 
        -> std::enable_if_t<!std::is_same<Ty_, bool>::value, array_view<std::add_const_t<Ty_>>>
    {
        return array_view<std::add_const_t<Ty_>>(vec.data(), yato::dims(vec.size()));
    }

    template<typename Ty_, size_t Size1_, size_t Size2_>
    auto cview(Ty_(&arr)[Size1_][Size2_])
    {
        return array_view_nd<std::add_const_t<Ty_>, 2>(&arr[0][0], yato::dims(Size1_, Size2_));
    }

    template<typename Ty_, size_t Size1_, size_t Size2_, size_t Size3_>
    auto cview(Ty_(&arr)[Size1_][Size2_][Size3_])
    {
        return array_view_nd<std::add_const_t<Ty_>, 3>(&arr[0][0][0], yato::dims(Size1_, Size2_, Size3_));
    }

    template<typename Ty_, size_t Dims_ , typename Impl_>
    auto cview(const const_container_nd<Ty_, Dims_, Impl_> & c)
    {
        return array_view_nd<std::add_const_t<Ty_>, Dims_>(c.cdata(), c.dimensions(), c.strides());
    }

    template <typename Ty_, size_t Dims_>
    auto cview(const array_view_nd<Ty_, Dims_> & v)
        -> std::enable_if_t<!std::is_const<Ty_>::value, array_view_nd<std::add_const_t<Ty_>, Dims_>>
    {
        return static_cast<array_view_nd<std::add_const_t<Ty_>, Dims_>>(v);
    }

    template <typename Ty_, size_t Dims_>
    auto cview(array_view_nd<Ty_, Dims_> & v)
        -> std::enable_if_t<std::is_const<Ty_>::value, array_view_nd<Ty_, Dims_> & >
    {
        return v;
    }

    template <typename Ty_, size_t Dims_>
    auto cview(const array_view_nd<Ty_, Dims_> & v)
        -> std::enable_if_t<std::is_const<Ty_>::value, const array_view_nd<Ty_, Dims_> & >
    {
        return v;
    }



    template<typename T, size_t Size>
    YATO_DEPRECATED("Use yato::view/yato::cview")
    array_view<T> make_view(T (& arr)[Size]) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<T>(arr, yato::dims(Size));
    }

    template<typename T, size_t Size>
    YATO_DEPRECATED("Use yato::view/yato::cview")
    array_view<T> make_view(std::array<T, Size> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<T>(arr.data(), yato::dims(Size));
    }

    template<typename T, size_t Size>
    YATO_DEPRECATED("Use yato::view/yato::cview")
    array_view<const T> make_view(const std::array<T, Size> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<const T>(arr.data(), yato::dims(Size));
    }

    template<typename T>
    YATO_DEPRECATED("Use yato::view/yato::cview")
    auto make_view(std::vector<T> & vec) YATO_NOEXCEPT_KEYWORD 
        -> typename std::enable_if<!std::is_same<T, bool>::value, array_view<T> >::type
    {
        return array_view<T>(vec.data(), yato::dims(vec.size()));
    }

    template<typename T>
    YATO_DEPRECATED("Use yato::view/yato::cview")
    auto make_view(const std::vector<T> & vec) YATO_NOEXCEPT_KEYWORD
        -> typename std::enable_if<!std::is_same<T, bool>::value, array_view<const T> >::type
    {
        return array_view<const T>(vec.data(), yato::dims(vec.size()));
    }

    template<typename T, typename Size1, typename... Sizes>
    array_view_nd<T, sizeof...(Sizes) + 1> make_view(T* ptr, Size1 && size1, Sizes && ...sizes)
    {
        return array_view_nd<T, sizeof...(Sizes) + 1>(ptr, yato::dims(std::forward<Size1>(size1), std::forward<Sizes>(sizes)...));
    }

    template<typename T, size_t Size1, size_t Size2>
    YATO_DEPRECATED("Use yato::view/yato::cview")
    array_view_nd<T, 2> make_view(T(&arr)[Size1][Size2])
    {
        return array_view_nd<T, 2>(&arr[0][0], yato::dims(Size1, Size2));
    }

    template<typename T, size_t Size1, size_t Size2, size_t Size3>
    YATO_DEPRECATED("Use yato::view/yato::cview")
    array_view_nd<T, 3> make_view(T(&arr)[Size1][Size2][Size3])
    {
        return array_view_nd<T, 3>(&arr[0][0][0], yato::dims(Size1, Size2, Size3));
    }


    template<typename Ty_, size_t Dims_ , typename Impl_>
    YATO_DEPRECATED("Use yato::view/yato::cview")
    array_view_nd<std::add_const_t<Ty_>, Dims_> make_view(const const_container_nd<Ty_, Dims_, Impl_> & c) YATO_NOEXCEPT_KEYWORD
    {
        return array_view_nd<std::add_const_t<Ty_>, Dims_>(c.cdata(), c.dimensions(), c.strides());
    }

    template<typename Ty_, size_t Dims_ , typename Impl_>
    YATO_DEPRECATED("Use yato::view/yato::cview")
    array_view_nd<Ty_, Dims_> make_view(const container_nd<Ty_, Dims_, Impl_> & c) YATO_NOEXCEPT_KEYWORD
    {
        return array_view_nd<Ty_, Dims_>(c.data(), c.dimensions(), c.strides());
    }

}

#endif
