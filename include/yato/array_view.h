/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
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

            using value_type = ValueType;
            using dimensions_type = dimensionality<DimsNum, size_t>;
            using strides_type    = dimensionality<DimsNum - 1, size_t>;
            using size_type = size_t;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = DimsNum;
            static_assert(dimensions_number > 1, "Dimensions number should be greater than 1");

            using value_iterator        = value_type*;
            using const_value_iterator  = const value_type*;
            using value_reference       = value_type&;
            using const_value_reference = const value_type&;

            using dim_descriptor = dimension_descriptor_strided<size_type>; // size, stride, offset

            using sub_view       = details::sub_array_proxy<value_iterator, dim_descriptor, dimensions_number - 1>;
            using const_sub_view = details::sub_array_proxy<const_value_iterator, dim_descriptor, dimensions_number - 1>;

            using reference       = sub_view;
            using const_reference = const_sub_view;
            using iterator        = sub_view;
            using const_iterator  = const_sub_view;

            //--------------------------------------------------------------------

            value_iterator m_base_ptr;
            std::array<dim_descriptor::type, dimensions_number> m_descriptors;
            //--------------------------------------------------------------------

            array_view_base(value_type* ptr, const dimensions_type & extents, const strides_type & strides)
                : m_base_ptr(ptr)
            {
                YATO_REQUIRES(ptr != nullptr);
                m_descriptors[dimensions_number - 1] = std::make_tuple(extents[dimensions_number - 1], extents[dimensions_number - 1], strides[dimensions_number - 2]);
                for (size_t i = dimensions_number - 1; i > 0; --i) {
                    m_descriptors[i - 1] = std::make_tuple(extents[i - 1],
                        extents[i - 1] * std::get<dim_descriptor::idx_total>(m_descriptors[i]),
                        (i > 1 ? strides[i - 2] : extents[0]) * std::get<dim_descriptor::idx_offset>(m_descriptors[i]));
                }
            }

            ~array_view_base() = default;
            
            array_view_base(const this_type&) = default;
            array_view_base& operator= (const this_type&) = default;

#ifndef YATO_MSVC_2013
            array_view_base(this_type&&) = default;
            array_view_base& operator= (this_type&&) = default;
#else
            array_view_base(this_type && other)
                : m_descriptors(std::move(other.m_descriptors)), m_base_ptr(std::move(other.m_base_ptr))
            { }

            array_view_base& operator= (this_type && other)
            {
                REQUIRES(this != &other);
                m_descriptors = std::move(other.m_descriptors);
                m_base_ptr = std::move(other.m_base_ptr);
            }
#endif

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

            sub_view get_sub_view_(size_t idx)
            {
                return sub_view(std::next(m_base_ptr, idx * std::get<dim_descriptor::idx_offset>(m_descriptors[1])), &(m_descriptors[1]));
            }

            const_sub_view get_const_sub_view_(size_t idx) const
            {
                return const_sub_view(std::next(m_base_ptr, idx * std::get<dim_descriptor::idx_offset>(m_descriptors[1])), &(m_descriptors[1]));
            }

            iterator get_iterator_(size_t idx)
            {
                return get_sub_view_(idx);
            }

            const_iterator get_const_iterator_(size_t idx) const
            {
                return get_const_sub_view_(idx);
            }

            template<typename... IdxTail>
            value_reference at_impl_(size_t idx, IdxTail && ... tail)
            {
                static_assert(sizeof...(IdxTail)+1 == dimensions_number, "Wrong indexes number");
                if (idx >= get_size_(0)) {
                    throw yato::out_of_range_error("yato::array_view_nd[at]: out of range!");
                }
                return get_sub_view_(idx).at(std::forward<IdxTail>(tail)...);
            }

            template<typename... IdxTail>
            const_value_reference const_at_impl_(size_t idx, IdxTail && ... tail) const
            {
                static_assert(sizeof...(IdxTail)+1 == dimensions_number, "Wrong indexes number");
                if (idx >= get_size_(0)) {
                    throw yato::out_of_range_error("yato::array_view_nd[at]: out of range!");
                }
                return get_const_sub_view_(idx).at(std::forward<IdxTail>(tail)...);
            }
        };

        // Single dimension case
        template<typename ValueType>
        class array_view_base<ValueType, 1>
        {
            using this_type = array_view_base<ValueType, 1>;

        public:
            using dimensions_type = dimensionality<1, size_t>;
            using strides_type    = dimensionality<0, size_t>;
            using value_type = ValueType;
            using size_type = size_t;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;

            using value_iterator        = value_type*;
            using const_value_iterator  = const value_type*;
            using value_reference       = value_type&;
            using const_value_reference = const value_type&;

            using iterator        = value_iterator;
            using const_iterator  = const_value_iterator;
            using reference       = value_reference&;
            using const_reference = const_value_reference;
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

#ifndef YATO_MSVC_2013
            array_view_base(this_type&&) = default;
            array_view_base& operator= (this_type&&) = default;
#else
            array_view_base(this_type && other)
                : m_base_ptr(std::move(other.m_base_ptr)), m_size(std::move(other.m_size))
            { }

            array_view_base& operator= (this_type && other)
            {
                REQUIRES(this != &other);
                m_base_ptr = std::move(other.m_base_ptr);
                m_size = std::move(other.m_size);
            }
#endif
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

            value_reference get_sub_view_(size_t idx)
            {
                return *std::next(m_base_ptr, idx);
            }

            const_value_reference get_const_sub_view_(size_t idx) const
            {
                return *std::next(m_base_ptr, idx);
            }

            iterator get_iterator_(size_t idx)
            {
                return std::next(m_base_ptr, idx);
            }

            const_iterator get_const_iterator_(size_t idx) const
            {
                return std::next(m_base_ptr, idx);
            }

            value_reference at_impl_(size_t idx)
            {
                if (idx >= m_size[0]) {
                    throw yato::out_of_range_error("yato::array_view_nd[at]: out of range!");
                }
                return get_sub_view_(idx);
            }

            const_value_reference const_at_impl_(size_t idx) const
            {
                if (idx >= m_size[0]) {
                    throw yato::out_of_range_error("yato::array_view_nd[at]: out of range!");
                }
                return get_const_sub_view_(idx);
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
        using typename base_type::const_value_iterator;
        using typename base_type::value_reference;
        using typename base_type::const_value_reference;
        using typename base_type::reference;
        using typename base_type::const_reference;
        using typename base_type::iterator;
        using typename base_type::const_iterator;
        //-------------------------------------------------------

        array_view_nd(value_type* ptr, const dimensions_type & extents)
            : base_type(ptr, extents, extents.sub_dims())
        { }

        array_view_nd(value_type* ptr, const dimensions_type & extents, const strides_type & strides)
            : base_type(ptr, extents, strides)
        { }

        array_view_nd(const this_type & other) = default;
        array_view_nd& operator=(const this_type &) = default;

#ifndef YATO_MSVC_2013
        array_view_nd(this_type &&) = default;
        array_view_nd& operator=(this_type && other) = default;
#else
        array_view_nd(this_type && other)
            : base_type(std::move(other))
        { }

        array_view_nd& operator=(this_type && other)
        {
            move_impl(std::move(other));
            return *this;
        }
#endif

        ~array_view_nd() = default;

        /**
         * Create a new array view on the same data but with another shape
         * Total size should be unchanged
         */
        template <size_t NewDimsNum>
        auto reshape(const dimensionality<NewDimsNum, size_type> & extents) const
            -> array_view_nd<value_type, NewDimsNum>
        {
            YATO_REQUIRES(extents.total_size() == total_stored());
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
            YATO_REQUIRES(extents[0] * strides.total_size() == total_stored());
            return array_view_nd<value_type, NewDimsNum>(base_type::get_pointer_(), extents, strides);
        }

        const_reference operator[](size_t idx) const
        {
            YATO_REQUIRES(idx < base_type::get_size_(0));
            return base_type::get_const_sub_view_(idx);
        }

        reference operator[](size_t idx)
        {
            YATO_REQUIRES(idx < base_type::get_size_(0));
            return base_type::get_sub_view_(idx);
        }

#ifndef YATO_MSVC_2013
        template<typename... Indexes>
        auto at(Indexes &&... indexes) const
            -> typename std::enable_if<(sizeof...(Indexes) == dimensions_number), const_value_reference>::type
        {
            return base_type::const_at_impl_(std::forward<Indexes>(indexes)...);
        }

        template<typename... Indexes>
        auto at(Indexes &&... indexes)
            -> typename std::enable_if<(sizeof...(Indexes) == dimensions_number), value_reference>::type
        {
            return base_type::at_impl_(std::forward<Indexes>(indexes)...);
        }
#else
        template<typename... Indexes>
        const_value_reference at(Indexes &&... indexes) const
        {
            static_assert((sizeof...(Indexes) == dimensions_number), "Invalid arguments number");
            return base_type::const_at_impl_(std::forward<Indexes>(indexes)...);
        }

        template<typename... Indexes>
        value_reference at(Indexes &&... indexes)
        {
            static_assert((sizeof...(Indexes) == dimensions_number), "Invalid arguments number");
            return base_type::at_impl_(std::forward<Indexes>(indexes)...);
        }
#endif
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
         * Get total number of elements in the view with strides
         */
        size_type total_stored() const
        {
            return base_type::get_total_stored_();
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

        iterator begin()
        {
            return base_type::get_iterator_(0);
        }

        const_iterator cend() const
        {
            return base_type::get_const_iterator_(size(0));
        }

        iterator end()
        {
            return base_type::get_iterator_(size(0));
        }

        const_value_iterator plain_cbegin() const
        {
            return base_type::get_pointer_();
        }

        value_iterator plain_begin()
        {
            return base_type::get_pointer_();
        }

        const_value_iterator plain_cend() const
        {
            return base_type::get_pointer_() + base_type::get_total_stored_();
        }

        value_iterator plain_end()
        {
            return base_type::get_pointer_() + base_type::get_total_stored_();
        }

        yato::range<const_iterator> crange() const
        {
            return make_range(cbegin(), cend());
        }

        yato::range<iterator> range()
        {
            return make_range(begin(), end());
        }

        yato::range<const_value_iterator> plain_crange() const
        {
            return make_range(plain_cbegin(), plain_cend());
        }

        yato::range<value_iterator> plain_range()
        {
            return make_range(plain_begin(), plain_end());
        }

        /**
         * Get raw pointer to underlying data
         */
        value_type* data()
        {
            return base_type::get_pointer_();
        }

        /**
         * Get raw pointer to underlying data
         */
        const value_type* data() const
        {
            return base_type::get_pointer_();
        }
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
