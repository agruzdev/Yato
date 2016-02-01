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

namespace yato
{
   
    namespace details
    {


        template <typename _DataType, size_t _DimensionsNum, typename _Allocator>
        class vector_nd_impl
        {
        public:
            using my_type = vector_nd_impl<_DataType, _DimensionsNum, _Allocator>;
            using data_type = _DataType;
            using allocator_type = _Allocator;

            static YATO_CONSTEXPR_VAR size_t dimensions_num = _DimensionsNum;
            static_assert(dimensions_num > 1, "Implementation for dimensions number larger than 1");

        private:
            using sizes_array = std::array<size_t, dimensions_num>;
            using size_iterator = typename sizes_array::iterator;

            sizes_array m_dimensions;
            sizes_array m_sub_sizes;
            std::vector<data_type, allocator_type> m_plain_vector;

            void _init_sizes(const std::initializer_list<size_t> & sizes) YATO_NOEXCEPT_KEYWORD{
                std::copy(sizes.begin(), sizes.end(), m_dimensions.begin());
                m_sub_sizes[dimensions_num - 1] = m_dimensions[dimensions_num - 1];
                for (size_t i = dimensions_num - 1; i > 0; --i) {
                    m_sub_sizes[i - 1] = m_dimensions[i - 1] * m_sub_sizes[i];
                }
            }

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
                _init_sizes(sizes);
                m_plain_vector.resize(m_sub_sizes[0]);
            }

            /**
             *	Create with initialization
             */
            vector_nd_impl(const std::initializer_list<size_t> & sizes, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (sizes.size() != dimensions_num) {
                    throw yato::assertion_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                _init_sizes(sizes);
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
             *	Move assign
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
             *	Destructor
             */
            ~vector_nd_impl()
            { }

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
        };

        template<typename _DataType, size_t _DimensionsNum, typename _Allocator>
        struct alias_impl
        {
            using type = vector_nd_impl<_DataType, _DimensionsNum, _Allocator>;
        };

        template<typename _DataType, typename _Allocator>
        struct alias_impl<_DataType, 1, _Allocator>
        {
            using type = std::vector<_DataType, _Allocator>;
        };
    }

    template<typename _DataType, size_t _DimensionsNum, typename _Allocator = std::allocator<_DataType> >
    using vector_nd = typename details::alias_impl<_DataType, _DimensionsNum, _Allocator>::type;
}

#endif