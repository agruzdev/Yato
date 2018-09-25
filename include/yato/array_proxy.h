/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_RPOXY_H_
#define _YATO_ARRAY_RPOXY_H_

#include <iterator>
#include "assert.h"
#include "types.h"
#include "range.h"
#include "container_base.h"

namespace yato
{

    namespace details
    {
        enum class proxy_access_policy
        {
            lvalue_ref,   ///< Return lvalue referrence to underlying data
            rvalue_ref    ///< Return rvalue referrence to underlying data
        };

        template <typename ValueType_, proxy_access_policy AccessPolicy_>
        struct proxy_access_traits
        { };

        template <typename ValueType_>
        struct proxy_access_traits<ValueType_, proxy_access_policy::lvalue_ref>
        {
            using reference      = std::add_lvalue_reference_t<ValueType_>;
            using plain_iterator = std::add_pointer_t<ValueType_>;
        };

        template <typename ValueType_>
        struct proxy_access_traits<ValueType_, proxy_access_policy::rvalue_ref>
        {
            using reference      = std::add_rvalue_reference_t<ValueType_>;
            using plain_iterator = std::move_iterator<std::add_pointer_t<ValueType_>>;
        };


        template<typename ValueType_, typename DimensionDescriptor_, size_t DimsNum_, proxy_access_policy AccessPolicy_ = proxy_access_policy::lvalue_ref>
        class sub_array_proxy;

        //-------------------------------------------------------
        // Proxy class for accessing multidimensional containers
        // Has trait of iterator
        // 

        template<typename ValueType_, typename DimensionDescriptor, size_t DimsNum, proxy_access_policy AccessPolicy_>
        class sub_array_proxy
        {
            static_assert(!std::is_reference<ValueType_>::value, "ValueType can't be reference");
            static_assert(DimsNum >= 1, "dimensions_number cant be 0");
        public:
            using this_type = sub_array_proxy <ValueType_, DimensionDescriptor, DimsNum, AccessPolicy_>;
            using dim_descriptor = DimensionDescriptor;
            using desc_iterator  = const typename DimensionDescriptor::type*;
            using data_iterator  = std::add_pointer_t<ValueType_>;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = DimsNum;
            static YATO_CONSTEXPR_VAR proxy_access_policy access_policy = AccessPolicy_;

            using sub_proxy = sub_array_proxy<ValueType_, dim_descriptor, dimensions_number - 1>;

            using data_value_type     = ValueType_;
            using data_pointer_type   = std::add_pointer_t<ValueType_>;
            using data_reference_type = typename proxy_access_traits<ValueType_, AccessPolicy_>::reference;

            // iterator traits
            using size_type         = typename DimensionDescriptor::size_type;
            using value_type        = this_type; // dereferencing returns itself for stl compatibility
            using pointer           = this_type*;
            using reference         = this_type&;
            using difference_type   = std::ptrdiff_t;
            using iterator_category = std::random_access_iterator_tag;

            using plain_iterator = typename proxy_access_traits<ValueType_, AccessPolicy_>::plain_iterator;
            using iterator       = sub_proxy;

            using dimensions_type = dimensionality<DimsNum, size_type>;
            //-------------------------------------------------------

        private:
            data_iterator m_data_iter;
            desc_iterator m_desc_iter;
            //-------------------------------------------------------

            template<size_t MyDimsNum_ = dimensions_number>
            auto get_stride_(size_t idx) const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if <(MyDimsNum_ > 1), size_type>::type
            {
                return dim_descriptor::template offset_to_bytes<data_value_type>(std::get<dim_descriptor::idx_offset>(*std::next(m_desc_iter, idx + 1)));
            }

            template<size_t MyDimsNum_ = dimensions_number>
            auto get_stride_(size_t) const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if <(MyDimsNum_ == 1), size_type>::type
            {
                return dim_descriptor::template offset_to_bytes<data_value_type>(std::get<dim_descriptor::idx_size>(*m_desc_iter));
            }

            YATO_CONSTEXPR_FUNC_CXX14
            sub_proxy create_sub_proxy_(size_t offset) const YATO_NOEXCEPT_KEYWORD
            {
                data_iterator sub_proxy_iter{ m_data_iter };
                details::advance_bytes(sub_proxy_iter, offset * dim_descriptor::template offset_to_bytes<data_value_type>(std::get<dim_descriptor::idx_offset>(*std::next(m_desc_iter))));
                return sub_proxy(sub_proxy_iter, std::next(m_desc_iter));
            }
            //-------------------------------------------------------

        public:
            YATO_CONSTEXPR_FUNC
            sub_array_proxy(const data_iterator & data, desc_iterator descriptors) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(data), m_desc_iter(descriptors)
            { }

            sub_array_proxy(const sub_array_proxy &) = default;

            template <typename AnotherDataIterator>
            sub_array_proxy(const sub_array_proxy<AnotherDataIterator, dim_descriptor, dimensions_number, access_policy> & other)
                : m_data_iter(other.m_data_iter), m_desc_iter(other.m_desc_iter)
            { }

            template <proxy_access_policy ProxyAccess_, typename =
                std::enable_if_t<ProxyAccess_ != access_policy>
            >
            explicit
            sub_array_proxy(const sub_array_proxy<data_value_type, dim_descriptor, dimensions_number, ProxyAccess_> & other)
                : m_data_iter(other.m_data_iter), m_desc_iter(other.m_desc_iter)
            { }


#ifndef YATO_MSVC_2013
            sub_array_proxy(sub_array_proxy &&) = default;
#else
            sub_array_proxy(sub_array_proxy && other) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(std::move(other.m_data_iter)), m_desc_iter(std::move(other.m_desc_iter))
            {}
#endif

            sub_array_proxy & operator= (const sub_array_proxy & other)
            {
                YATO_REQUIRES(this != &other);
                m_data_iter = other.m_data_iter;
                m_desc_iter = other.m_desc_iter;
                return *this;
            }

            sub_array_proxy & operator= (sub_array_proxy && other) YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(this != &other);
                m_data_iter = std::move(other.m_data_iter);
                m_desc_iter = std::move(other.m_desc_iter);
                return *this;
            }

            ~sub_array_proxy() = default;

            template<size_t MyDimsNum = dimensions_number>
            YATO_CONSTEXPR_FUNC_CXX14
            auto operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(MyDimsNum > 1), sub_proxy>::type
            {
                YATO_REQUIRES(idx < size(0));
                return create_sub_proxy_(idx);
            }

            template<size_t MyDimsNum = dimensions_number>
            YATO_CONSTEXPR_FUNC_CXX14
            auto operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if <(MyDimsNum == 1), data_reference_type>::type
            {
                YATO_REQUIRES(idx < size(0));
                return *std::next(m_data_iter, idx);
            }

            template<size_t MyDimsNum = dimensions_number, typename... _IdxTail>
            auto at(size_t idx, _IdxTail... tail) const
                -> typename std::enable_if <(MyDimsNum > 1), data_reference_type>::type
            {
                if (idx >= size(0)) {
                    throw yato::out_of_range_error("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx].at(tail...);
            }

            template<size_t MyDimsNum = dimensions_number>
            auto at(size_t idx) const
                -> typename std::enable_if <(MyDimsNum == 1), data_reference_type>::type
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
            YATO_CONSTEXPR_FUNC_CXX14
            dimensions_type dimensions() const
            {
                return dimensions_type(dimensions_range());
            }

            //non public interface
            YATO_CONSTEXPR_FUNC_CXX14
            yato::range<desc_iterator> descriptors_range_() const
            {
                return yato::range<desc_iterator>(m_desc_iter, std::next(m_desc_iter, dimensions_number));
            }

            /**
             *  Get dimensions range
             */
            YATO_CONSTEXPR_FUNC_CXX14
            auto dimensions_range() const
                -> decltype(descriptors_range_().map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>()))
            {
                return descriptors_range_().map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>());
            }

            /**
             *  Get size along one dimension
             */
            YATO_CONSTEXPR_FUNC_CXX14
            size_type size(size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < dimensions_number);
                return std::get<dim_descriptor::idx_size>(*std::next(m_desc_iter, idx));
            }

            /**
             *  Get byte offset till next sub-proxy
             *  Returns size in bytes for 1D proxy
             */
            YATO_CONSTEXPR_FUNC_CXX14
            size_type stride(size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < dimensions_number - 1);
                return get_stride_(idx);
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
            size_type total_stored() const
            {
                return dim_descriptor::template offset_to_bytes<data_value_type>(std::get<dim_descriptor::idx_offset>(*m_desc_iter));
            }

            /**
             * Check that proxy represents a continuous data segment and plain access can be used
             */
            template<size_t MyDimsNum_ = dimensions_number>
            YATO_CONSTEXPR_FUNC_CXX14
            auto continuous() const
                -> typename std::enable_if<(MyDimsNum_ > 1), bool>::type
            {
                const size_t stride_offset = dim_descriptor::template offset_to_bytes<data_value_type>(std::get<dim_descriptor::idx_offset>(*std::next(m_desc_iter)));
                const size_t elem_offset   = std::get<dim_descriptor::idx_total>(*std::next(m_desc_iter)) * sizeof(data_value_type);
                return (stride_offset == elem_offset);
            }

            /**
             * Check that proxy represents a continuous data segment and plain access can be used
             */
            template<size_t MyDimsNum_ = dimensions_number>
            YATO_CONSTEXPR_FUNC
            auto continuous() const
                -> typename std::enable_if<(MyDimsNum_ == 1), bool>::type
            {
                return true;
            }

            /**
             *  Get begin iterator for going through arrays of lower dimensionality
             */
            template<size_t MyDimsNum = dimensions_number>
            auto begin() const
                -> typename std::enable_if<(MyDimsNum > 1), iterator>::type
            {
                return create_sub_proxy_(0);
            }

            /**
            *  Get begin iterator for going through arrays of lower dimensionality
            */
            template<size_t MyDimsNum = dimensions_number>
            auto begin() const
                -> typename std::enable_if<(MyDimsNum == 1), plain_iterator>::type
            {
                return static_cast<plain_iterator>(plain_begin());
            }

            /**
             *  Get begin iterator for going through arrays of lower dimensionality
             */
            template<size_t MyDimsNum = dimensions_number>
            auto cbegin() const
                -> typename std::enable_if<(MyDimsNum > 1), iterator>::type
            {
                return create_sub_proxy_(0);
            }

            /**
            *  Get begin iterator for going through arrays of lower dimensionality
            */
            template<size_t MyDimsNum = dimensions_number>
            auto cbegin() const
                -> typename std::enable_if<(MyDimsNum == 1), plain_iterator>::type
            {
                return static_cast<plain_iterator>(plain_begin());
            }

            /**
             *  Get end iterator for going through arrays of lower dimensionality
             */
            template<size_t MyDimsNum = dimensions_number>
            auto end() const
                -> typename std::enable_if<(MyDimsNum > 1), iterator>::type
            {
                return create_sub_proxy_(size(0));
            }

            /**
            *  Get end iterator for going through arrays of lower dimensionality
            */
            template<size_t MyDimsNum = dimensions_number>
            auto end() const
                -> typename std::enable_if<(MyDimsNum == 1), plain_iterator>::type
            {
                return plain_end();
            }

            /**
             *  Get end iterator for going through arrays of lower dimensionality
             */
            template<size_t MyDimsNum = dimensions_number>
            auto cend() const
                -> typename std::enable_if<(MyDimsNum > 1), iterator>::type
            {
                return create_sub_proxy_(size(0));
            }

            /**
            *  Get end iterator for going through arrays of lower dimensionality
            */
            template<size_t MyDimsNum = dimensions_number>
            auto cend() const
                -> typename std::enable_if<(MyDimsNum == 1), plain_iterator>::type
            {
                return plain_end();
            }

            /**
             *  Get begin iterator for going through all elements of all dimensions
             */
            plain_iterator plain_begin() const
            {
                YATO_REQUIRES(continuous());
                return static_cast<plain_iterator>(m_data_iter);
            }

            /**
             *  Get end iterator for going through all elements of all dimensions
             */
            plain_iterator plain_end() const
            {
                YATO_REQUIRES(continuous());
                return static_cast<plain_iterator>(std::next(m_data_iter, total_size()));
            }

            /**
             *  Get begin iterator for going through all elements of all dimensions
             */
            plain_iterator plain_cbegin() const
            {
                YATO_REQUIRES(continuous());
                return static_cast<plain_iterator>(m_data_iter);
            }

            /**
             *  Get end iterator for going through all elements of all dimensions
             */
            plain_iterator plain_cend() const
            {
                YATO_REQUIRES(continuous());
                return static_cast<plain_iterator>(std::next(m_data_iter, total_size()));
            }

            /**
             *  Get range of iterators for going through the top dimension
             */
            yato::range<iterator> range() const
            {
                return make_range(begin(), end());
            }

            /**
             *  Get range of iterators for going through all elements of all dimensions
             */
            yato::range<plain_iterator> plain_range() const
            {
                return make_range(plain_begin(), plain_end());
            }

            /**
             *  Get raw pointer to underlying data
             */
            data_pointer_type data() const YATO_NOEXCEPT_KEYWORD
            {
                return &(*m_data_iter);
            }

            /**
             *  Return the current proxy
             *  Is necessary for supporting ranged 'for' 
             */
            YATO_CONSTEXPR_FUNC
            reference operator* () const
            {
                return const_cast<reference>(*this);
            }

            /**
             *  Increment iterator
             */
            YATO_CONSTEXPR_FUNC_CXX14
            this_type & operator++() YATO_NOEXCEPT_KEYWORD
            {
                details::advance_bytes(m_data_iter, total_stored());
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
                details::advance_bytes(m_data_iter, -narrow_cast<difference_type>(total_stored()));
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
                details::advance_bytes(m_data_iter, offset * total_stored());
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
                details::advance_bytes(m_data_iter, -offset * narrow_cast<difference_type>(total_stored()));
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
                YATO_REQUIRES(one.total_stored() == another.total_stored());
                using byte_pointer = const volatile uint8_t*;
                return (yato::pointer_cast<byte_pointer>(one.m_data_iter) - yato::pointer_cast<byte_pointer>(another.m_data_iter)) / one.total_stored();
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

            template<typename, typename, size_t, proxy_access_policy>
            friend class sub_array_proxy;
        };

        template <typename ProxyValue_, typename ProxyDimension_, size_t ProxyDims_, proxy_access_policy ProxyAccess_>
        YATO_CONSTEXPR_FUNC
        auto make_move_iterator(const sub_array_proxy<ProxyValue_, ProxyDimension_, ProxyDims_, ProxyAccess_> & it)
            -> sub_array_proxy<ProxyValue_, ProxyDimension_, ProxyDims_, proxy_access_policy::rvalue_ref>
        {
            return sub_array_proxy<ProxyValue_, ProxyDimension_, ProxyDims_, proxy_access_policy::rvalue_ref>(it);
        }

    }

    using details::make_move_iterator;

}

#endif
