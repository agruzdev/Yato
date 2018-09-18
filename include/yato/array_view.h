/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_VIEW_H_
#define _YATO_ARRAY_VIEW_H_

#include <array>
#include <initializer_list>
#include <vector>

#include "array_proxy.h"
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
            using strides_type      = dimensionality<DimsNum - 1, size_t>;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = DimsNum;
            static_assert(dimensions_number > 1, "Dimensions number should be greater than 1");

            using value_iterator  = std::add_pointer_t<value_type>;
            using value_reference = std::add_lvalue_reference_t<value_type>;

            using dim_descriptor = dimension_descriptor_strided<size_type>; // size, stride, offset

            using sub_view       = details::sub_array_proxy<value_iterator, dim_descriptor, dimensions_number - 1>;

            using reference       = sub_view;
            using const_reference = sub_view;
            using iterator        = sub_view;
            using const_iterator  = sub_view;

            using plain_iterator        = std::add_pointer_t<value_type>;
            using const_plain_iterator  = std::add_pointer_t<std::add_const_t<value_type>>;
            //--------------------------------------------------------------------

            value_iterator m_base_ptr;
            std::array<dim_descriptor::type, dimensions_number> m_descriptors;
            //--------------------------------------------------------------------

            array_view_base(value_type* ptr, const dimensions_type & extents, const strides_type & strides)
                : m_base_ptr(ptr)
            {
                YATO_REQUIRES(ptr != nullptr);
                YATO_REQUIRES(extents[dimensions_number - 1] <= strides[dimensions_number - 2]);
                m_descriptors[dimensions_number - 1] = std::make_tuple(extents[dimensions_number - 1], extents[dimensions_number - 1], strides[dimensions_number - 2]);
                for (size_t i = dimensions_number - 1; i > 0; --i) {
                    YATO_REQUIRES((i > 1) ? extents[i - 1] <= strides[i - 2] : true);
                    m_descriptors[i - 1] = std::make_tuple(
                        extents[i - 1],
                        extents[i - 1] * std::get<dim_descriptor::idx_total>(m_descriptors[i]),
                        (i > 1 ? strides[i - 2] : extents[0]) * std::get<dim_descriptor::idx_offset>(m_descriptors[i]));
                }
            }

            ~array_view_base() = default;
            
            array_view_base(const this_type&) = default;
            array_view_base& operator= (const this_type&) = default;

            array_view_base(this_type&&) = default;
            array_view_base& operator= (this_type&&) = default;

            template <typename VTy_ = value_type, typename = std::enable_if_t<std::is_const<VTy_>::value>>
            array_view_base(const array_view_base<std::remove_const_t<VTy_>, dimensions_number> & other)
                : m_base_ptr(other.m_base_ptr), m_descriptors(other.m_descriptors)
            { }

            template <typename VTy_ = value_type, typename = std::enable_if_t<std::is_const<VTy_>::value>>
            array_view_base& operator=(const array_view_base<std::remove_const_t<VTy_>, dimensions_number> & other)
            {
                m_base_ptr    = other.m_base_ptr;
                m_descriptors = other.m_descriptors;
                return *this;
            }

            value_iterator get_pointer_() const
            {
                return m_base_ptr;
            }

            size_type get_size_(size_t idx) const
            {
                YATO_REQUIRES(idx < dimensions_number);
                return std::get<dim_descriptor::idx_size>(m_descriptors[idx]);
            }

            size_type get_stride_(size_t idx) const
            {
                YATO_REQUIRES(idx < dimensions_number - 1);
                return (idx + 2 < dimensions_number)
                    ? std::get<dim_descriptor::idx_offset>(m_descriptors[idx + 1]) / std::get<dim_descriptor::idx_offset>(m_descriptors[idx + 2])
                    : std::get<dim_descriptor::idx_offset>(m_descriptors[dimensions_number - 1]);
            }

            size_type get_total_size_() const
            {
                return std::get<dim_descriptor::idx_total>(m_descriptors[0]);
            }

            size_type get_total_stored_() const
            {
                return std::get<dim_descriptor::idx_offset>(m_descriptors[0]);
            }

            auto get_dims_iter_() const
                -> decltype(yato::make_range(m_descriptors).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>()))
            {
                return yato::make_range(m_descriptors).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>());
            }

            sub_view get_sub_view_(size_t idx) const
            {
                return sub_view(std::next(m_base_ptr, idx * std::get<dim_descriptor::idx_offset>(m_descriptors[1])), &(m_descriptors[1]));
            }

            iterator get_iterator_(size_t idx) const
            {
                return get_sub_view_(idx);
            }

            const_iterator get_const_iterator_(size_t idx) const
            {
                return get_sub_view_(idx);
            }

            template<typename... IdxTail>
            value_reference at_impl_(size_t idx, IdxTail && ... tail) const
            {
                static_assert(sizeof...(IdxTail)+1 == dimensions_number, "Wrong indexes number");
                if (idx >= get_size_(0)) {
                    throw yato::out_of_range_error("yato::array_view_nd[at]: out of range!");
                }
                return get_sub_view_(idx).at(std::forward<IdxTail>(tail)...);
            }

            bool is_continuous_() const
            {
                return get_total_size_() == get_total_stored_();
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
            using strides_type      = dimensionality<0, size_t>;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;

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

            array_view_base(value_type* ptr, const dimensions_type & extents, const strides_type &)
                : m_base_ptr(ptr), m_size(extents)
            {
                YATO_REQUIRES(ptr != nullptr);
            }

            ~array_view_base() = default;

            array_view_base(const this_type&) = default;
            array_view_base& operator= (const this_type&) = default;

            array_view_base(this_type&&) = default;
            array_view_base& operator= (this_type&&) = default;

            template <typename VTy_ = value_type, typename = std::enable_if_t<std::is_const<VTy_>::value>>
            array_view_base(const array_view_base<std::remove_const_t<VTy_>, dimensions_number> & other)
                : m_base_ptr(other.m_base_ptr), m_size(other.m_size)
            { }

            template <typename VTy_ = value_type, typename = std::enable_if_t<std::is_const<VTy_>::value>>
            array_view_base& operator=(const array_view_base<std::remove_const_t<VTy_>, dimensions_number> & other)
            {
                m_base_ptr = other.m_base_ptr;
                m_size     = other.m_size;
                return *this;
            }

            value_iterator get_pointer_() const
            {
                return m_base_ptr;
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
                YATO_REQUIRES(idx == 0);
                return m_size[0];
            }

            size_type get_total_size_() const
            {
                return m_size[0];
            }

            size_type get_total_stored_() const
            {
                return m_size[0];
            }

            auto get_dims_iter_() const
                -> typename yato::range<dimensions_type::const_iterator>
            {
                return yato::make_range(m_size.cbegin(), m_size.cend());
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
    template<typename ValueType, size_t DimsNum>
    class array_view_nd
        : private details::array_view_base<ValueType, DimsNum>
    {
    public:
        using this_type = array_view_nd<ValueType, DimsNum>;
        using base_type = details::array_view_base<ValueType, DimsNum>;

        using base_type::dimensions_number;
        using typename base_type::dimensions_type;
        using typename base_type::strides_type;
        using typename base_type::value_type;
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

        array_view_nd(value_type* ptr, const dimensions_type & extents)
            : base_type(ptr, extents, extents.sub_dims())
        { }

        array_view_nd(value_type* ptr, const dimensions_type & extents, const strides_type & strides)
            : base_type(ptr, extents, strides)
        { }

        array_view_nd(const this_type & other) = default;
        array_view_nd& operator=(const this_type &) = default;

        array_view_nd(this_type &&) = default;
        array_view_nd& operator=(this_type && other) = default;

        ~array_view_nd() = default;

        /**
         * Convert view<T> to view<const T>
         */
        template <typename VTy_ = value_type, typename = std::enable_if_t<std::is_const<VTy_>::value>>
        array_view_nd(const array_view_nd<std::remove_const_t<VTy_>, dimensions_number> & other)
            : base_type(other)
        { }

        /**
         * Assign view<T> to view<const T>
         */
        template <typename VTy_ = value_type, typename = std::enable_if_t<std::is_const<VTy_>::value>>
        array_view_nd& operator=(const array_view_nd<std::remove_const_t<VTy_>, dimensions_number> & other)
        {
            base_type::operator=(other);
            return *this;
        }

        /**
         * Create a new array view on the same data but with another shape
         * Total size should be unchanged
         */
        template <size_t NewDimsNum>
        auto reshape(const dimensionality<NewDimsNum, size_type> & extents) const
            -> array_view_nd<value_type, NewDimsNum>
        {
            YATO_REQUIRES(extents.total_size() == base_type::get_total_stored_());
            return array_view_nd<value_type, NewDimsNum>(base_type::get_pointer_(), extents);
        }

        /**
         * Create a new array view on the same data but with another shape
         * Total size should be unchanged
         */
        template <size_t NewDimsNum>
        auto reshape(const dimensionality<NewDimsNum, size_type> & extents, const dimensionality<NewDimsNum - 1, size_type> & strides) const
            -> array_view_nd<value_type, NewDimsNum>
        {
            YATO_REQUIRES(extents[0] * strides.total_size() == base_type::get_total_stored_());
            return array_view_nd<value_type, NewDimsNum>(base_type::get_pointer_(), extents, strides);
        }

        reference operator[](size_t idx) const
        {
            YATO_REQUIRES(idx < base_type::get_size_(0));
            return base_type::get_sub_view_(idx);
        }

        template<typename... Indexes>
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
         * Get stride along specified dimension
         * For 1D array view is equal to size()
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
            return dimensions_type(base_type::get_dims_iter_());
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
            -> decltype(std::declval<base_type>().get_dims_iter_())
        {
            return base_type::get_dims_iter_();
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
            return base_type::get_pointer_() + base_type::get_total_stored_();
        }

        plain_iterator plain_end() const
        {
            YATO_REQUIRES(continuous());
            return base_type::get_pointer_() + base_type::get_total_stored_();
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
         * Get raw pointer to underlying data
         */
        value_type* data() const
        {
            return base_type::get_pointer_();
        }

        // For view<T> to view<const T> conversion
        friend class array_view_nd<std::add_const_t<value_type>, dimensions_number>;
    };


    template<typename _DataType>
    using array_view = array_view_nd<_DataType, 1>;

    template<typename _DataType>
    using array_view_1d = array_view_nd<_DataType, 1>;

    template<typename _DataType>
    using array_view_2d = array_view_nd<_DataType, 2>;

    template<typename _DataType>
    using array_view_3d = array_view_nd<_DataType, 3>;


    template<typename T, size_t Size>
    inline
    array_view<T> make_view(T (& arr)[Size]) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<T>(arr, yato::dims(Size));
    }

    template<typename T, size_t Size>
    inline
    array_view<T> make_view(std::array<T, Size> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<T>(arr.data(), yato::dims(Size));
    }

    template<typename T, size_t Size>
    inline
    array_view<const T> make_view(const std::array<T, Size> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view<const T>(arr.data(), yato::dims(Size));
    }

    template<typename T>
    inline
    auto make_view(std::vector<T> & vec) YATO_NOEXCEPT_KEYWORD 
        -> typename std::enable_if<!std::is_same<T, bool>::value, array_view<T> >::type
    {
        return array_view<T>(vec.data(), yato::dims(vec.size()));
    }

    template<typename T>
    inline
    auto make_view(const std::vector<T> & vec) YATO_NOEXCEPT_KEYWORD
        -> typename std::enable_if<!std::is_same<T, bool>::value, array_view<const T> >::type
    {
        return array_view<const T>(vec.data(), yato::dims(vec.size()));
    }

    template<typename T, typename Size1, typename... Sizes>
    inline
    array_view_nd<T, sizeof...(Sizes) + 1> make_view(T* ptr, Size1 && size1, Sizes && ...sizes)
    {
        return array_view_nd<T, sizeof...(Sizes) + 1>(ptr, yato::dims(std::forward<Size1>(size1), std::forward<Sizes>(sizes)...));
    }

    template<typename T, size_t Size1, size_t Size2>
    inline
    array_view_nd<T, 2> make_view(T(&arr)[Size1][Size2])
    {
        return array_view_nd<T, 2>(&arr[0][0], yato::dims(Size1, Size2));
    }

    template<typename T, size_t Size1, size_t Size2, size_t Size3>
    inline
    array_view_nd<T, 3> make_view(T(&arr)[Size1][Size2][Size3])
    {
        return array_view_nd<T, 3>(&arr[0][0][0], yato::dims(Size1, Size2, Size3));
    }

}

#endif
