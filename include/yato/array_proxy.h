/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_PROXY_H_
#define _YATO_ARRAY_PROXY_H_

#include <iterator>
#include "assertion.h"
#include "types.h"
#include "range.h"

#include "container_nd.h"

namespace yato
{
    namespace details
    {

        template <typename Ty_, typename CvTy_>
        struct convertible_view_type
            : yato::boolean_constant<!std::is_same<Ty_, CvTy_>::value &&
                (std::is_same<std::add_const_t<Ty_>, CvTy_>::value ||
                 std::is_same<std::add_volatile_t<Ty_>, CvTy_>::value ||
                 std::is_same<std::add_cv_t<Ty_>, CvTy_>::value)
            >
        { };

    }


    template <typename ValueType_, typename DimensionDescriptor_, size_t DimsNum_, proxy_access_policy AccessPolicy_ = proxy_access_policy::lvalue_ref>
    class proxy_nd;

    template <typename ProxyType_>
    class iterator_nd;


    /**
     * Provides access to miltidimensional data
     */
    template <typename ValueType_, typename DimensionDescriptor_, size_t DimsNum_, proxy_access_policy AccessPolicy_>
    class proxy_nd
        : public details::choose_container_interface_t<ValueType_, DimsNum_, proxy_nd<ValueType_, DimensionDescriptor_, DimsNum_, AccessPolicy_>>
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType can't be reference");
        static_assert(DimsNum_ >= 1, "dimensions_number cant be 0");
    public:
        using this_type      = proxy_nd <ValueType_, DimensionDescriptor_, DimsNum_, AccessPolicy_>;
        using value_type     = ValueType_;
        using size_type      = size_t;
        using dim_descriptor = DimensionDescriptor_;
        using desc_iterator  = std::add_pointer_t<std::add_const_t<typename DimensionDescriptor_::type>>;
        using data_iterator  = std::add_pointer_t<ValueType_>;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = DimsNum_;
        static YATO_CONSTEXPR_VAR proxy_access_policy access_policy = AccessPolicy_;

        using sub_view       = proxy_nd<value_type, dim_descriptor, dimensions_number - 1, access_policy>;
        using const_sub_view = proxy_nd<typename proxy_access_traits<value_type, access_policy>::const_value_type, dim_descriptor, dimensions_number - 1, access_policy>;

        using iterator              = iterator_nd<sub_view>;
        using const_iterator        = iterator_nd<const_sub_view>;
        using plain_iterator        = typename proxy_access_traits<value_type, access_policy>::plain_iterator;
        using const_plain_iterator  = typename proxy_access_traits<value_type, access_policy>::const_plain_iterator;

        using reference             = sub_view;
        using const_reference       = const_sub_view;
        using value_reference       = typename proxy_access_traits<value_type, access_policy>::reference;
        using const_value_reference = typename proxy_access_traits<value_type, access_policy>::const_reference;

        using dimensions_type = dimensionality<dimensions_number, size_type>;

        static YATO_CONSTEXPR_VAR container_tag container_category = container_tag::general;
        //-------------------------------------------------------

        template <proxy_access_policy Py_>
        using rebind_access_t = proxy_nd<ValueType_, DimensionDescriptor_, DimsNum_, Py_>;
        //-------------------------------------------------------

    private:
        data_iterator m_data_iter;
        desc_iterator m_desc_iter;
        //-------------------------------------------------------

    protected:
        std::add_pointer_t<ValueType_> & raw_ptr_()
        {
            return m_data_iter;
        }

        std::add_const_t<std::add_pointer_t<ValueType_>> & raw_ptr_() const
        {
            return m_data_iter;
        }

        YATO_CONSTEXPR_FUNC_CXX14
        sub_view create_sub_view_(size_t offset) const YATO_NOEXCEPT_KEYWORD
        {
            data_iterator sub_proxy_iter{ m_data_iter };
            details::advance_bytes(sub_proxy_iter, offset * dim_descriptor::template offset_to_bytes<value_type>(std::get<dim_descriptor::idx_offset>(*yato::next(m_desc_iter))));
            return sub_view(sub_proxy_iter, yato::next(m_desc_iter));
        }

        YATO_CONSTEXPR_FUNC_CXX14
        const_sub_view create_const_sub_view_(size_t offset) const YATO_NOEXCEPT_KEYWORD
        {
            data_iterator sub_proxy_iter{ m_data_iter };
            details::advance_bytes(sub_proxy_iter, offset * dim_descriptor::template offset_to_bytes<value_type>(std::get<dim_descriptor::idx_offset>(*yato::next(m_desc_iter))));
            return const_sub_view(sub_proxy_iter, yato::next(m_desc_iter));
        }
        //-------------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        proxy_nd(const data_iterator & data, const desc_iterator & descriptors) YATO_NOEXCEPT_KEYWORD
            : m_data_iter(data), m_desc_iter(descriptors)
        { }

        proxy_nd(const proxy_nd &) = default;

        template <typename AnotherValue_>
        proxy_nd(const proxy_nd<AnotherValue_, dim_descriptor, dimensions_number, access_policy> & other)
            : m_data_iter(other.m_data_iter), m_desc_iter(other.m_desc_iter)
        { }

        template <proxy_access_policy ProxyAccess_, typename =
            std::enable_if_t<ProxyAccess_ != access_policy>
        >
        explicit
        proxy_nd(const proxy_nd<value_type, dim_descriptor, dimensions_number, ProxyAccess_> & other)
            : m_data_iter(other.m_data_iter), m_desc_iter(other.m_desc_iter)
        { }

        proxy_nd(proxy_nd &&) YATO_NOEXCEPT_KEYWORD = default;

        proxy_nd & operator= (const proxy_nd & other)
        {
            YATO_REQUIRES(this != &other);
            m_data_iter = other.m_data_iter;
            m_desc_iter = other.m_desc_iter;
            return *this;
        }

        proxy_nd & operator= (proxy_nd && other) YATO_NOEXCEPT_KEYWORD
        {
            YATO_REQUIRES(this != &other);
            m_data_iter = std::move(other.m_data_iter);
            m_desc_iter = std::move(other.m_desc_iter);
            return *this;
        }

        ~proxy_nd() = default;

        YATO_CONSTEXPR_FUNC_CXX14
        reference operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
        {
            YATO_REQUIRES(idx < size(0));
            return create_sub_view_(idx);
        }

        template<typename... IdxTail_>
        value_reference at(size_t idx, IdxTail_... tail) const
        {
            if (idx >= size(0)) {
                throw yato::out_of_range_error("yato::array_sub_view_nd: out of range!");
            }
            return create_sub_view_(idx).at(tail...);
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

        /**
         * Get strides
         */
        YATO_CONSTEXPR_FUNC_CXX14
        strides_array<dimensions_number - 1, size_type> strides() const
        {
            return strides_array<dimensions_number - 1, size_type>(strides_range());
        }

        //non public interface
        YATO_CONSTEXPR_FUNC_CXX14
        yato::range<desc_iterator> descriptors_range_() const
        {
            return yato::range<desc_iterator>(m_desc_iter, yato::next(m_desc_iter, dimensions_number));
        }

        /**
         *  Get dimensions range
         */
        YATO_CONSTEXPR_FUNC_CXX14
        auto dimensions_range() const
            -> decltype(descriptors_range_().map(tuple_cgetter<dim_descriptor::idx_size>()))
        {
            return descriptors_range_().map(tuple_cgetter<dim_descriptor::idx_size>());
        }

        /**
         * Get strides range
         */
        YATO_CONSTEXPR_FUNC_CXX14
        auto strides_range() const
        {
            return descriptors_range_().tail().map([](const typename dim_descriptor::type & d) {
                return dim_descriptor::template offset_to_bytes<value_type>(std::get<dim_descriptor::idx_offset>(d));
            });
        }

        /**
         *  Get size along one dimension
         */
        YATO_CONSTEXPR_FUNC_CXX14
        size_type size(size_t idx) const YATO_NOEXCEPT_KEYWORD
        {
            YATO_REQUIRES(idx < dimensions_number);
            return std::get<dim_descriptor::idx_size>(*yato::next(m_desc_iter, idx));
        }

        /**
         *  Get size along one dimension
         */
        YATO_CONSTEXPR_FUNC
        size_type size() const YATO_NOEXCEPT_KEYWORD
        {
            return std::get<dim_descriptor::idx_size>(*m_desc_iter);
        }

        /**
         *  Get byte offset till next sub-proxy
         *  Returns size in bytes for 1D proxy
         */
        YATO_CONSTEXPR_FUNC_CXX14
        size_type stride(size_t idx) const YATO_NOEXCEPT_KEYWORD
        {
            YATO_REQUIRES(idx < dimensions_number - 1);
            return dim_descriptor::template offset_to_bytes<value_type>(std::get<dim_descriptor::idx_offset>(*yato::next(m_desc_iter, idx + 1)));
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
            return dim_descriptor::template offset_to_bytes<value_type>(std::get<dim_descriptor::idx_offset>(*m_desc_iter));
        }

        /**
         * Check that proxy represents a continuous data segment and plain access can be used
         */
        YATO_CONSTEXPR_FUNC_CXX14
        bool continuous() const
        {
            const size_t stride_offset = dim_descriptor::template offset_to_bytes<value_type>(std::get<dim_descriptor::idx_offset>(*yato::next(m_desc_iter)));
            const size_t elem_offset   = std::get<dim_descriptor::idx_total>(*yato::next(m_desc_iter)) * sizeof(value_type);
            return (stride_offset == elem_offset);
        }

        /**
         *  Get begin iterator for going through arrays of lower dimensionality
         */
        iterator begin() const
        {
            return static_cast<iterator>(create_sub_view_(0));
        }

        /**
         *  Get begin iterator for going through arrays of lower dimensionality
         */
        const_iterator cbegin() const
        {
            return static_cast<const_iterator>(create_const_sub_view_(0));
        }

        /**
         *  Get end iterator for going through arrays of lower dimensionality
         */
        iterator end() const
        {
            return static_cast<iterator>(create_sub_view_(size(0)));
        }

        /**
         *  Get end iterator for going through arrays of lower dimensionality
         */
        const_iterator cend() const
        {
            return static_cast<const_iterator>(create_const_sub_view_(size(0)));
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
            return static_cast<plain_iterator>(yato::next(m_data_iter, total_size()));
        }

        /**
         *  Get begin iterator for going through all elements of all dimensions
         */
        const_plain_iterator plain_cbegin() const
        {
            YATO_REQUIRES(continuous());
            return static_cast<const_plain_iterator>(m_data_iter);
        }

        /**
         *  Get end iterator for going through all elements of all dimensions
         */
        const_plain_iterator plain_cend() const
        {
            YATO_REQUIRES(continuous());
            return static_cast<const_plain_iterator>(yato::next(m_data_iter, total_size()));
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
         *  Get range of iterators for going through the top dimension
         */
        yato::range<const_iterator> crange() const
        {
            return make_range(begin(), end());
        }

        /**
         *  Get range of iterators for going through all elements of all dimensions
         */
        yato::range<const_plain_iterator> plain_crange() const
        {
            return make_range(plain_begin(), plain_end());
        }

        /**
         *  Get raw pointer to underlying data
         */
        std::add_pointer_t<value_type> data() const YATO_NOEXCEPT_KEYWORD
        {
            return m_data_iter;
        }

        /**
         *  Get raw pointer to underlying data
         */
        std::add_pointer_t<std::add_const_t<value_type>> cdata() const YATO_NOEXCEPT_KEYWORD
        {
            return m_data_iter;
        }

        template<typename, typename, size_t, proxy_access_policy>
        friend class proxy_nd;

        template<typename>
        friend class iterator_nd;
    };




    template <typename ValueType_, typename DimensionDescriptor_, proxy_access_policy AccessPolicy_>
    class proxy_nd<ValueType_, DimensionDescriptor_, 1, AccessPolicy_>
        : public details::choose_container_interface_t<ValueType_, 1, proxy_nd<ValueType_, DimensionDescriptor_, 1, AccessPolicy_>>
    {
        static_assert(!std::is_reference<ValueType_>::value, "ValueType can't be reference");
    public:
        using this_type      = proxy_nd <ValueType_, DimensionDescriptor_, 1, AccessPolicy_>;
        using value_type     = ValueType_;
        using size_type      = size_t;
        using dim_descriptor = DimensionDescriptor_;
        using data_iterator  = std::add_pointer_t<ValueType_>;
        using desc_iterator  = std::add_pointer_t<std::add_const_t<typename DimensionDescriptor_::type>>;
        static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;
        static YATO_CONSTEXPR_VAR proxy_access_policy access_policy = AccessPolicy_;

        using plain_iterator        = typename proxy_access_traits<value_type, access_policy>::plain_iterator;
        using const_plain_iterator  = typename proxy_access_traits<value_type, access_policy>::const_plain_iterator;
        using iterator              = plain_iterator;
        using const_iterator        = const_plain_iterator;

        using reference             = typename proxy_access_traits<value_type, access_policy>::reference;
        using const_reference       = typename proxy_access_traits<value_type, access_policy>::const_reference;
        using value_reference       = reference;
        using const_value_reference = const_reference;

        using dimensions_type = dimensionality<dimensions_number, size_type>;

        static YATO_CONSTEXPR_VAR container_tag container_category = container_tag::continuous;
        //-------------------------------------------------------

        template <proxy_access_policy Py_>
        using rebind_access_t = proxy_nd <ValueType_, DimensionDescriptor_, 1, Py_>;
        //-------------------------------------------------------

    private:
        data_iterator m_data_iter;
        const size_t* m_size_ptr;
        const size_t* m_stride_ptr;
        //-------------------------------------------------------

    protected:
        std::add_pointer_t<ValueType_> & raw_ptr_()
        {
            return m_data_iter;
        }

        std::add_const_t<std::add_pointer_t<ValueType_>> & raw_ptr_() const
        {
            return m_data_iter;
        }
        //-------------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        proxy_nd(const data_iterator & data, const size_t* size, const size_t* stride) YATO_NOEXCEPT_KEYWORD
            : m_data_iter(data), m_size_ptr(size), m_stride_ptr(stride)
        {
            YATO_REQUIRES(m_size_ptr   != nullptr);
            YATO_REQUIRES(m_stride_ptr != nullptr);
        }

        YATO_CONSTEXPR_FUNC
        proxy_nd(const data_iterator & data, const desc_iterator & descriptors) YATO_NOEXCEPT_KEYWORD
            : m_data_iter(data), m_size_ptr(&std::get<dim_descriptor::idx_size>(*descriptors)), m_stride_ptr(&std::get<dim_descriptor::idx_offset>(*descriptors))
        { }

        proxy_nd(const proxy_nd &) = default;

        template <typename AnotherValue_>
        proxy_nd(const proxy_nd<AnotherValue_, dim_descriptor, dimensions_number, access_policy> & other)
            : m_data_iter(other.m_data_iter), m_size_ptr(other.m_size_ptr), m_stride_ptr(other.m_stride_ptr)
        { }

        template <proxy_access_policy ProxyAccess_, typename =
            std::enable_if_t<ProxyAccess_ != access_policy>
        >
        explicit
        proxy_nd(const proxy_nd<value_type, dim_descriptor, dimensions_number, ProxyAccess_> & other)
            : m_data_iter(other.m_data_iter), m_size_ptr(other.m_size_ptr), m_stride_ptr(other.m_stride_ptr)
        { }

        proxy_nd(proxy_nd &&) YATO_NOEXCEPT_KEYWORD = default;

        proxy_nd & operator= (const proxy_nd & other)
        {
            YATO_REQUIRES(this != &other);
            m_data_iter  = other.m_data_iter;
            m_size_ptr   = other.m_size_ptr;
            m_stride_ptr = other.m_stride_ptr;
            return *this;
        }

        proxy_nd & operator= (proxy_nd && other) YATO_NOEXCEPT_KEYWORD
        {
            YATO_REQUIRES(this != &other);
            m_data_iter  = std::move(other.m_data_iter);
            m_size_ptr   = std::move(other.m_size_ptr);
            m_stride_ptr = std::move(other.m_stride_ptr);
            return *this;
        }

        ~proxy_nd() = default;

        YATO_CONSTEXPR_FUNC_CXX14
        reference operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
        {
            YATO_REQUIRES(idx < size(0));
            return *yato::next(m_data_iter, idx);
        }

        value_reference at(size_t idx) const
        {
            if (idx >= size(0)) {
                throw yato::out_of_range_error("yato::proxy_nd: out of range!");
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

        /**
         * Get dimensions
         */
        YATO_CONSTEXPR_FUNC_CXX14
        strides_array<dimensions_number - 1, size_type> strides() const
        {
            return strides_array<dimensions_number - 1, size_type>(strides_range());
        }

        /**
         *  Get dimensions range
         */
        YATO_CONSTEXPR_FUNC_CXX14
        auto dimensions_range() const
        {
            return yato::range<const size_t*>(m_size_ptr, yato::next(m_size_ptr));
        }

        /**
         *  Get sstrides range
         */
        YATO_CONSTEXPR_FUNC_CXX14
        auto strides_range() const
        {
            return yato::range<const size_type*>(nullptr, nullptr);
        }

        /**
         *  Get size along one dimension
         */
        YATO_CONSTEXPR_FUNC_CXX14
        size_type size(size_t idx = 0) const YATO_NOEXCEPT_KEYWORD
        {
            YATO_MAYBE_UNUSED(idx);
            YATO_REQUIRES(idx < dimensions_number);
            return *m_size_ptr;
        }

        /**
         *  Get byte offset till next sub-proxy
         */
        YATO_CONSTEXPR_FUNC_CXX14
        size_type stride(size_t idx) const YATO_NOEXCEPT_KEYWORD
        {
            YATO_MAYBE_UNUSED(idx);
            YATO_REQUIRES(false && "No stride for 1D");
            return 0;
        }

        /**
         *  Get total size of multidimensional proxy
         */
        YATO_CONSTEXPR_FUNC
        size_type total_size() const YATO_NOEXCEPT_KEYWORD
        {
            return size(0);
        }

        /**
         * Get total number of elements in the view with strides
         */
        YATO_CONSTEXPR_FUNC
        size_type total_stored() const
        {
            return dim_descriptor::template offset_to_bytes<value_type>(*m_stride_ptr);
        }

        /**
         * Check that proxy represents a continuous data segment and plain access can be used
         */
        YATO_CONSTEXPR_FUNC
        bool continuous() const
        {
            return true;
        }

        /**
         *  Get begin iterator for going through arrays of lower dimensionality
         */
        iterator begin() const
        {
            return plain_begin();
        }

        /**
         *  Get begin iterator for going through arrays of lower dimensionality
         */
        const_iterator cbegin() const
        {
            return plain_begin();
        }

        /**
         *  Get end iterator for going through arrays of lower dimensionality
         */
        iterator end() const
        {
            return plain_end();
        }

        /**
         *  Get end iterator for going through arrays of lower dimensionality
         */
        const_plain_iterator cend() const
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
            return static_cast<plain_iterator>(yato::next(m_data_iter, total_size()));
        }

        /**
         *  Get begin iterator for going through all elements of all dimensions
         */
        const_plain_iterator plain_cbegin() const
        {
            YATO_REQUIRES(continuous());
            return static_cast<const_plain_iterator>(m_data_iter);
        }

        /**
         *  Get end iterator for going through all elements of all dimensions
         */
        const_plain_iterator plain_cend() const
        {
            YATO_REQUIRES(continuous());
            return static_cast<const_plain_iterator>(yato::next(m_data_iter, total_size()));
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
         *  Get range of iterators for going through the top dimension
         */
        yato::range<const_iterator> crange() const
        {
            return make_range(begin(), end());
        }

        /**
         *  Get range of iterators for going through all elements of all dimensions
         */
        yato::range<const_plain_iterator> plain_crange() const
        {
            return make_range(plain_begin(), plain_end());
        }

        /**
         *  Get raw pointer to underlying data
         */
        std::add_pointer_t<value_type> data() const YATO_NOEXCEPT_KEYWORD
        {
            return m_data_iter;
        }

        /**
         *  Get raw pointer to underlying data
         */
        std::add_pointer_t<std::add_const_t<value_type>> cdata() const YATO_NOEXCEPT_KEYWORD
        {
            return m_data_iter;
        }

        template<typename, typename, size_t, proxy_access_policy>
        friend class proxy_nd;
    };

}

#endif //_YATO_ARRAY_PROXY_H_
