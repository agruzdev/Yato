/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_VIEW_H_
#define _YATO_ARRAY_VIEW_H_

#include <array>
#include <vector>
#include <initializer_list>

#include "array_proxy.h"
#include "tuple.h"

namespace yato
{
#ifdef YATO_MSVC
/*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
 *	MSVC complains about type cast otherwise
 */
#pragma warning(push)
#pragma warning(disable:4702) 
#endif


    /**
     *  Non-owning light-weight container for contiguous data 
     */
    template<typename ValueType, size_t DimsNum>
    class array_view_nd
    {
    public:
        using this_type = array_view_nd<ValueType, DimsNum>;
        using dimensions_type = dimensionality<DimsNum,     size_t>;
        using strides_type    = dimensionality<DimsNum - 1, size_t>;
        using value_type = ValueType;
        using size_type = size_t;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = DimsNum;
        static_assert(dimensions_number > 1, "Dimensions number should be greater than 1");

        using value_iterator       = value_type*;
        using const_value_iterator = const value_type*;

    private:
        using dim_descriptor = dimension_descriptor_strided<size_type>; // size, stride, offset
        using const_sizes_iterator = typename dimensions_type::const_iterator;
        using sub_view       = details::sub_array_proxy<value_iterator,       dim_descriptor, dimensions_number - 1>;
        using const_sub_view = details::sub_array_proxy<const_value_iterator, dim_descriptor, dimensions_number - 1>;

    public:
        using iterator       = sub_view;
        using const_iterator = const_sub_view;

        //-------------------------------------------------------
    private:
        std::array<dim_descriptor::type, dimensions_number> m_descriptors;
        value_iterator m_base_ptr;

        void set_extents_(const dimensions_type & extents, const strides_type & strides)
        {
            //m_descriptors[dimensions_number - 1] = std::make_tuple(extents[dimensions_number - 1], extents[dimensions_number - 1], static_cast<size_type>(1));
            //for (size_t i = dimensions_number - 1; i > 0; --i) {
            //    m_descriptors[i - 1] = std::make_tuple(extents[i - 1], extents[i - 1] * std::get<dim_descriptor::idx_total>(m_descriptors[i]), strides[i - 1] * std::get<dim_descriptor::idx_offset>(m_descriptors[i]));
            //}
            m_descriptors[dimensions_number - 1] = std::make_tuple(extents[dimensions_number - 1], extents[dimensions_number - 1], strides[dimensions_number - 2]);
            for (size_t i = dimensions_number - 1; i > 0; --i) {
                m_descriptors[i - 1] = std::make_tuple( extents[i - 1], 
                                                        extents[i - 1] * std::get<dim_descriptor::idx_total>(m_descriptors[i]), 
                                                        (i > 1 ? strides[i - 2] : extents[0]) * std::get<dim_descriptor::idx_offset>(m_descriptors[i]));
            }
        }

        size_type get_stride_(size_t idx) const YATO_NOEXCEPT_KEYWORD
        {
            return (idx + 2 < dimensions_number)
                ? std::get<dim_descriptor::idx_offset>(m_descriptors[idx + 1]) / std::get<dim_descriptor::idx_offset>(m_descriptors[idx + 2])
                : std::get<dim_descriptor::idx_offset>(m_descriptors[dimensions_number - 1]);
        }

        sub_view create_sub_view_(size_t offset)
        {
            return sub_view(std::next(m_base_ptr, offset * std::get<dim_descriptor::idx_offset>(m_descriptors[1])), &(m_descriptors[1]));
        }

        const_sub_view create_const_sub_view_(size_t offset) const
        {
            return const_sub_view(std::next(m_base_ptr, offset * std::get<dim_descriptor::idx_offset>(m_descriptors[1])), &(m_descriptors[1]));
        }
        //-------------------------------------------------------

    public:
        array_view_nd(value_type* ptr, const dimensions_type & extents)
            : m_base_ptr(ptr)
        { 
            YATO_REQUIRES(ptr != nullptr);
            set_extents_(extents, extents.sub_dims());
        }

        array_view_nd(value_type* ptr, const dimensions_type & extents, const strides_type & strides)
            : m_base_ptr(ptr)
        {
            YATO_REQUIRES(ptr != nullptr);
            set_extents_(extents, strides);
        }

        array_view_nd(const this_type & other) = default;

        array_view_nd& operator=(const this_type & other)
        {
            YATO_REQUIRES(this != &other);
            m_descriptors = other.m_descriptors;
            m_base_ptr = other.m_base_ptr;
            return *this;
        }

#ifndef YATO_MSVC_2013
        array_view_nd(this_type &&) = default;
#else
        array_view_nd(this_type && other)
            : m_descriptors(std::move(other.m_descriptors)), m_base_ptr(std::move(other.m_base_ptr))
        { }
#endif

        array_view_nd& operator=(this_type && other)
        {
            YATO_REQUIRES(this != &other);
            m_descriptors = std::move(other.m_descriptors);
            m_base_ptr = std::move(other.m_base_ptr);
            return *this;
        }

        ~array_view_nd() = default;

        /**
         * Create a new array view on the same data but with another shape
         * Total size should be unchanged
         */
        template <size_t NewDimsNum>
        auto reshape(const dimensionality<NewDimsNum, size_type> & extents) const
            -> array_view_nd<value_type, NewDimsNum>
        {
            YATO_REQUIRES(extents.total_size() == total_reserved());
            return array_view_nd<value_type, NewDimsNum>(m_base_ptr, extents);
        }

        /**
         * Create a new array view on the same data but with another shape
         * Total size should be unchanged
         */
        template <size_t NewDimsNum>
        auto reshape(const dimensionality<NewDimsNum, size_type> & extents, const dimensionality<NewDimsNum - 1, size_type> & strides) const
            -> array_view_nd<value_type, NewDimsNum>
        {
            YATO_REQUIRES(extents[0] * strides.total_size() == total_reserved());
            return array_view_nd<value_type, NewDimsNum>(m_base_ptr, extents, strides);
        }

        const_sub_view operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return (idx < size(0))
                ? create_const_sub_view_(idx)
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd: out of range!"), create_const_sub_view_(0));
#else
            return create_const_sub_view_(idx);
#endif
        }

        sub_view operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return (idx < size(0))
                ? create_sub_view_(idx)
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd: out of range!"), create_sub_view_(idx));
#else
            return create_sub_view_(idx);
#endif
        }

        template<typename... _IdxTail>
        const value_type & at(size_t idx, _IdxTail... tail) const
        {
            if (idx >= size(0)) {
                throw yato::out_of_range_error("yato::array_view_nd: out of range!");
            }
            return (*this)[idx].at(tail...);
        }

        template<typename... _IdxTail>
        value_type & at(size_t idx, _IdxTail... tail)
        {
            if (idx >= size(0)) {
                throw yato::out_of_range_error("yato::array_view_nd: out of range!");
            }
            return (*this)[idx].at(tail...);
        }

        size_type total_size() const YATO_NOEXCEPT_KEYWORD
        {
            return std::get<dim_descriptor::idx_total>(m_descriptors[0]);
        }

        size_type size(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return idx < dimensions_number
                ? std::get<dim_descriptor::idx_size>(m_descriptors[idx])
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd[size]: idx out of range!"), 0);
#else
            return std::get<dim_descriptor::idx_size>(m_descriptors[idx]);
#endif
        }

        size_type stride(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return idx < (dimensions_number - 1)
                ? get_stride_(idx)
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view_nd[stride]: idx out of range!"), 0);
#else
            return get_stride_(idx);
#endif
        }

        /**
         * Get total number of elements in the view with strides
         */
        size_type total_reserved() const
        {
            return std::get<dim_descriptor::idx_offset>(m_descriptors[0]);
        }

        /**
         * Get dimensions
         */
        dimensions_type dimensions() const
        {
            return dimensions_type(dimensions_range());
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
            -> decltype(yato::make_range(m_descriptors).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>()))
        {
            return yato::make_range(m_descriptors).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>());
        }

        const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return create_const_sub_view_(0);
        }

        iterator begin() YATO_NOEXCEPT_KEYWORD
        {
            return create_sub_view_(0);
        }

        const_iterator cend() const YATO_NOEXCEPT_KEYWORD
        {
            return create_const_sub_view_(size(0));
        }

        iterator end() YATO_NOEXCEPT_KEYWORD
        {
            return create_sub_view_(size(0));
        }

        const_value_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        value_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        const_value_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + total_reserved();
        }

        value_iterator plain_end() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + total_reserved();
        }

        yato::range<const_iterator> crange() const YATO_NOEXCEPT_KEYWORD
        {
            return make_range(cbegin(), cend());
        }

        yato::range<iterator> range() YATO_NOEXCEPT_KEYWORD
        {
            return make_range(begin(), end());
        }

        yato::range<const_value_iterator> plain_crange() const YATO_NOEXCEPT_KEYWORD
        {
            return make_range(plain_cbegin(), plain_cend());
        }

        yato::range<value_iterator> plain_range() YATO_NOEXCEPT_KEYWORD
        {
            return make_range(plain_begin(), plain_end());
        }

        /**
         * Get raw pointer to underlying data
         */
        value_type* data()
        {
            return m_base_ptr;
        }

        /**
         * Get raw pointer to underlying data
         */
        const value_type* data() const
        {
            return const_cast<this_type*>(this)->data();
        }
    };

    /**
     *	More effective specialization of 1D array view
     */
    template<typename _DataType>
    class array_view_nd<_DataType, 1>
    {
    public:
        using dimensions_type = dimensionality<1, size_t>;
        using data_type = _DataType;
        using data_iterator = data_type*;
        using const_data_iterator = const data_type*;
        using iterator = data_iterator;
        using const_iterator = const_data_iterator;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;

    private:
        data_type * m_base_ptr;
        const dimensions_type m_size;

    public:
        array_view_nd(data_type* ptr, const dimensions_type & sizes) YATO_NOEXCEPT_IN_RELEASE
            : m_base_ptr(ptr), m_size(sizes)
        {
            YATO_REQUIRES(ptr != nullptr);
        }

        array_view_nd(const array_view_nd & other) YATO_NOEXCEPT_KEYWORD
            : m_base_ptr(other.m_base_ptr), m_size(other.m_size)
        { }

        array_view_nd(array_view_nd && other) YATO_NOEXCEPT_KEYWORD
            : m_base_ptr(other.m_base_ptr), m_size(other.m_size)
        { }

        array_view_nd& operator=(const array_view_nd & other) YATO_NOEXCEPT_KEYWORD
        {
            if (this != &other) {
                m_base_ptr = std::move(other.m_base_ptr);
                m_size = std::move(other.m_size);
            }
            return *this;
        }
        
        array_view_nd& operator=(array_view_nd && other) YATO_NOEXCEPT_KEYWORD
        {
            if (this != &other) {
                m_base_ptr = other.m_base_ptr;
                m_size = other.m_size;
            }
            return *this;
        }

        ~array_view_nd() = default;

        /**
         * Create a new array view on the same data but with another shape
         * Total size should be unchanged
         */
        template <size_t NewDimsNum>
        auto reshape(const dimensionality<NewDimsNum, size_t> & extents) const
            -> array_view_nd<data_type, NewDimsNum>
        {
            YATO_REQUIRES(extents.total_size() == total_size());
            return array_view_nd<data_type, NewDimsNum>(m_base_ptr, extents);
        }

        /**
        * Create a new array view on the same data but with another shape
        * Total size should be unchanged
        */
        template <size_t NewDimsNum>
        auto reshape(const dimensionality<NewDimsNum, size_t> & extents, const dimensionality<NewDimsNum - 1, size_t> & strides) const
            -> array_view_nd<data_type, NewDimsNum>
        {
            YATO_REQUIRES(extents[0] * strides.total_size() == total_reserved());
            return array_view_nd<data_type, NewDimsNum>(m_base_ptr, extents, strides);
        }
        
        const data_type & operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return at(idx);
#else
            return m_base_ptr[idx];
#endif
        }

        data_type & operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return at(idx);
#else
            return m_base_ptr[idx];
#endif
        }

        const data_type & at(size_t idx) const
        {
            return idx < m_size[0]
                ? m_base_ptr[idx]
                : (throw yato::out_of_range_error("yato::array_view: index out of range!"), m_base_ptr[idx]);
        }

        data_type & at(size_t idx)
        {
            return idx < m_size[0]
                ? m_base_ptr[idx]
                : (throw yato::out_of_range_error("yato::array_view: index out of range!"), m_base_ptr[idx]);
        }

        const data_type * data() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        data_type * data() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        size_t total_size() const YATO_NOEXCEPT_KEYWORD
        {
            return m_size[0];
        }

        size_t total_reserved() const
        {
            return total_size();
        }

        size_t size(size_t idx = 0) const YATO_NOEXCEPT_IN_RELEASE
        {
#if YATO_DEBUG
            return idx < dimensions_number
                ? m_size[0]
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_view[size]: idx is out of range!"), m_size[0]);
#else
            (void)idx;
            return m_size[0];
#endif
        }

        /**
        * Get dimensions
        */
        const dimensions_type & dimensions() const
        {
            return m_size;
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
            -> yato::range<typename dimensions_type::const_iterator>
        {
            return yato::make_range(m_size.cbegin(), m_size.cend());
        }

        const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        iterator begin() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        const_iterator cend() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size[0];
        }

        iterator end() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size[0];
        }

        const_data_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        data_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr;
        }

        const_data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size[0];
        }

        data_iterator plain_end() YATO_NOEXCEPT_KEYWORD
        {
            return m_base_ptr + m_size[0];
        }

        yato::range<const_iterator> crange() const
        {
            return make_range(cbegin(), cend());
        }

        yato::range<iterator> range()
        {
            return make_range(begin(), end());
        }

        yato::range<const_data_iterator> plain_crange() const
        {
            return make_range(plain_cbegin(), plain_cend());
        }

        yato::range<data_iterator> plain_range()
        {
            return make_range(plain_begin(), plain_end());
        }

    };
#ifdef YATO_MSVC
#pragma warning(pop)
#endif
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
    array_view_nd<T, sizeof...(Sizes) + 1> make_view(T* ptr, Size1 && size1, Sizes && ...sizes) YATO_NOEXCEPT_IN_RELEASE
    {
        return array_view_nd<T, sizeof...(Sizes) + 1>(ptr, yato::dims(std::forward<Size1>(size1), std::forward<Sizes>(sizes)...));
    }

    template<typename T, size_t Size1, size_t Size2>
    inline
    array_view_nd<T, 2> make_view(T(&arr)[Size1][Size2]) YATO_NOEXCEPT_IN_RELEASE
    {
        return array_view_nd<T, 2>(&arr[0][0], yato::dims(Size1, Size2));
    }

    template<typename T, size_t Size1, size_t Size2, size_t Size3>
    inline
    array_view_nd<T, 3> make_view(T(&arr)[Size1][Size2][Size3]) YATO_NOEXCEPT_IN_RELEASE
    {
        return array_view_nd<T, 3>(&arr[0][0][0], yato::dims(Size1, Size2, Size3));
    }

}

#endif
