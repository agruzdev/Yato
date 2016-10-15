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

            static YATO_CONSTEXPR_VAR size_t dimensions_number = _DimensionsNum;
            static_assert(dimensions_number > 1, "Implementation for dimensions number larger than 1");
            //-------------------------------------------------------

        private:
            using size_iterator = typename dimensions_type::iterator;
            using size_const_iterator = typename dimensions_type::const_iterator;

            template<size_t _Dims>
            using initilizer_type = typename initilizer_list_nd<data_type, _Dims>::type;

            template<typename _SomeDataIter, typename _SomeSizeIter>
            using proxy_tmpl = details::sub_array_proxy<_SomeDataIter, _SomeSizeIter, dimensions_number - 1>;

            using proxy = proxy_tmpl<data_iterator, typename dimensions_type::const_iterator>;
            using const_proxy = proxy_tmpl<const_data_iterator, typename dimensions_type::const_iterator>;

        public:
            using iterator = proxy;
            using const_iterator = const_proxy;

            //-------------------------------------------------------

        private:
            dimensions_type m_dimensions;
            dimensions_type m_sub_sizes;
            container_type  m_plain_vector;
            //-------------------------------------------------------

            proxy _create_proxy(size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                return proxy(std::next(m_plain_vector.begin(), offset * m_sub_sizes[1]), std::next(m_dimensions.cbegin()), std::next(m_sub_sizes.cbegin()));
            }

            proxy _create_proxy(data_iterator plain_position) YATO_NOEXCEPT_KEYWORD
            {
                return proxy(plain_position, std::next(m_dimensions.cbegin()), std::next(m_sub_sizes.cbegin()));
            }

            YATO_CONSTEXPR_FUNC
            const_proxy _create_const_proxy(size_t offset) const YATO_NOEXCEPT_KEYWORD
            {
                return const_proxy(std::next(m_plain_vector.cbegin(), offset * m_sub_sizes[1]), std::next(m_dimensions.cbegin()), std::next(m_sub_sizes.cbegin()));
            }

            YATO_CONSTEXPR_FUNC
            const_proxy _create_const_proxy(const_data_iterator plain_position) const YATO_NOEXCEPT_KEYWORD
            {
                return const_proxy(plain_position, std::next(m_dimensions.cbegin()), std::next(m_sub_sizes.cbegin()));
            }

            void _init_subsizes() YATO_NOEXCEPT_KEYWORD
            {
                m_sub_sizes[dimensions_number - 1] = m_dimensions[dimensions_number - 1];
                for (size_t i = dimensions_number - 1; i > 0; --i) {
                    m_sub_sizes[i - 1] = m_dimensions[i - 1] * m_sub_sizes[i];
                }
            }

            template<typename _RangeType>
            void _init_sizes(_RangeType range) YATO_NOEXCEPT_IN_RELEASE
            {
                YATO_REQUIRES(range.distance() == dimensions_number);
                std::copy(range.begin(), range.end(), m_dimensions.begin());
                _init_subsizes();
            }

            template<size_t _Depth>
            auto _init_sizes(const initilizer_type<_Depth> & init_list) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if < (_Depth > 1), void > ::type
            {
                m_dimensions[dimensions_number - _Depth] = init_list.size();
                _init_sizes<_Depth - 1>(*(init_list.begin()));
            }

            template<size_t _Depth>
            auto _init_sizes(const initilizer_type<_Depth> & init_list) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_Depth == 1), void>::type
            {
                m_dimensions[dimensions_number - _Depth] = init_list.size();
                _init_subsizes();
            }

            template<size_t _Depth, typename _Iter>
            auto _init_values(const initilizer_type<_Depth> & init_list, _Iter & iter) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if < (_Depth > 1), void > ::type
            {
                for (const auto & init_sub_list : init_list) {
                    _init_values<_Depth - 1>(init_sub_list, iter);
                }
            }

            template<size_t _Depth, typename _Iter>
            auto _init_values(const initilizer_type<_Depth> & init_list, _Iter & iter) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_Depth == 1), void>::type
            {
                for (const auto & value : init_list) {
                    *iter++ = value;
                }
            }

            void _update_top_dimension(size_t new_size) YATO_NOEXCEPT_KEYWORD
            {
                m_dimensions[0] = new_size;
                m_sub_sizes[0] = m_sub_sizes[1] * new_size;
            }

            template<typename _SizeIterator>
            yato::range<data_iterator> _prepare_push_back(const yato::range<_SizeIterator> & sub_dims)
            {
                const size_t old_size = m_plain_vector.size();
                if (old_size > 0) {
                    if (!std::equal(std::next(m_dimensions.cbegin()), m_dimensions.cend(), sub_dims.begin())) {
                        YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[push_back]: Cannot push subvector with a different shape");
                    }
                }
                else {
                    std::copy(sub_dims.begin(), sub_dims.end(), std::next(m_dimensions.begin()));
                    _init_subsizes();
                }
                _update_top_dimension(m_dimensions[0] + 1);
                m_plain_vector.resize(m_sub_sizes[0]);
                return yato::make_range(std::next(m_plain_vector.begin(), old_size), m_plain_vector.end());
            }

            template<typename _SizeIterator, typename _Iterator>
            yato::range<data_iterator> _prepare_insert(const yato::range<_SizeIterator> & sub_dims, const _Iterator & position, size_t length)
            {
                const size_t old_size = m_plain_vector.size();
                if (old_size > 0) {
                    if (!std::equal(std::next(m_dimensions.cbegin()), m_dimensions.cend(), sub_dims.begin())) {
                        YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[insert]: Cannot insert sub-vector with a different shape");
                    }
                }
                else {
                    std::copy(sub_dims.begin(), sub_dims.end(), std::next(m_dimensions.begin()));
                    _init_subsizes();
                }
                //Compute offsets since iterators may be destroyed
                auto insert_begin_offset = std::distance<const_data_iterator>(m_plain_vector.cbegin(), position.plain_cbegin());
                if (insert_begin_offset < 0) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[insert]: position iterator doesn't belong to this vector!");
                }
                auto insert_end_offset = insert_begin_offset + length * m_dimensions[1];                
                //Allocate new memory
                _update_top_dimension(m_dimensions[0] + length);
                m_plain_vector.resize(m_sub_sizes[0]);
                //Shift stored elements to the end
                std::copy_backward(std::next(m_plain_vector.cbegin(), insert_begin_offset), std::next(m_plain_vector.cbegin(), old_size), m_plain_vector.end());
                //Return range for new elements
                return yato::make_range(std::next(m_plain_vector.begin(), insert_begin_offset), std::next(m_plain_vector.begin(), insert_end_offset));
            }

            iterator _erase_impl(const const_iterator & first, const const_iterator & last)
            {
                auto count = std::distance(first, last);
                if (count < 0) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[insert]: invalid iterators range!");
                }
                size_t erase_size = count * first.total_size();
                size_t erase_offset = std::distance(plain_cbegin(), first.plain_cbegin());
                std::move(std::next(plain_cbegin(), erase_offset + erase_size), plain_cend(), std::next(plain_begin(), erase_offset));
                resize(m_dimensions[0] - count);
                return _create_proxy(std::next(plain_begin(), erase_offset));
            }

            vector_nd_impl(container_type && plain_vector, const dimensions_type & sizes)
                : m_plain_vector(std::move(plain_vector))
            {
                _init_sizes(yato::make_range(sizes.cbegin(), sizes.cend()));
            }

            //-------------------------------------------------------

        public:
            /**
             *	Create empty vector
             */
            YATO_CONSTEXPR_FUNC
            vector_nd_impl() YATO_NOEXCEPT_KEYWORD
                : m_dimensions(), m_sub_sizes(), m_plain_vector()
            { }

            /**
             *	Create empty vector
             */
            explicit
            vector_nd_impl(const allocator_type & alloc) YATO_NOEXCEPT_KEYWORD
                : m_dimensions(), m_sub_sizes(), m_plain_vector(alloc)
            { }

            /**
             *  Create without initialization
             */
            explicit
            vector_nd_impl(const dimensions_type & sizes, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                _init_sizes(yato::make_range(sizes.cbegin(), sizes.cend()));
                m_plain_vector.resize(m_sub_sizes[0]);
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
                _init_sizes(yato::make_range(sizes.cbegin(), sizes.cend()));
                m_plain_vector.resize(m_sub_sizes[0]);
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
                _init_sizes<dimensions_number>(init_list);
                if (m_sub_sizes[0] > 0) {
                    m_plain_vector.reserve(m_sub_sizes[0]);
                    auto iter = std::back_inserter(m_plain_vector);
                    _init_values<dimensions_number>(init_list, iter);
                }
            }

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
                _init_sizes(range);
                m_plain_vector.resize(m_sub_sizes[0]);
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
                _init_sizes(range);
                m_plain_vector.resize(m_sub_sizes[0], value);
            }

            /**
             *	Copy constructor
             */
            vector_nd_impl(const my_type & other)
                : m_dimensions(other.m_dimensions), m_sub_sizes(other.m_sub_sizes), m_plain_vector(other.m_plain_vector)
            { }

            /**
             * Move-copy constructor
             */
            vector_nd_impl(my_type && other)
                : m_dimensions(std::move(other.m_dimensions)), m_sub_sizes(std::move(other.m_sub_sizes)), m_plain_vector(std::move(other.m_plain_vector))
            { }

            /**
             *	Copy assign
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
                    m_dimensions = std::move(other.m_dimensions);
                    m_sub_sizes = std::move(other.m_sub_sizes);
                    m_plain_vector = std::move(other.m_plain_vector);
                }
                return *this;
            }

            /**
             *  Copy from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            explicit
            vector_nd_impl(const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                _init_sizes(other.dimensions_range());
                m_plain_vector.resize(m_sub_sizes[0]);
                std::copy(other.plain_cbegin(), other.plain_cend(), plain_begin());
            }

            /**
             *  Assign from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            my_type & operator = (const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                my_type tmp{ other };
                tmp.swap(*this);
                return *this;
            }

            /**
             *  Destructor
             */
            ~vector_nd_impl()
            {
            }

            /**
             *  Replaces the contents of the container
             */
            void assign(const dimensions_type & sizes, const data_type & value)
            {
                _init_sizes(yato::make_range(sizes.cbegin(), sizes.cend()));
                m_plain_vector.assign(m_sub_sizes[0], value);
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
                _update_top_dimension(0); // make "empty"
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
             *	Save swap
             */
            void swap(my_type & other) YATO_NOEXCEPT_KEYWORD
            {
                if (this != &other) {
                    using std::swap;
                    swap(m_dimensions, other.m_dimensions);
                    swap(m_sub_sizes, other.m_sub_sizes);
                    swap(m_plain_vector, other.m_plain_vector);
                }
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
            YATO_CONSTEXPR_FUNC
            const_proxy operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < m_dimensions[0])
                    ? _create_const_proxy(idx)
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd: out of range!"), _create_const_proxy(0));
#else
                return _create_const_proxy(idx);
#endif
            }
            /**
             *  Element access without bounds check in release
             */
            proxy operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < m_dimensions[0])
                    ? _create_proxy(idx)
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd: out of range!"), _create_proxy(0));
#else
                return _create_proxy(idx);
#endif
            }
#ifdef YATO_MSVC
#pragma warning(pop)
#endif
            /**
             *  Element access with bounds check
             */
            template<typename... _Tail>
            YATO_CONSTEXPR_FUNC
                auto at(size_t idx, _Tail &&... tail) const
                -> typename std::enable_if<(yato::args_length<_Tail...>::value == dimensions_number - 1), const_reference>::type
            {
                if (idx >= m_dimensions[0]) {
                    throw yato::assertion_error("yato::array_nd: out of range!");
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
                if (idx >= m_dimensions[0]) {
                    throw yato::assertion_error("yato::array_nd: out of range!");
                }
                return (*this)[idx].at(std::forward<_Tail>(tail)...);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            YATO_CONSTEXPR_FUNC
            const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return _create_const_proxy(0);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return _create_proxy(0);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            YATO_CONSTEXPR_FUNC
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return _create_const_proxy(size(0));
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return _create_proxy(size(0));
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
            bool empty() const YATO_NOEXCEPT_KEYWORD
            {
                return (m_sub_sizes[0] == 0);
            }

            /**
             *  Get dimensions
             */
            YATO_CONSTEXPR_FUNC
            const dimensions_type & dimensions() const YATO_NOEXCEPT_KEYWORD
            {
                return m_dimensions;
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
             *  Get number of dimensions
             */
            auto dimensions_range() const
                -> yato::range<size_const_iterator>
            {
                return yato::range<size_const_iterator>(m_dimensions.cbegin(), m_dimensions.cend());
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
            YATO_CONSTEXPR_FUNC
            size_t size(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < dimensions_number)
                    ? m_dimensions[idx]
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[size]: dimension index is out of range"), 0);
#else
                return m_dimensions[idx];
#endif
            }
#ifdef YATO_MSVC
#pragma warning(pop)
#endif
            /**
             *  Get the total size of the vector (number of all elements)
             */
            YATO_CONSTEXPR_FUNC
            size_t total_size() const YATO_NOEXCEPT_KEYWORD
            {
                return m_sub_sizes[0];
            }

            /**
             *  Returns the number of elements that the container has currently allocated space for
             */
            YATO_CONSTEXPR_FUNC
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
                _update_top_dimension(length);
                m_plain_vector.resize(m_sub_sizes[0]);
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than cirrent sze then all stored data will be preserved
             *  @param length desired length in number of sub-vectors
             *  @param value if length is bigger than current size new elements will be copy initialized from 'value'
             */
            void resize(size_t length, const data_type & value)
            {
                _update_top_dimension(length);
                m_plain_vector.resize(m_sub_sizes[0], value);
            }

            /**
             * Resize all vector's extents
             * All stored data will become invalid
             * @param extents desired size of the vector
             */
            void resize(const dimensions_type & extents)
            {
                _init_sizes(yato::make_range(extents.cbegin(), extents.cend()));
                m_plain_vector.resize(m_sub_sizes[0]);
            }

            /**
             * Resize all vector's extents
             * All stored data will become invalid
             * @param extents desired size of the vector
             * @param value if the new size is bigger than the current size new elements will be copy initialized from 'value'
             */
            void resize(const dimensions_type & extents, const data_type & value)
            {
                _init_sizes(yato::make_range(extents.cbegin(), extents.cend()));
                m_plain_vector.resize(m_sub_sizes[0], value);
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
            YATO_CONSTEXPR_FUNC
            const_proxy front() const
            {
                if (m_dimensions[0] == 0) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[front]: vector is empty");
                }
                return (*this)[0];
            }

            /**
             *  Get the first sub-vector proxy
             */
            proxy front()
            {
                if (m_dimensions[0] == 0) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[front]: vector is empty");
                }
                return (*this)[0];
            }

            /**
             *  Get the last sub-vector proxy
             */
            YATO_CONSTEXPR_FUNC
            const_proxy back() const
            {
                if (m_dimensions[0] == 0) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[back]: vector is empty");
                }
                return (*this)[m_dimensions[0] - 1];
            }

            /**
             *  Get the last sub-vector proxy
             */
            proxy back()
            {
                if (m_dimensions[0] == 0) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[back]: vector is empty");
                }
                return (*this)[m_dimensions[0] - 1];
            }

            /**
             *  Add sub-vector element to the back
             */
            template<typename _OtherDataType, typename _OtherAllocator>
            void push_back(const vector_nd_impl<_OtherDataType, dimensions_number - 1, _OtherAllocator> & sub_vector)
            {
                auto insert_range = _prepare_push_back(sub_vector.dimensions_range());
                std::copy(sub_vector.plain_cbegin(), sub_vector.plain_cend(), insert_range.begin());
            }

            /**
             *  Add sub-vector element to the back
             */
            void push_back(vector_nd_impl<data_type, dimensions_number - 1, allocator_type> && sub_vector)
            {
                auto insert_range = _prepare_push_back(sub_vector.dimensions_range());
                std::move(sub_vector.plain_cbegin(), sub_vector.plain_cend(), insert_range.begin());
            }

            /**
             *  Removes the last element of the container.
             */
            void pop_back()
            {
                if (empty()) {
                    YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[pop_back]: vector is already empty!");
                }
                _update_top_dimension(m_dimensions[0] - 1);
                m_plain_vector.resize(m_sub_sizes[0]);
            }

            /**
             *  Insert sub-vector element
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename _OtherDataType, typename _OtherAllocator>
            iterator insert(const const_iterator & position, const vector_nd_impl<_OtherDataType, dimensions_number - 1, _OtherAllocator> & sub_vector)
            {
                auto insert_range = _prepare_insert(sub_vector.dimensions_range(), position, 1);
                std::copy(sub_vector.plain_cbegin(), sub_vector.plain_cend(), insert_range.begin());
                return _create_proxy(insert_range.begin());
            }

            /**
             *  Insert sub-vector element
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            iterator insert(const const_iterator & position, vector_nd_impl<data_type, dimensions_number - 1, allocator_type> && sub_vector)
            {
                auto insert_range = _prepare_insert(sub_vector.dimensions_range(), position, 1);
                std::move(sub_vector.plain_cbegin(), sub_vector.plain_cend(), insert_range.begin());
                return _create_proxy(insert_range.begin());
            }

            /**
             *  Insert count copies of sub-vector element
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename _OtherDataType, typename _OtherAllocator>
            iterator insert(const const_iterator & position, size_t count, const vector_nd_impl<_OtherDataType, dimensions_number - 1, _OtherAllocator> & sub_vector)
            {
                auto insert_range = _prepare_insert(sub_vector.dimensions_range(), position, count);
                const size_t copy_size = sub_vector.total_size();
                auto copy_dst = insert_range.begin();
                for (size_t i = 0; i < count; ++i, std::advance(copy_dst, copy_size)) {
                    std::copy(sub_vector.plain_cbegin(), sub_vector.plain_cend(), copy_dst);
                }
                return _create_proxy(insert_range.begin());
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
                auto insert_range = _prepare_insert(first.dimensions_range(), position, count);
                std::copy(first.plain_cbegin(), last.plain_cbegin(), insert_range.begin());
                return _create_proxy(insert_range.begin());
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
                return _erase_impl(position, std::next(position));
            }

            /**
             *  Removes the sub-vector elements in the range [first, last)
             */
            iterator erase(const const_iterator & first, const const_iterator & last)
            {
                return _erase_impl(first, last);
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
            bool empty() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.empty();
            }

            /**
             *  Get dimensions
             */
            YATO_CONSTEXPR_FUNC
            dimensions_type dimensions() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::dims(m_plain_vector.size());
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
            size_t total_size() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.size();
            }

            /**
             *  Returns the number of elements that the container has currently allocated space for
             */
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
            YATO_CONSTEXPR_FUNC
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
