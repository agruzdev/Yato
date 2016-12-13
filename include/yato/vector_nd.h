/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_ND_H_
#define _YATO_ARRAY_ND_H_

#include <array>
#include <vector>
#include "assert.h"
#include "array_view.h"
#include "range.h"

namespace yato
{
   
    namespace details
    {
        /*
         * Make type of multidimensional initializer list
         */
        template<typename _T, size_t _Dims>
        struct initilizer_list_nd
        {
            using type = std::initializer_list<typename initilizer_list_nd<_T, _Dims - 1>::type>;
        };

        template<typename _T>
        struct initilizer_list_nd<_T, 1>
        {
            using type = std::initializer_list<_T>;
        };


        //-------------------------------------------------------
        // Generic implementation

        template <typename _DataType, size_t _DimensionsNum, typename _Allocator>
        class vector_nd_impl
        {
        public:
            /*
             * Public traits of the multidimensional vector
             */
            using my_type = vector_nd_impl<_DataType, _DimensionsNum, _Allocator>;
            using dimensions_type = dimensionality<_DimensionsNum, size_t>;
            using data_type = _DataType;
            using allocator_type = _Allocator;
            using container_type = std::vector<data_type, allocator_type>;
            using data_iterator = typename container_type::iterator;
            using const_data_iterator = typename container_type::const_iterator;
            using reference = decltype(*std::declval<data_iterator>());
            using const_reference = decltype(*std::declval<const_data_iterator>());
            using size_type = size_t;

            using dim_descriptor = dimension_descriptor<size_type>;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = _DimensionsNum;
            static_assert(dimensions_number > 1, "Implementation for dimensions number larger than 1");
            //-------------------------------------------------------

        private:
            using size_iterator = typename dimensions_type::iterator;
            using size_const_iterator = typename dimensions_type::const_iterator;

            template<size_t _Dims>
            using initilizer_type = typename initilizer_list_nd<data_type, _Dims>::type;

            template<typename SomeDataIter, typename SomeDescriptor>
            using proxy_tmpl = details::sub_array_proxy<SomeDataIter, SomeDescriptor, dimensions_number - 1>;

            using proxy       = proxy_tmpl<data_iterator,       dim_descriptor>;
            using const_proxy = proxy_tmpl<const_data_iterator, dim_descriptor>;

        public:
            using iterator = proxy;
            using const_iterator = const_proxy;

            //-------------------------------------------------------

        private:
            std::array<dim_descriptor::type, dimensions_number> m_descriptors = {};
            container_type  m_plain_vector;
            //-------------------------------------------------------

            proxy create_proxy_(size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                return proxy(std::next(m_plain_vector.begin(), offset * std::get<dim_descriptor::idx_total>(m_descriptors[1])), &m_descriptors[1]);
            }

            proxy create_proxy_(data_iterator plain_position) YATO_NOEXCEPT_KEYWORD
            {
                return proxy(plain_position, &m_descriptors[1]);
            }

            const_proxy create_const_proxy_(size_t offset) const YATO_NOEXCEPT_KEYWORD
            {
                return const_proxy(std::next(m_plain_vector.cbegin(), offset * std::get<dim_descriptor::idx_total>(m_descriptors[1])), &m_descriptors[1]);
            }

            const_proxy create_const_proxy_(const_data_iterator plain_position) const YATO_NOEXCEPT_KEYWORD
            {
                return const_proxy(plain_position, &m_descriptors[1]);
            }

            void init_subsizes_() YATO_NOEXCEPT_KEYWORD
            {
                std::get<dim_descriptor::idx_total>(m_descriptors[dimensions_number - 1]) = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - 1]);
                for (size_t i = dimensions_number - 1; i > 0; --i) {
                    std::get<dim_descriptor::idx_total>(m_descriptors[i - 1]) = std::get<dim_descriptor::idx_size>(m_descriptors[i - 1]) * std::get<dim_descriptor::idx_total>(m_descriptors[i]);
                }
            }

            void init_sizes_(const dimensions_type & extents) YATO_NOEXCEPT_KEYWORD
            {
                m_descriptors[dimensions_number - 1] = std::make_tuple(extents[dimensions_number - 1], extents[dimensions_number - 1]);
                for (size_t i = dimensions_number - 1; i > 0; --i) {
                    m_descriptors[i - 1] = std::make_tuple(extents[i - 1], extents[i - 1] * std::get<dim_descriptor::idx_total>(m_descriptors[i]));
                }
            }

            auto dimensions_ref_range_()
                -> decltype(yato::make_range(m_descriptors).map(tuple_getter<typename dim_descriptor::type, dim_descriptor::idx_size>()))
            {
                return yato::make_range(m_descriptors).map(tuple_getter<typename dim_descriptor::type, dim_descriptor::idx_size>());
            }

            template<typename RangeType>
            void init_sizes_(RangeType range) YATO_NOEXCEPT_IN_RELEASE
            {
                YATO_REQUIRES(range.distance() == dimensions_number);
                std::copy(range.begin(), range.end(), dimensions_ref_range_().begin());
                init_subsizes_();
            }

            template<size_t _Depth>
            auto init_sizes_(const initilizer_type<_Depth> & init_list) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if < (_Depth > 1), void > ::type
            {
                std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - _Depth]) = init_list.size();
                init_sizes_<_Depth - 1>(*(init_list.begin()));
            }
            
            template<size_t _Depth>
            auto init_sizes_(const initilizer_type<_Depth> & init_list) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_Depth == 1), void>::type
            {
                std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - _Depth]) = init_list.size();
                init_subsizes_();
            }

            template<size_t _Depth, typename _Iter>
            auto init_values_(const initilizer_type<_Depth> & init_list, _Iter & iter) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if < (_Depth > 1), void > ::type
            {
                for (const auto & init_sub_list : init_list) {
                    init_values_<_Depth - 1>(init_sub_list, iter);
                }
            }

            template<size_t _Depth, typename _Iter>
            auto init_values_(const initilizer_type<_Depth> & init_list, _Iter & iter) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_Depth == 1), void>::type
            {
                for (const auto & value : init_list) {
                    *iter++ = value;
                }
            }

            void update_top_dimension_(size_t new_size) YATO_NOEXCEPT_KEYWORD
            {
                std::get<dim_descriptor::idx_size>(m_descriptors[0])  = new_size;
                std::get<dim_descriptor::idx_total>(m_descriptors[0]) = std::get<dim_descriptor::idx_total>(m_descriptors[1]) * new_size;
            }

            size_type get_top_dimension_() const YATO_NOEXCEPT_KEYWORD
            {
                return std::get<dim_descriptor::idx_size>(m_descriptors[0]);
            }

            template<typename _SizeIterator>
            yato::range<data_iterator> prepare_push_back_(const yato::range<_SizeIterator> & sub_dims)
            {
                const size_t old_size = m_plain_vector.size();
                auto current_sub_dims = dimensions_ref_range_().tail();
                if (old_size > 0) {
                    //if (!std::equal(std::next(m_dimensions.cbegin()), m_dimensions.cend(), sub_dims.begin())) {
                    if (!std::equal(current_sub_dims.begin(), current_sub_dims.end(), sub_dims.begin())) {
                        YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[push_back]: Cannot push subvector with a different shape");
                    }
                }
                else {
                    std::copy(sub_dims.begin(), sub_dims.end(), current_sub_dims.begin());
                    init_subsizes_();
                }
                update_top_dimension_(get_top_dimension_() + 1);
                m_plain_vector.resize(total_size());
                return yato::make_range(std::next(m_plain_vector.begin(), old_size), m_plain_vector.end());
            }

            template<typename _SizeIterator, typename _Iterator>
            yato::range<data_iterator> prepare_insert_(const yato::range<_SizeIterator> & sub_dims, const _Iterator & position, size_t length)
            {
                const size_t old_size = m_plain_vector.size();
                auto current_sub_dims = dimensions_ref_range_().tail();
                if (old_size > 0) {
                    //if (!std::equal(std::next(m_dimensions.cbegin()), m_dimensions.cend(), sub_dims.begin())) {
                    if (!std::equal(current_sub_dims.begin(), current_sub_dims.end(), sub_dims.begin())) {
                        YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[insert]: Cannot insert sub-vector with a different shape");
                    }
                }
                else {
                    std::copy(sub_dims.begin(), sub_dims.end(), current_sub_dims.begin());
                    init_subsizes_();
                }
                // Compute offsets because iterators may be destroyed
                auto insert_begin_offset = std::distance<const_data_iterator>(m_plain_vector.cbegin(), position.plain_cbegin());
                if (insert_begin_offset < 0) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[insert]: position iterator doesn't belong to this vector!");
                }
                //auto insert_end_offset = insert_begin_offset + length * m_dimensions[1];
                auto insert_end_offset = insert_begin_offset + length * std::get<dim_descriptor::idx_total>(m_descriptors[1]);
                //Allocate new memory
                update_top_dimension_(get_top_dimension_() + length);
                m_plain_vector.resize(total_size());
                //Shift stored elements to the end
                std::move_backward(std::next(m_plain_vector.cbegin(), insert_begin_offset), std::next(m_plain_vector.cbegin(), old_size), m_plain_vector.end());
                //Return range for new elements
                return yato::make_range(std::next(m_plain_vector.begin(), insert_begin_offset), std::next(m_plain_vector.begin(), insert_end_offset));
            }

            iterator erase_impl_(const const_iterator & first, const const_iterator & last)
            {
                auto count = std::distance(first, last);
                if (count < 0) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[insert]: invalid iterators range!");
                }
                size_t erase_size = count * first.total_size();
                size_t erase_offset = std::distance(plain_cbegin(), first.plain_cbegin());
                std::move(std::next(plain_cbegin(), erase_offset + erase_size), plain_cend(), std::next(plain_begin(), erase_offset));
                resize(get_top_dimension_() - count);
                return create_proxy_(std::next(plain_begin(), erase_offset));
            }

            vector_nd_impl(container_type && plain_vector, const dimensions_type & sizes)
                : m_plain_vector(std::move(plain_vector))
            {
                init_sizes_(yato::make_range(sizes.cbegin(), sizes.cend()));
            }

            //-------------------------------------------------------

        public:
            /**
             *  Create empty vector
             */
            vector_nd_impl() = default;

            /**
             *	Create empty vector
             */
            explicit
            vector_nd_impl(const allocator_type & alloc) YATO_NOEXCEPT_KEYWORD
                :  m_plain_vector(alloc)
            { }

            /**
             *  Create without initialization
             */
            explicit
            vector_nd_impl(const dimensions_type & sizes, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                init_sizes_(sizes);
                m_plain_vector.resize(total_size());
            }

            /**
             *  Create with initialization
             */
            vector_nd_impl(const dimensions_type & sizes, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                assign(sizes, value);
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes 
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const InputIt & first, const InputIt & last, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                YATO_REQUIRES(sizes.total_size() == narrow_cast<size_t>(std::distance(first, last)));
                init_sizes_(sizes);
                m_plain_vector.resize(total_size());
                std::copy(first, last, plain_begin());
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const yato::range<InputIt> & range, const allocator_type & alloc = allocator_type())
                : vector_nd_impl(sizes, range.begin(), range.end(), alloc)
            { }

            /**
             *  Create from initializer list
             */
            vector_nd_impl(const initilizer_type<dimensions_number> & init_list)
                : m_plain_vector()
            {
                init_sizes_<dimensions_number>(init_list);
                if (get_top_dimension_() > 0) {
                    m_plain_vector.reserve(total_size());
                    auto iter = std::back_inserter(m_plain_vector);
                    init_values_<dimensions_number>(init_list, iter);
                }
            }

            /**
             *  Create with sizes from a generic range of sizes without initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                YATO_REQUIRES(range.distance() == dimensions_number); // "Constructor takes the amount of arguments equal to dimensions number"
                init_sizes_(range);
                m_plain_vector.resize(total_size());
            }

            /**
             *  Create with sizes from a generic range of sizes with initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                YATO_REQUIRES(range.distance() == dimensions_number); // "Constructor takes the amount of arguments equal to dimensions number"
                init_sizes_(range);
                m_plain_vector.assign(total_size(), value);
            }

            /**
             *  Copy constructor
             */
            vector_nd_impl(const my_type &) = default;

            /**
             * Move-copy constructor
             */
#ifndef YATO_MSVC_2013
            vector_nd_impl(my_type &&) = default;
#else
            vector_nd_impl(my_type && other)
                : m_dimensions(std::move(other.m_dimensions)), m_sub_sizes(std::move(other.m_sub_sizes)), m_plain_vector(std::move(other.m_plain_vector))
            { }
#endif
            /**
             *	Copy assign
             */
            my_type & operator= (const my_type & other)
            {
                YATO_REQUIRES(this != &other);
                my_type tmp(other);
                tmp.swap(*this);
                return *this;
            }

            /**
             *  Move assign
             */
            my_type & operator= (my_type && other) YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(this != &other);
                m_descriptors  = std::move(other.m_descriptors);
                m_plain_vector = std::move(other.m_plain_vector);
                return *this;
            }

            /**
             *  Copy from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            explicit
            vector_nd_impl(const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                init_sizes_(other.dimensions_range());
                m_plain_vector.resize(total_size());
                std::copy(other.plain_cbegin(), other.plain_cend(), plain_begin());
            }

            /**
             *  Assign from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            my_type & operator = (const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                my_type tmp(other);
                tmp.swap(*this);
                return *this;
            }

            /**
             *  Destructor
             */
            ~vector_nd_impl() = default;

            /**
             *  Replaces the contents of the container
             */
            void assign(const dimensions_type & sizes, const data_type & value)
            {
                init_sizes_(sizes);
                m_plain_vector.assign(total_size(), value);
            }

            /**
             * Create a new vector with another shape
             * All data will be copied to the new vector
             */
            template <size_t NewDimsNum, typename NewAllocatorType = allocator_type>
            vector_nd_impl<data_type, NewDimsNum, NewAllocatorType> reshape(const dimensionality<NewDimsNum, size_t> & extents, const NewAllocatorType & alloc = NewAllocatorType()) const
            {
                YATO_REQUIRES(extents.total_size() == total_size());
                return vector_nd_impl<data_type, NewDimsNum, NewAllocatorType>(extents, plain_crange(), alloc);
            }

#ifndef YATO_MSVC_2013
            /**
             * Create a new vector with another shape
             * All data will be moved to the new vector
             */
            template <size_t NewDimsNum>
            vector_nd_impl<data_type, NewDimsNum, allocator_type> reshape(const dimensionality<NewDimsNum, size_t> & extents) &&
            {
                YATO_REQUIRES(extents.total_size() == total_size());
                update_top_dimension_(0); // make "empty"
                return vector_nd_impl<data_type, NewDimsNum, allocator_type>(std::move(m_plain_vector), extents);
            }
#endif
            /**
             *  Returns the allocator associated with the container
             */
            allocator_type get_allocator() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.get_allocator();
            }

            /**
             *  Save swap
             */
            void swap(my_type & other) YATO_NOEXCEPT_KEYWORD
            {
                using std::swap;
                swap(m_descriptors, other.m_descriptors);
                swap(m_plain_vector, other.m_plain_vector);
            }
#ifdef YATO_MSVC
            /*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
            *	MSVC complains about type cast otherwise
            */
#pragma warning(push)
#pragma warning(disable:4702) 
#endif
            /**
             *  Element access without bounds check in release
             */
            const_proxy operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < size(0))
                    ? create_const_proxy_(idx)
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd: out of range!"), create_const_proxy_(0));
#else
                return create_const_proxy_(idx);
#endif
            }
            /**
             *  Element access without bounds check in release
             */
            proxy operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < size(0))
                    ? create_proxy_(idx)
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd: out of range!"), create_proxy_(0));
#else
                return create_proxy_(idx);
#endif
            }
#ifdef YATO_MSVC
#pragma warning(pop)
#endif
            /**
             *  Element access with bounds check
             */
            template<typename... _Tail>
            auto at(size_t idx, _Tail &&... tail) const
                -> typename std::enable_if<(yato::args_length<_Tail...>::value == dimensions_number - 1), const_reference>::type
            {
                if (idx >= size(0)) {
                    throw yato::out_of_range_error("yato::array_nd: out of range!");
                }
                return (*this)[idx].at(std::forward<_Tail>(tail)...);
            }

            /**
             *  Element access with bounds check
             */
            template<typename... _Tail>
            auto at(size_t idx, _Tail &&... tail)
                -> typename std::enable_if<(yato::args_length<_Tail...>::value == dimensions_number - 1), reference>::type
            {
                if (idx >= size(0)) {
                    throw yato::out_of_range_error("yato::array_nd: out of range!");
                }
                return (*this)[idx].at(std::forward<_Tail>(tail)...);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return create_const_proxy_(0);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return create_proxy_(0);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return create_const_proxy_(size(0));
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return create_proxy_(size(0));
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cbegin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.begin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cend();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_end() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.end();
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<const_data_iterator> crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(cbegin(), cend());
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<data_iterator> range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(begin(), end());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<const_data_iterator> plain_crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(plain_cbegin(), plain_cend());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<data_iterator> plain_range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(plain_begin(), plain_end());
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            data_type* data() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.data();
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            const data_type* data() const YATO_NOEXCEPT_KEYWORD
            {
                return const_cast<my_type*>(this)->data();
            }

            /**
             *  Checks whether the vector is empty
             */
            bool empty() const YATO_NOEXCEPT_KEYWORD
            {
                return (get_top_dimension_() == 0);
            }

            /**
             *  Get dimensions
             */
            dimensions_type dimensions() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_type(dimensions_range());
            }

            /**
             *  Get number of dimensions
             */
            size_t dimensions_num() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_number;
            }

            /**
             *  Get number of dimensions
             */
            auto dimensions_range() const
                -> decltype(yato::make_range(m_descriptors).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>()))
            {
                return make_range(m_descriptors).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>());
            }

#ifdef YATO_MSVC
            /*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
            *	MSVC complains about type cast otherwise
            */
#pragma warning(push)
#pragma warning(disable:4702) 
#endif
            /**
             *  Get size of specified dimension
             *  If the vector is empty ( empty() returns true ) then calling for size(idx) returns 0 for idx = 0; Return value for any idx > 0 is undefined
             */
            size_type size(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < dimensions_number)
                    ? std::get<dim_descriptor::idx_size>(m_descriptors[idx])
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[size]: dimension index is out of range"), 0);
#else
                return std::get<dim_descriptor::idx_size>(m_descriptors[idx]);
#endif
            }
#ifdef YATO_MSVC
#pragma warning(pop)
#endif
            /**
             *  Get the total size of the vector (number of all elements)
             */
            size_t total_size() const YATO_NOEXCEPT_KEYWORD
            {
                return std::get<dim_descriptor::idx_total>(m_descriptors[0]);
            }

            /**
             *  Returns the number of elements that the container has currently allocated space for
             */
            size_t capacity() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.capacity();
            }

            /**
             *  Increase the capacity of the container to a value that's greater or equal to new_capacity
             */
            void reserve(size_t new_capacity)
            {
                m_plain_vector.reserve(new_capacity);
            }

            /**
             *  Requests the removal of unused capacity
             */
            void shrink_to_fit()
            {
                m_plain_vector.shrink_to_fit();
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than cirrent sze then all stored data will be preserved
             *  @param length desired length in number of sub-vectors
             */
            void resize(size_t length)
            {
                update_top_dimension_(length);
                m_plain_vector.resize(total_size());
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than cirrent sze then all stored data will be preserved
             *  @param length desired length in number of sub-vectors
             *  @param value if length is bigger than current size new elements will be copy initialized from 'value'
             */
            void resize(size_t length, const data_type & value)
            {
                update_top_dimension_(length);
                m_plain_vector.resize(total_size(), value);
            }

            /**
             * Resize all vector's extents
             * All stored data will become invalid
             * @param extents desired size of the vector
             */
            void resize(const dimensions_type & extents)
            {
                init_sizes_(extents);
                m_plain_vector.resize(total_size());
            }

            /**
             * Resize all vector's extents
             * All stored data will become invalid
             * @param extents desired size of the vector
             * @param value if the new size is bigger than the current size new elements will be copy initialized from 'value'
             */
            void resize(const dimensions_type & extents, const data_type & value)
            {
                init_sizes_(extents);
                m_plain_vector.resize(total_size(), value);
            }

            /**
             *  Clear vector
             */
            void clear()
            {
                resize(0);
            }

            /**
             *  Get the first sub-vector proxy
             */
            const_proxy front() const
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
#endif
                return (*this)[0];
            }

            /**
             *  Get the first sub-vector proxy
             */
            proxy front()
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
#endif
                return (*this)[0];
            }

            /**
             *  Get the last sub-vector proxy
             */
            const_proxy back() const
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
#endif
                return (*this)[size(0) - 1];
            }

            /**
             *  Get the last sub-vector proxy
             */
            proxy back()
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
#endif
                return (*this)[size(0) - 1];
            }

            /**
             *  Add sub-vector element to the back
             */
            template<typename _OtherDataType, typename _OtherAllocator>
            void push_back(const vector_nd_impl<_OtherDataType, dimensions_number - 1, _OtherAllocator> & sub_vector)
            {
                auto insert_range = prepare_push_back_(sub_vector.dimensions_range());
                std::copy(sub_vector.plain_cbegin(), sub_vector.plain_cend(), insert_range.begin());
            }

            /**
             *  Add sub-vector element to the back
             */
            void push_back(vector_nd_impl<data_type, dimensions_number - 1, allocator_type> && sub_vector)
            {
                auto insert_range = prepare_push_back_(sub_vector.dimensions_range());
                std::move(sub_vector.plain_cbegin(), sub_vector.plain_cend(), insert_range.begin());
            }

            /**
             *  Removes the last element of the container.
             */
            void pop_back()
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[pop_back]: vector is already empty!");
                }
#endif
                update_top_dimension_(size(0) - 1);
                m_plain_vector.resize(total_size());
            }

            /**
             *  Insert sub-vector element
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename _OtherDataType, typename _OtherAllocator>
            iterator insert(const const_iterator & position, const vector_nd_impl<_OtherDataType, dimensions_number - 1, _OtherAllocator> & sub_vector)
            {
                auto insert_range = prepare_insert_(sub_vector.dimensions_range(), position, 1);
                std::copy(sub_vector.plain_cbegin(), sub_vector.plain_cend(), insert_range.begin());
                return create_proxy_(insert_range.begin());
            }

            /**
             *  Insert sub-vector element
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            iterator insert(const const_iterator & position, vector_nd_impl<data_type, dimensions_number - 1, allocator_type> && sub_vector)
            {
                auto insert_range = prepare_insert_(sub_vector.dimensions_range(), position, 1);
                std::move(sub_vector.plain_cbegin(), sub_vector.plain_cend(), insert_range.begin());
                return create_proxy_(insert_range.begin());
            }

            /**
             *  Insert count copies of sub-vector element
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename _OtherDataType, typename _OtherAllocator>
            iterator insert(const const_iterator & position, size_t count, const vector_nd_impl<_OtherDataType, dimensions_number - 1, _OtherAllocator> & sub_vector)
            {
                auto insert_range = prepare_insert_(sub_vector.dimensions_range(), position, count);
                const size_t copy_size = sub_vector.total_size();
                auto copy_dst = insert_range.begin();
                for (size_t i = 0; i < count; ++i, std::advance(copy_dst, copy_size)) {
                    std::copy(sub_vector.plain_cbegin(), sub_vector.plain_cend(), copy_dst);
                }
                return create_proxy_(insert_range.begin());
            }

            /**
             *  Inserts sub-vector elements from range [first, last) before 'position'
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename _OtherDataIterator, typename _SizeIterator>
            iterator insert(const const_iterator & position, const proxy_tmpl<_OtherDataIterator, _SizeIterator> & first, const proxy_tmpl<_OtherDataIterator, _SizeIterator> & last)
            {
                auto count = std::distance(first, last);
                if (count < 0) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[insert]: invalid iterators range!");
                }
                auto insert_range = prepare_insert_(first.dimensions_range(), position, count);
                std::copy(first.plain_cbegin(), last.plain_cbegin(), insert_range.begin());
                return create_proxy_(insert_range.begin());
            }

            /**
             *  Inserts sub-vector elements from range [first, last) before 'position'
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename _OtherDataIterator, typename _SizeIterator>
            iterator insert(const const_iterator & position, const yato::range< proxy_tmpl<_OtherDataIterator, _SizeIterator> > & range)
            {
                return insert(position, range.begin(), range.end());
            }

            /**
             *  Removes the sub-vector element at 'position'
             */
            iterator erase(const const_iterator & position)
            {
                return erase_impl_(position, std::next(position));
            }

            /**
             *  Removes the sub-vector elements in the range [first, last)
             */
            iterator erase(const const_iterator & first, const const_iterator & last)
            {
                return erase_impl_(first, last);
            }

            //------------------------------------------------------------

            template <typename, size_t, typename>
            friend class vector_nd_impl;
        };







        //-------------------------------------------------------
        // Implementation of 1D case
        // Delegates most methods to std::vector

        template <typename _DataType, typename _Allocator>
        class vector_nd_impl<_DataType, 1, _Allocator>
        {
        public:
            /*
            * Public traits of the multidimensional vector
            */
            using my_type = vector_nd_impl<_DataType, 1, _Allocator>;
            using dimensions_type = dimensionality<1, size_t>;
            using data_type = _DataType;
            using allocator_type = _Allocator;
            using container_type = std::vector<data_type, allocator_type>;
            using data_iterator = typename container_type::iterator;
            using const_data_iterator = typename container_type::const_iterator;
            using reference = decltype(*std::declval<data_iterator>());
            using const_reference = decltype(*std::declval<const_data_iterator>());

            static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;

            using iterator = data_iterator;
            using const_iterator = const_data_iterator;
            //-------------------------------------------------------

        private:
            container_type m_plain_vector;
            //-------------------------------------------------------

            vector_nd_impl(container_type && plain_vector, const dimensions_type &)
                : m_plain_vector(std::move(plain_vector))
            { }
            //-------------------------------------------------------

        public:
            /**
             *  Create empty vector
             */
            vector_nd_impl() YATO_NOEXCEPT_KEYWORD
                : m_plain_vector()
            {}

            /**
             *  Create empty vector
             */
            explicit
            vector_nd_impl(const allocator_type & alloc) YATO_NOEXCEPT_KEYWORD
                : m_plain_vector(alloc)
            {}

            /**
             *  Create without initialization
             */
            explicit
            vector_nd_impl(const dimensions_type & size, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                m_plain_vector.resize(size[0]);
            }

            /**
             *  Create with initialization
             */
            vector_nd_impl(const dimensions_type & size, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                assign(size, value);
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const InputIt & first, const InputIt & last, const allocator_type & alloc = allocator_type())
                : m_plain_vector(first, last, alloc)
            {
                YATO_MAYBE_UNUSED(sizes);
                YATO_REQUIRES(sizes.total_size() == narrow_cast<size_t>(std::distance(first, last)));
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const yato::range<InputIt> & range, const allocator_type & alloc = allocator_type())
                : vector_nd_impl(sizes, range.begin(), range.end(), alloc)
            { }

            /**
             *  Create from initializer list
             */
            vector_nd_impl(const std::initializer_list<data_type> & init_list)
                : m_plain_vector(init_list)
            { }

            /**
             *  Create with sizes from a generic range of sizes without initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (range.distance() != dimensions_number) {
                    throw yato::assertion_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                m_plain_vector.resize(*range.begin());
            }

            /**
             *  Create with sizes from a generic range of sizes with initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (range.distance() != dimensions_number) {
                    throw yato::assertion_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                m_plain_vector.resize(*range.begin(), value);
            }

            /**
             *  Create from std::vector
             */
            template<typename _VecDataType, typename _VecAllocator>
            vector_nd_impl(const std::vector<_VecDataType, _VecAllocator> & vector)
                : m_plain_vector(vector)
            { }

            /**
             *  Create from std::vector
             */
            vector_nd_impl(std::vector<data_type, allocator_type> && vector)
                : m_plain_vector(std::move(vector))
            { }

            /**
             *  Copy constructor
             */
            vector_nd_impl(const my_type & other)
                : m_plain_vector(other.m_plain_vector)
            {}

            /**
             * Move-copy constructor
             */
            vector_nd_impl(my_type && other)
                : m_plain_vector(std::move(other.m_plain_vector))
            {}

            /**
             *  Copy assign
             */
            my_type & operator= (const my_type & other)
            {
                if (this != &other) {
                    my_type tmp{ other };
                    tmp.swap(*this);
                }
                return *this;
            }

            /**
             *  Move assign
             */
            my_type & operator= (my_type && other) YATO_NOEXCEPT_KEYWORD
            {
                if (this != &other) {
                    m_plain_vector = std::move(other.m_plain_vector);
                }
                return *this;
            }

            /**
             *  Assign from std::vector
             */
            template<typename _VecDataType, typename _VecAllocator>
            my_type & operator= (const std::vector<_VecDataType, _VecAllocator> & vector)
            {
                m_plain_vector = vector;
                return *this;
            }

            /**
             *  Assign from std::vector
             */
            my_type & operator= (std::vector<data_type, allocator_type> && vector)
            {
                m_plain_vector = std::move(vector);
                return *this;
            }

            /**
             *  Copy from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            explicit
            vector_nd_impl(const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                m_plain_vector.resize(other.total_size());
                std::copy(other.plain_cbegin(), other.plain_cend(), plain_begin());
            }

            /**
             *  Assign from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            my_type & operator= (const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                my_type tmp{ other };
                tmp.swap(*this);
                return *this;
            }

            /**
             *  Destructor
             */
            ~vector_nd_impl()
            {}

#ifdef YATO_MSVC_2013
            /**
             *  Convert to std::vector
             */
            operator std::vector<data_type, allocator_type> & ()
            {
                return m_plain_vector;
            }
#else
            /**
             *  Convert to std::vector
             */
            operator std::vector<data_type, allocator_type> & () &
            {
                return m_plain_vector;
            }

            /**
             *  Convert to std::vector
             */
            operator std::vector<data_type, allocator_type> && () &&
            {
                return std::move(m_plain_vector);
            }
#endif
            /**
             *  Replaces the contents of the container
             */
            void assign(const dimensions_type & size, const data_type & value)
            {
                m_plain_vector.assign(size[0], value);
            }

            /**
             * Create a new vector with another shape
             * All data will be copied to the new vector
             */
            template <size_t NewDimsNum, typename NewAllocatorType = allocator_type>
            vector_nd_impl<data_type, NewDimsNum, NewAllocatorType> reshape(const dimensionality<NewDimsNum, size_t> & extents, const NewAllocatorType & alloc = NewAllocatorType()) const
            {
                YATO_REQUIRES(extents.total_size() == total_size());
                return vector_nd_impl<data_type, NewDimsNum, NewAllocatorType>(extents, plain_crange(), alloc);
            }

#ifndef YATO_MSVC_2013
            /**
             * Create a new vector with another shape
             * All data will be moved to the new vector
             */
            template <size_t NewDimsNum>
            vector_nd_impl<data_type, NewDimsNum, allocator_type> reshape(const dimensionality<NewDimsNum, size_t> & extents) &&
            {
                YATO_REQUIRES(extents.total_size() == total_size());
                return vector_nd_impl<data_type, NewDimsNum, allocator_type>(std::move(m_plain_vector), extents);
            }
#endif

            /**
             *  Returns the allocator associated with the container
             */
            allocator_type get_allocator() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.get_allocator();
            }

            /**
             *  Save swap
             */
            void swap(my_type & other) YATO_NOEXCEPT_KEYWORD
            {
                m_plain_vector.swap(other.m_plain_vector);
            }
#ifdef YATO_MSVC
            /*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
             *  MSVC complains about type cast otherwise
             */
#pragma warning(push)
#pragma warning(disable:4702) 
#endif
            /**
             *  Element access without bounds check in release
             */
            const_reference operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < m_plain_vector.size())
                    ? m_plain_vector[idx]
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd: out of range!"), m_plain_vector[0]);
#else
                return m_plain_vector[idx];
#endif
            }
            /**
             *  Element access without bounds check in release
             */
            reference operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < m_plain_vector.size())
                    ? m_plain_vector[idx]
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd: out of range!"), m_plain_vector[0]);
#else
                return m_plain_vector[idx];
#endif
            }
#ifdef YATO_MSVC
#pragma warning(pop)
#endif
            /**
             *  Element access with bounds check
             */
            const_reference at(size_t idx) const
            {
                return m_plain_vector.at(idx);
            }

            /**
             *  Element access with bounds check
             */
            reference at(size_t idx)
            {
                return m_plain_vector.at(idx);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return plain_cbegin();
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return plain_begin();
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return plain_cend();
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return plain_end();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cbegin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.begin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cend();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_end() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.end();
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<const_data_iterator> crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(cbegin(), cend());
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<data_iterator> range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(begin(), end());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<const_data_iterator> plain_crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(plain_cbegin(), plain_cend());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<data_iterator> plain_range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(plain_begin(), plain_end());
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            data_type* data() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.data();
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            const data_type* data() const YATO_NOEXCEPT_KEYWORD
            {
                return const_cast<my_type*>(this)->data();
            }

            /**
             *  Checks whether the vector is empty
             */
            bool empty() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.empty();
            }

            /**
             *  Get dimensions
             */
            dimensions_type dimensions() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::dims(m_plain_vector.size());
            }

            /**
             *  Get number of dimensions
             */
            size_t dimensions_num() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_number;
            }

            /**
             *  Get number of dimensions
             */
            auto dimensions_range() const
                -> yato::range<yato::numeric_iterator<size_t>>
            {
                return yato::make_range(m_plain_vector.size(), m_plain_vector.size() + 1);
            }

#ifdef YATO_MSVC
            /*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
             *  MSVC complains about type cast otherwise
             */
#pragma warning(push)
#pragma warning(disable:4702) 
#endif
            /**
             *  Get size of specified dimension
             */
            size_t size(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < dimensions_number)
                    ? m_plain_vector.size()
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[size]: dimension index is out of range"), 0);
#else
                YATO_MAYBE_UNUSED(idx);
                return m_plain_vector.size();
#endif
            }
#ifdef YATO_MSVC
#pragma warning(pop)
#endif
            /**
             *  Get the total size of the vector (number of all elements)
             */
            size_t total_size() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.size();
            }

            /**
             *  Returns the number of elements that the container has currently allocated space for
             */
            size_t capacity() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.capacity();
            }

            /**
             *  Increase the capacity of the container to a value that's greater or equal to new_capacity
             */
            void reserve(size_t new_capacity)
            {
                m_plain_vector.reserve(new_capacity);
            }

            /**
             *  Clear vector
             */
            void clear()
            {
                m_plain_vector.clear();
            }

            /**
             *  Requests the removal of unused capacity
             */
            void shrink_to_fit()
            {
                m_plain_vector.shrink_to_fit();
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than the current size, then all containing data will be preserved
             */
            void resize(size_t length)
            {
                m_plain_vector.resize(length);
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than the current size, then all containing data will be preserved
             */
            void resize(size_t length, const data_type & value)
            {
                m_plain_vector.resize(length, value);
            }

            /**
             *  Resize vector extents. 
             *  All stored data becomes invalid
             */
            void resize(const dimensions_type & extents)
            {
                m_plain_vector.resize(extents[0]);
            }

            /**
             *  Resize vector extents.
             *  All stored data becomes invalid
             */
            void resize(const dimensions_type & extents, const data_type & value)
            {
                m_plain_vector.resize(extents[0], value);
            }

            /**
             *  Get the first sub-vector proxy
             */
            const_reference front() const
            {
                if (empty()) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[front]: vector is empty");
                }
                return m_plain_vector.front();
            }

            /**
             *  Get the first sub-vector proxy
             */
            reference front()
            {
                if (empty()) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[front]: vector is empty");
                }
                return m_plain_vector.front();
            }

            /**
             *  Get the last sub-vector proxy
             */
            const_reference back() const
            {
                if (empty()) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[back]: vector is empty");
                }
                return m_plain_vector.back();
            }

            /**
             *  Get the last sub-vector proxy
             */
            reference back()
            {
                if (empty()) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[back]: vector is empty");
                }
                return m_plain_vector.back();
            }

            /**
             *  Add sub-vector element to the back
             */
            void push_back(const data_type & value)
            {
                m_plain_vector.push_back(value);
            }

            /**
             *  Add sub-vector element to the back
             */
            void push_back(data_type && value)
            {
                m_plain_vector.push_back(std::move(value));
            }

            /**
             *  Removes the last element of the container.
             */
            void pop_back()
            {
                if (empty()) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[pop_back]: vector is already empty!");
                }
                m_plain_vector.pop_back();
            }

            /**
             *  Inserts elements at the specified location before 'position'
             */
            iterator insert(const const_iterator & position, const data_type & value)
            {
                m_plain_vector.insert(position, value);
            }

            /**
             *  Inserts elements at the specified location before 'position'
             */
            iterator insert(const const_iterator & position, data_type && value)
            {
                m_plain_vector.insert(position, std::move(value));
            }

            /**
             *  Inserts elements at the specified location before 'position'
             */
            iterator insert(const const_iterator & position, size_t count, const data_type & value)
            {
                m_plain_vector.insert(position, count, value);
            }

            /**
             *  Inserts elements from range [first, last) before 'position'
             */
            template<class _InputIt>
            iterator insert(const const_iterator & position, _InputIt && first, _InputIt && last)
            {
                m_plain_vector.insert(position, std::forward<_InputIt>(first), std::forward<_InputIt>(last));
            }

            /**
             *  Inserts elements from range before 'position'
             */
            template<class _InputIt>
            iterator insert(const const_iterator & position, const yato::range<_InputIt> & range)
            {
                m_plain_vector.insert(position, range.begin(), range.end());
            }

            /**
             *  Removes the element at 'position'
             */
            iterator erase(const const_iterator & position)
            {
                m_plain_vector.erase(position);
            }

            /**
             *  Removes the elements in the range [first; last)
             */
            iterator erase(const const_iterator & first, const const_iterator & last)
            {
                m_plain_vector.erase(first, last);
            }

            /**
             *  Removes the elements in the range 
             */
            template<class _InputIt>
            iterator erase(const yato::range<_InputIt> & range)
            {
                m_plain_vector.erase(range.begin(), range.last());
            }

            //------------------------------------------------------------

            template <typename, size_t, typename>
            friend class vector_nd_impl;
        };

    }

    template <typename _DataType, size_t _DimensionsNum, typename _Allocator = std::allocator<_DataType> >
    using vector_nd = details::vector_nd_impl<_DataType, _DimensionsNum, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector_1d = vector_nd<_DataType, 1, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector = vector_1d<_DataType, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector_2d = vector_nd<_DataType, 2, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector_3d = vector_nd<_DataType, 3, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector_4d = vector_nd<_DataType, 4, _Allocator>;
}

#endif
