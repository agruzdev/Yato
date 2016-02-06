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
#include "type_traits.h"
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
            using data_type = _DataType;
            using allocator_type = _Allocator;
            using container_type = std::vector<data_type, allocator_type>;
            using iterator = typename container_type::iterator;
            using const_iterator = typename container_type::const_iterator;
            using reference = decltype(*std::declval<iterator>());
            using const_reference = decltype(*std::declval<const_iterator>());

            static YATO_CONSTEXPR_VAR size_t dimensions_num = _DimensionsNum;
            static_assert(dimensions_num > 1, "Implementation for dimensions number larger than 1");
            //-------------------------------------------------------

        private:
            using sizes_array = std::array<size_t, dimensions_num>;
            using size_iterator = typename sizes_array::iterator;
            using size_const_iterator = typename sizes_array::const_iterator;

            template<size_t _Dims>
            using initilizer_type = typename initilizer_list_nd<data_type, _Dims>::type;

            using proxy = details::sub_array_proxy<iterator, typename sizes_array::const_iterator, dimensions_num - 1>;
            using const_proxy = details::sub_array_proxy<const_iterator, typename sizes_array::const_iterator, dimensions_num - 1>;

            sizes_array m_dimensions;
            sizes_array m_sub_sizes;
            container_type m_plain_vector;
            //-------------------------------------------------------

            void _init_subsizes() YATO_NOEXCEPT_KEYWORD
            {
                m_sub_sizes[dimensions_num - 1] = m_dimensions[dimensions_num - 1];
                for (size_t i = dimensions_num - 1; i > 0; --i) {
                    m_sub_sizes[i - 1] = m_dimensions[i - 1] * m_sub_sizes[i];
                }
            }

            template<typename _RangeType>
            void _init_sizes(_RangeType range) YATO_NOEXCEPT_IN_RELEASE
            {
                YATO_REQUIRES(range.distance() == dimensions_num);
                std::copy(range.begin(), range.end(), m_dimensions.begin());
                _init_subsizes();
            }

            template<size_t _Depth>
            auto _init_sizes(const initilizer_type<_Depth> & init_list) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_Depth > 1), void>::type
            {
                m_dimensions[dimensions_num - _Depth] = init_list.size();
                _init_sizes<_Depth-1>(*(init_list.begin()));
            }

            template<size_t _Depth>
            auto _init_sizes(const initilizer_type<_Depth> & init_list) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_Depth == 1), void>::type
            {
                m_dimensions[dimensions_num - _Depth] = init_list.size();
               _init_subsizes();
            }

            template<size_t _Depth, typename _Iter>
            auto _init_values(const initilizer_type<_Depth> & init_list, _Iter & iter) YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_Depth > 1), void>::type
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
            yato::range<iterator> _prepare_push_back(const yato::range<_SizeIterator> & sub_dims)
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

            //-------------------------------------------------------

        public:
            /**
             *	Create empty vector
             */
            YATO_CONSTEXPR_FUNC
            vector_nd_impl() YATO_NOEXCEPT_KEYWORD
                : m_dimensions({ 0 }), m_sub_sizes({ 0 }), m_plain_vector()
            { }

            /**
             *	Create empty vector
             */
            explicit 
            vector_nd_impl(const allocator_type & alloc) YATO_NOEXCEPT_KEYWORD
                : m_dimensions({ 0 }), m_sub_sizes({ 0 }), m_plain_vector(alloc)
            { }

            /**
             *	Create without initialization
             */
            vector_nd_impl(const std::initializer_list<size_t> & sizes, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (sizes.size() != dimensions_num) {
                    throw yato::assertion_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                _init_sizes(yato::make_range(sizes.begin(), sizes.end()));
                m_plain_vector.resize(m_sub_sizes[0]);
            }

            /**
             *	Create with initialization
             */
            vector_nd_impl(const std::initializer_list<size_t> & sizes, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                assign(sizes, value);
            }

            /**
             *	Create from initializer list
             */
            vector_nd_impl(const initilizer_type<dimensions_num> & init_list)
                : m_plain_vector()
            {
                _init_sizes<dimensions_num>(init_list);
                if (m_sub_sizes[0] > 0) {
                    m_plain_vector.reserve(m_sub_sizes[0]);
                    auto iter = std::back_inserter(m_plain_vector);
                    _init_values<dimensions_num>(init_list, iter);
                }
            }

            /**
             *	Create with sizes from a generic range without initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (range.distance() != dimensions_num) {
                    throw yato::assertion_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                _init_sizes(range);
                m_plain_vector.resize(m_sub_sizes[0]);
            }

            /**
            *	Create with sizes from a generic range without initialization
            */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (range.distance() != dimensions_num) {
                    throw yato::assertion_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                _init_sizes(range);
                m_plain_vector.resize(m_sub_sizes[0], value);
            }

            /**
             *	Copy constructor
             */
            vector_nd_impl(const my_type & other) 
                : m_dimensions(other.m_dimensions),
                  m_sub_sizes(other.m_sub_sizes),
                  m_plain_vector(other.m_plain_vector)
            { }

            /**
             * Move-copy constructor
             */
            vector_nd_impl(my_type && other)
                : m_dimensions(std::move(other.m_dimensions)),
                  m_sub_sizes(std::move(other.m_sub_sizes)),
                  m_plain_vector(std::move(other.m_plain_vector))
            { }

            /**
             *	Copy assign
             */
            my_type & operator= (const my_type & other)
            {
                if (this != &other) {
                    //try to reserve memory to get out of memory exception before changing anything
                    m_plain_vector.reserve(other.m_plain_vector.size());
                    //copy data
                    m_dimensions = other.m_dimensions;
                    m_sub_sizes = other.m_sub_sizes;
                    m_plain_vector = other.m_plain_vector;
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
            vector_nd_impl(const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_num> & other)
            {
                _init_sizes(other.dimensions_range());
                m_plain_vector.resize(m_sub_sizes[0]);
                std::copy(other.cbegin(), other.cend(), begin());
            }

            /**
             *  Assign from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            my_type & operator= (const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_num> & other)
            {
                _init_sizes(other.dimensions_range());
                m_plain_vector.resize(m_sub_sizes[0]);
                std::copy(other.cbegin(), other.cend(), begin());
                return *this;
            }

            /**
             *  Destructor
             */
            ~vector_nd_impl()
            { }

            /**
             *  Replaces the contents of the container
             */
            void assign(const std::initializer_list<size_t> & sizes, const data_type & value)
            {
                if (sizes.size() != dimensions_num) {
                    throw yato::assertion_error("Assign takes the amount of arguments equal to dimensions number");
                }
                _init_sizes(yato::make_range(sizes.begin(), sizes.end()));
                m_plain_vector.resize(m_sub_sizes[0], value);
            }

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
                using std::swap;
                swap(m_dimensions, other.m_dimensions);
                swap(m_sub_sizes, other.m_sub_sizes);
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
            YATO_CONSTEXPR_FUNC
            const_proxy operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < m_dimensions[0]) 
                    ? const_proxy{ std::next(m_plain_vector.cbegin(), idx * m_sub_sizes[1]), std::next(std::begin(m_dimensions)), std::next(std::begin(m_sub_sizes)) }
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd: out of range!"), const_proxy{ m_plain_vector.cbegin(), std::begin(m_dimensions), std::begin(m_sub_sizes) });
#else
                return const_proxy{ std::next(m_plain_vector.cbegin(), idx * m_sub_sizes[1]), std::next(std::begin(m_dimensions)), std::next(std::begin(m_sub_sizes)) };
#endif
            }
            /**
             *  Element access without bounds check in release
             */
            proxy operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < m_dimensions[0])
                    ? proxy{ std::next(m_plain_vector.begin(), idx * m_sub_sizes[1]), std::next(std::begin(m_dimensions)), std::next(std::begin(m_sub_sizes)) }
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd: out of range!"), proxy{ m_plain_vector.begin(), std::begin(m_dimensions), std::begin(m_sub_sizes) });
#else
                return proxy{ std::next(m_plain_vector.begin(), idx * m_sub_sizes[1]), std::next(std::begin(m_dimensions)), std::next(std::begin(m_sub_sizes)) };
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
                -> typename std::enable_if<(yato::length<_Tail...>::value == dimensions_num - 1), const_reference>::type
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
                -> typename std::enable_if<(yato::length<_Tail...>::value == dimensions_num - 1), reference>::type
            {
                if (idx >= m_dimensions[0]) {
                    throw yato::assertion_error("yato::array_nd: out of range!");
                }
                return (*this)[idx].at(std::forward<_Tail>(tail)...);
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            YATO_CONSTEXPR_FUNC
            const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cbegin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.begin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            YATO_CONSTEXPR_FUNC
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cend();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.end();
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            YATO_CONSTEXPR_FUNC
            yato::range<const_iterator> crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(cbegin(), cend());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<iterator> range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(begin(), end());
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
             *  Get number of dimensions
             */
            YATO_CONSTEXPR_FUNC
            size_t dimensions() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_num;
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
             */
            YATO_CONSTEXPR_FUNC
            size_t dim_size(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < dimensions_num)
                    ? m_dimensions[idx]
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[dim_size]: dimension index is out of range"), 0);
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
            size_t size() const YATO_NOEXCEPT_KEYWORD
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
             *  @param length desired length in number of sub-vectors
             */
            void resize(size_t length)
            {
                _update_top_dimension(length);
                m_plain_vector.resize(m_sub_sizes[0]);
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
            void push_back(const vector_nd_impl<_OtherDataType, dimensions_num - 1, _OtherAllocator> & sub_vector)
            {
                auto insert_range = _prepare_push_back(sub_vector.dimensions_range());
                std::copy(sub_vector.cbegin(), sub_vector.cend(), insert_range.begin());
            }

            /**
             *  Add sub-vector element to the back
             */
            template<typename _OtherDataType, typename _OtherAllocator>
            void push_back(vector_nd_impl<_OtherDataType, dimensions_num - 1, _OtherAllocator> && sub_vector)
            {
                auto insert_range = _prepare_push_back(sub_vector.dimensions_range());
                std::move(sub_vector.cbegin(), sub_vector.cend(), insert_range.begin());
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
            using data_type = _DataType;
            using allocator_type = _Allocator;
            using container_type = std::vector<data_type, allocator_type>;
            using iterator = typename container_type::iterator;
            using const_iterator = typename container_type::const_iterator;
            using reference = decltype(*std::declval<iterator>());
            using const_reference = decltype(*std::declval<const_iterator>());

            static YATO_CONSTEXPR_VAR size_t dimensions_num = 1;
            //-------------------------------------------------------

        private:
            container_type m_plain_vector;
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
            vector_nd_impl(size_t size, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                m_plain_vector.resize(size);
            }

            /**
             *  Create with initialization
             */
            vector_nd_impl(size_t size, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                assign(size, value);
            }

            /**
             *  Create from initializer list
             */
            vector_nd_impl(const std::initializer_list<data_type> & init_list)
                : m_plain_vector(init_list)
            { }

            /**
             *  Create with sizes from a generic range without initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (range.distance() != dimensions_num) {
                    throw yato::assertion_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                m_plain_vector.resize(*range.begin());
            }

            /**
             *  Create with sizes from a generic range without initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (range.distance() != dimensions_num) {
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
                    m_plain_vector = other.m_plain_vector;
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
            vector_nd_impl(const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_num> & other)
            {
                m_plain_vector.resize(other.size());
                std::copy(other.cbegin(), other.cend(), begin());
            }

            /**
             *  Assign from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            my_type & operator= (const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_num> & other)
            {
                m_plain_vector.resize(other.size());
                std::copy(other.cbegin(), other.cend(), begin());
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
            void assign(size_t size, const data_type & value)
            {
                m_plain_vector.resize(size, value);
            }

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
             *  Iterator for accessing elements trough all dimensions
             */
            YATO_CONSTEXPR_FUNC
            const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cbegin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.begin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            YATO_CONSTEXPR_FUNC
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cend();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.end();
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            YATO_CONSTEXPR_FUNC
            yato::range<const_iterator> crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(cbegin(), cend());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<iterator> range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(begin(), end());
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
             *  Get number of dimensions
             */
            YATO_CONSTEXPR_FUNC
            size_t dimensions() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_num;
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
            size_t dim_size(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < dimensions_num)
                    ? m_plain_vector.size()
                    : (YATO_THROW_ASSERT_EXCEPT("yato::vector_nd[dim_size]: dimension index is out of range"), 0);
#else
                (void)idx;
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
            size_t size() const YATO_NOEXCEPT_KEYWORD
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
             */
            void resize(size_t length)
            {
                m_plain_vector.resize(length);
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

        };

    }

    template<typename _DataType, size_t _DimensionsNum, typename _Allocator = std::allocator<_DataType> >
    using vector_nd = details::vector_nd_impl<_DataType, _DimensionsNum, _Allocator>;
}

#endif