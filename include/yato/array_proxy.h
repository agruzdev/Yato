/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_RPOXY_H_
#define _YATO_ARRAY_RPOXY_H_

#include <iterator>
#include "assert.h"
#include "not_null.h"
#include "types.h"
#include "range.h"
#include "container_base.h"

namespace yato
{

#ifdef YATO_MSVC
    /*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
    *	MSVC complains about type cast otherwise
    */
#pragma warning(push)
#pragma warning(disable:4702) 
#endif
    namespace details
    {
        template<typename DataIterator, typename DimensionDescriptor, size_t DimsNum>
        class sub_array_proxy;

        //-------------------------------------------------------
        // Proxy class for accessing multidimensional containers
        // Has trait of iterator
        // 

        template<typename DataIterator, typename DimensionDescriptor, size_t DimsNum>
        class sub_array_proxy
        {
            static_assert(DimsNum >= 1, "dimensions_number cant be 0");
        public:
            using this_type = sub_array_proxy <DataIterator, DimensionDescriptor, DimsNum>;
            using dim_descriptor = DimensionDescriptor;
            using desc_iterator  = const typename DimensionDescriptor::type*;
            using data_iterator       = DataIterator;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = DimsNum;

            using sub_proxy = sub_array_proxy<data_iterator, dim_descriptor, dimensions_number - 1>;

            using iter_value_type     = typename std::iterator_traits<DataIterator>::value_type;
            using iter_pointer_type   = typename std::iterator_traits<DataIterator>::pointer;
            using iter_reference_type = typename std::iterator_traits<DataIterator>::reference;

            using size_type         = typename DimensionDescriptor::size_type;
            using value_type        = typename std::conditional<(dimensions_number > 1), sub_proxy, iter_value_type>::type;
            using pointer           = typename std::conditional<(dimensions_number > 1), sub_proxy, iter_pointer_type>::type;
            using reference         = typename std::conditional<(dimensions_number > 1), sub_proxy, iter_reference_type>::type;
            using difference_type   = std::ptrdiff_t;
            using iterator_category = std::random_access_iterator_tag;

            using iterator       = sub_proxy;

            using dimensions_type = dimensionality<DimsNum, size_type>;
            //-------------------------------------------------------

        private:
            data_iterator m_data_iter;
            desc_iterator m_desc_iter;
            //-------------------------------------------------------

            size_type get_stride_(size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                return (idx + 2 < dimensions_number)
                    ? std::get<dim_descriptor::idx_offset>(*std::next(m_desc_iter)) / std::get<dim_descriptor::idx_offset>(*std::next(m_desc_iter, 2))
                    : std::get<dim_descriptor::idx_offset>(*std::next(m_desc_iter));
            }

            YATO_CONSTEXPR_FUNC
            sub_proxy create_sub_proxy_(size_t offset) const YATO_NOEXCEPT_KEYWORD
            {
                return sub_proxy(std::next(m_data_iter, offset * std::get<dim_descriptor::idx_offset>(*std::next(m_desc_iter))), std::next(m_desc_iter));
            }
            //-------------------------------------------------------

        public:
            YATO_CONSTEXPR_FUNC
            sub_array_proxy(const data_iterator & data, desc_iterator descriptors) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(data), m_desc_iter(descriptors)
            { }

            sub_array_proxy(const this_type &) = default;

            template <typename AnotherDataIterator>
            sub_array_proxy(const sub_array_proxy<AnotherDataIterator, dim_descriptor, dimensions_number> & other)
                : m_data_iter(other.m_data_iter), m_desc_iter(other.m_desc_iter)
            { }

#ifndef YATO_MSVC_2013
            sub_array_proxy(this_type &&) = default;
#else
            sub_array_proxy(sub_array_proxy && other) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(std::move(other.m_data_iter)), m_desc_iter(std::move(other.m_desc_iter))
            {}
#endif

            this_type & operator= (const this_type & other)
            {
                YATO_REQUIRES(this != &other);
                m_data_iter = other.m_data_iter;
                m_desc_iter = other.m_desc_iter;
                return *this;
            }

            this_type & operator= (this_type && other) YATO_NOEXCEPT_IN_RELEASE
            {
                YATO_REQUIRES(this != &other);
                m_data_iter = std::move(other.m_data_iter);
                m_desc_iter = std::move(other.m_desc_iter);
                return *this;
            }

            ~sub_array_proxy()= default;

            template<size_t MyDimsNum = dimensions_number>
            YATO_CONSTEXPR_FUNC
            auto operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if<(MyDimsNum > 1), reference>::type
            {
#if YATO_DEBUG
                return (idx < size(0))
                    ? create_sub_proxy_(idx)
                    : (YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!"), create_sub_proxy_(0));
#else
                return create_sub_proxy_(idx);
#endif
            }

            template<size_t MyDimsNum = dimensions_number>
            YATO_CONSTEXPR_FUNC
            auto operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if <(MyDimsNum == 1), reference>::type
            {
#if YATO_DEBUG
                return (idx < size(0))
                    ? *std::next(m_data_iter, idx)
                    : (YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!"), *(m_data_iter));
#else
                return *std::next(m_data_iter, idx);
#endif
            }

            template<size_t MyDimsNum = dimensions_number, typename... _IdxTail>
            auto at(size_t idx, _IdxTail... tail) const
                -> typename std::enable_if <(MyDimsNum > 1), iter_reference_type>::type
            {
                if (idx >= size(0)) {
                    throw yato::out_of_range_error("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx].at(tail...);
            }

            template<size_t MyDimsNum = dimensions_number>
            auto at(size_t idx) const
                -> typename std::enable_if <(MyDimsNum == 1), iter_reference_type>::type
            {
                if (idx >= size(0)) {
                    throw yato::out_of_range_error("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx];
            }

            /**
             *  Get number of dimensions
             */
            YATO_CONSTEXPR_FUNC
            size_t dimensions_num() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_number;
            }

            /**
             * Get dimensions
             */
            YATO_CONSTEXPR_FUNC_EX
            dimensions_type dimensions() const
            {
                return dimensions_type(dimensions_range());
            }

            /**
             *  Get dimensions range
             */
            YATO_CONSTEXPR_FUNC_EX
            auto dimensions_range() const
                -> decltype(yato::range<desc_iterator>(m_desc_iter, std::next(m_desc_iter, dimensions_number)).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>()))
            {
                return yato::range<desc_iterator>(m_desc_iter, std::next(m_desc_iter, dimensions_number)).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>());
            }

            /**
             *  Get size along one dimension
             */
            YATO_CONSTEXPR_FUNC
            size_type size(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < dimensions_number)
                    ? std::get<dim_descriptor::idx_size>(*std::next(m_desc_iter, idx))
                    : (YATO_THROW_ASSERT_EXCEPT("yato::sub_array_proxy[size]: Dimension index is out of range"), static_cast<size_type>(0));
#else
                return std::get<dim_descriptor::idx_size>(*std::next(m_desc_iter, idx));
#endif
            }

            /**
             *  Get stride along one dimension
             */
            YATO_CONSTEXPR_FUNC
            size_type stride(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < dimensions_number - 1)
                    ? get_stride_(idx)
                    : (YATO_THROW_ASSERT_EXCEPT("yato::sub_array_proxy[size]: Dimension index is out of range"), static_cast<size_type>(0));
#else
                return get_stride_(idx);
#endif
            }

            /**
             *  Get total size of multidimensional proxy
             */
            YATO_CONSTEXPR_FUNC
            size_type total_size() const YATO_NOEXCEPT_KEYWORD
            {
                return std::get<dim_descriptor::idx_total>(*m_desc_iter);
            }

            /**
             * Get total number of elements in the view with strides
             */
            YATO_CONSTEXPR_FUNC
            size_type total_reserved() const
            {
                return std::get<dim_descriptor::idx_offset>(*m_desc_iter);
            }

            /**
             *  Get begin iterator for going through arrays of lower dimensionality
             */
            template<size_t MyDimsNum = dimensions_number>
            auto begin() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(MyDimsNum > 1), iterator>::type
            {
                return create_sub_proxy_(0);
            }

            /**
            *  Get begin iterator for going through arrays of lower dimensionality
            */
            template<size_t MyDimsNum = dimensions_number>
            auto begin() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(MyDimsNum == 1), data_iterator>::type
            {
                return plain_begin();
            }

            /**
             *  Get begin iterator for going through arrays of lower dimensionality
             */
            template<size_t MyDimsNum = dimensions_number>
            auto cbegin() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(MyDimsNum > 1), iterator>::type
            {
                return create_sub_proxy_(0);
            }

            /**
            *  Get begin iterator for going through arrays of lower dimensionality
            */
            template<size_t MyDimsNum = dimensions_number>
            auto cbegin() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(MyDimsNum == 1), data_iterator>::type
            {
                return plain_begin();
            }

            /**
             *  Get end iterator for going through arrays of lower dimensionality
             */
            template<size_t MyDimsNum = dimensions_number>
            auto end() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(MyDimsNum > 1), iterator>::type
            {
                return create_sub_proxy_(size(0));
            }

            /**
            *  Get end iterator for going through arrays of lower dimensionality
            */
            template<size_t MyDimsNum = dimensions_number>
            auto end() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(MyDimsNum == 1), data_iterator>::type
            {
                return plain_end();
            }

            /**
             *  Get end iterator for going through arrays of lower dimensionality
             */
            template<size_t MyDimsNum = dimensions_number>
            auto cend() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(MyDimsNum > 1), iterator>::type
            {
                return create_sub_proxy_(size(0));
            }

            /**
            *  Get end iterator for going through arrays of lower dimensionality
            */
            template<size_t MyDimsNum = dimensions_number>
            auto cend() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(MyDimsNum == 1), data_iterator>::type
            {
                return plain_end();
            }

            /**
             *  Get begin iterator for going through all elements of all dimensions
             */
            data_iterator plain_begin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter;
            }

            /**
             *  Get end iterator for going through all elements of all dimensions
             */
            data_iterator plain_end() const YATO_NOEXCEPT_KEYWORD
            {
                return std::next(m_data_iter, total_reserved());
            }

            /**
             *  Get begin iterator for going through all elements of all dimensions
             */
            data_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter;
            }

            /**
             *  Get end iterator for going through all elements of all dimensions
             */
            data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
            {
                return std::next(m_data_iter, total_reserved());
            }

            /**
             *  Get range of iterators for going through the top dimension
             */
            yato::range<iterator> range() const YATO_NOEXCEPT_KEYWORD
            {
                return make_range(begin(), end());
            }

            /**
             *  Get range of iterators for going through all elements of all dimensions
             */
            yato::range<data_iterator> plain_range() const YATO_NOEXCEPT_KEYWORD
            {
                return make_range(plain_begin(), plain_end());
            }

            /**
             *  Return the current proxy
             *  Is necessary for supporting ranged 'for' 
             */
            YATO_CONSTEXPR_FUNC
            const this_type & operator* () const
            {
                return *this;
            }

            /**
             *  Return the current proxy
             *  Is necessary for supporting ranged 'for'
             */
            this_type & operator* ()
            {
                return *this;
            }

            /**
             *  Increment iterator
             */
            YATO_CONSTEXPR_FUNC_EX
            this_type & operator++() YATO_NOEXCEPT_KEYWORD
            {
                std::advance(m_data_iter, total_reserved());
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
            YATO_CONSTEXPR_FUNC_EX
            this_type & operator--() YATO_NOEXCEPT_KEYWORD
            {
                std::advance(m_data_iter, -narrow_cast<difference_type>(total_reserved()));
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
            this_type & operator+= (difference_type offset) YATO_NOEXCEPT_KEYWORD
            {
                std::advance(m_data_iter, offset * total_reserved());
                return *this;
            }

            /**
             *  Add offset
             */
            friend 
            this_type operator+ (const this_type & iter, difference_type offset)
            {
                this_type temp(iter);
                return (temp += offset);
            }

            /**
             *  Add offset
             */
            friend 
            this_type operator+ (difference_type offset, const this_type & iter)
            {
                return iter + offset;
            }

            /**
             *  Subtract offset
             */
            this_type & operator-= (difference_type offset) YATO_NOEXCEPT_KEYWORD
            {
                std::advance(m_data_iter, -offset * narrow_cast<difference_type>(total_reserved()));
                return *this;
            }

            /**
             *  Subtract offset
             */
            friend 
            this_type operator- (const this_type & iter, difference_type offset)
            {
                this_type temp(iter);
                return (temp -= offset);
            }

            /**
             *  Distance between iterators
             *  Can be computed only between iterators of the same container. Otherwise result is undefined
             */
            friend 
            difference_type operator- (const this_type & one, const this_type & another)
            {
                YATO_REQUIRES(one.total_reserved() == another.total_reserved());
                return (one.m_data_iter - another.m_data_iter) / one.total_reserved();
            }

            /**
             *  Equal
             */
            friend
            bool operator== (const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
            {
                return one.m_data_iter == another.m_data_iter;
            }

            /**
             *  Not equal
             */
            friend
            bool operator!= (const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
            {
                return one.m_data_iter != another.m_data_iter;
            }

            /**
             *  Less
             */
            friend
            bool operator< (const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
            {
                return one.m_data_iter < another.m_data_iter;
            }

            /**
             *  Greater
             */
            friend
            bool operator> (const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
            {
                return one.m_data_iter > another.m_data_iter;
            }

            /**
             *  Less or equal
             */
            friend
            bool operator<= (const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
            {
                return one.m_data_iter <= another.m_data_iter;
            }

            /**
             *  Greater or equal
             */
            friend
            bool operator>= (const this_type & one, const this_type & another) YATO_NOEXCEPT_KEYWORD
            {
                return one.m_data_iter >= another.m_data_iter;
            }

            //-------------------------------------------------------
            
            template<typename, typename, size_t>
            friend class sub_array_proxy;
        };

    }

#ifdef YATO_MSVC
#pragma warning(pop)
#endif

}

#endif
