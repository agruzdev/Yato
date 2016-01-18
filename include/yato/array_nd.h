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
    struct array_storage_on_stack {};
    struct array_storage_on_heap {};

    namespace details
    {
        struct null_shape {};

        template<size_t _Dim_Head, size_t... _Dim_Tail>
        struct plain_array_shape 
        {
            static_assert(_Dim_Head > 0, "The each array dimension should be greater than 0");

            using hyper_shape = plain_array_shape<_Dim_Tail...>;
            constexpr static const size_t total_size = _Dim_Head * hyper_shape::total_size;
            constexpr static const size_t dimensions[sizeof...(_Dim_Tail) + 1] = { _Dim_Head, _Dim_Tail... };
        };

        template<size_t _Dim_Head>
        struct plain_array_shape<_Dim_Head> 
        {
            static_assert(_Dim_Head > 0, "The each array dimension should be greater than 0");

            using hyper_shape = null_shape;
            constexpr static const size_t total_size = _Dim_Head;
            constexpr static const size_t dimensions[1] = { _Dim_Head };
        };

        template<typename _Iterator, typename _Shape>
        class sub_array 
        {
        public:
            using iterator = _Iterator;

        private:
            using _my_shape = _Shape;
            using _my_hyper_shape = typename _my_shape::hyper_shape;
            iterator m_iter;

        public:
            explicit constexpr sub_array(const _Iterator & iter) noexcept
                : m_iter(iter)
            { }

            explicit constexpr sub_array(_Iterator && iter) noexcept
                : m_iter(iter)
            { }

            constexpr sub_array(const sub_array & other) noexcept 
                : m_iter(other.m_iter)
            { }

            sub_array& operator=(const sub_array & other) noexcept {
                if (&other != this) {
                    m_iter = other.m_iter;
                }
            }

            ~sub_array()
            { }

            /**
            *    case of Nd shape
            */
            template <typename _HyperShape = _my_hyper_shape>
            constexpr 
                typename std::enable_if<!std::is_same<_HyperShape, null_shape>::value,
                sub_array<iterator, _HyperShape> >::type
                operator[](size_t idx) const noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                return sub_array<iterator, _HyperShape>{
                    idx < _my_shape::dimensions[0] ? std::next(m_iter, idx * _HyperShape::total_size) :
                        (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), m_iter)
                };
#else
                return sub_array<iterator, _HyperShape>{ std::next(m_iter, idx * _HyperShape::total_size) };
#endif
            }

            /**
             *    case of 1d shape 
             */
            template <typename _HyperShape = _my_hyper_shape>
            constexpr
                typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
                operator[](size_t idx) const noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                return (idx < _my_shape::dimensions[0]) ? *std::next(m_iter, idx) :
                    (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), *m_iter);
#else
                return *std::next(m_iter, idx);
#endif
            }

        };

        //-------------------------------------------------------
        // Container trait
        // Define container type and initialization way
        template<typename _DataType, size_t _Size, typename _Storage>
        struct container_trait {};

        template<typename _DataType, size_t _Size>
        struct container_trait<_DataType, _Size, array_storage_on_stack>
        {
            using type = std::array<_DataType, _Size>;
            constexpr static const auto initializer = static_cast<_DataType>(0);

            constexpr static void do_init(type & /*container*/) noexcept {}
        };

        template<typename _DataType, size_t _Size>
        struct container_trait<_DataType, _Size, array_storage_on_heap>
        {
            using type = std::vector<_DataType>;
            constexpr static const std::initializer_list<_DataType> initializer = {};

            static void do_init(type & container) {
                container.resize(_Size, static_cast<_DataType>(0));
            }
        };


        template<typename _DataType, typename _Shape, typename _Storage>
        class array_nd_impl 
        {
        public:
            using data_type = _DataType;
            using pointer = std::add_pointer<data_type>;
            using shape = _Shape;
            using storage_type = _Storage;

            using _my_container_trait = container_trait<data_type, shape::total_size, storage_type>;
            using container_type = typename _my_container_trait::type;

            using iterator = typename container_type::iterator;
            using const_iterator = typename container_type::const_iterator;
            //-------------------------------------------------------

        private:
            container_type m_plain_array;

        public:
            constexpr array_nd_impl() noexcept(noexcept(_my_container_trait::do_init(m_plain_array)))
                : m_plain_array{_my_container_trait::initializer}
            {
                _my_container_trait::do_init(m_plain_array);
            }

            //ToDo:: implement later
            array_nd_impl(const array_nd_impl&) = delete;
            array_nd_impl& operator=(const array_nd_impl&) = delete;

            ~array_nd_impl()
            { }


            /**
            *    case of Nd shape
            */
            template <typename _HyperShape = typename shape::hyper_shape>
            constexpr
                typename std::enable_if<!std::is_same<_HyperShape, null_shape>::value,
                sub_array<const_iterator, _HyperShape> >::type
            operator[](size_t idx) const noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                return sub_array<const_iterator, _HyperShape>{
                    (idx < shape::dimensions[0]) ? std::next(std::cbegin(m_plain_array), idx * _HyperShape::total_size) :
                        (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), std::begin(m_plain_array))
                };
#else
                return sub_array<const_iterator, _HyperShape>{ std::next(std::cbegin(m_plain_array), idx * _HyperShape::total_size) };
#endif
            }

            template <typename _HyperShape = typename shape::hyper_shape>
                typename std::enable_if<!std::is_same<_HyperShape, null_shape>::value,
                sub_array<iterator, _HyperShape> >::type
            operator[](size_t idx) noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                YATO_ASSERT(idx < shape::dimensions[0], "yato::array_nd: out of range!");
#endif
                return sub_array<iterator, _HyperShape>{ std::next(std::begin(m_plain_array), idx * _HyperShape::total_size) };
            }

            /**
            *    case of 1d shape
            */
            template <typename _HyperShape = typename shape::hyper_shape>
            constexpr
                typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<const_iterator>())>::type
            operator[](size_t idx) const noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                return idx < shape::dimensions[0] ? *std::next(std::cbegin(m_plain_array), idx) :
                    (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), *std::cbegin(m_plain_array));
#else
                return *std::next(std::cbegin(m_plain_array), idx);
#endif
            }

            template <typename _HyperShape = typename shape::hyper_shape>
                typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
            operator[](size_t idx) noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                YATO_ASSERT(idx < shape::dimensions[0], "yato::array_nd: out of range!");
#endif
                return *std::next(std::begin(m_plain_array), idx);
            }

        };
    }

    //Separate first dimension to force having at least one dimension 
    template<typename _DataType, size_t _First_Dimension, size_t... _More_Dimensions> 
    using array_nd = details::array_nd_impl < _DataType, details::plain_array_shape<_First_Dimension, _More_Dimensions...>, array_storage_on_stack>;

    template<typename _DataType, size_t _First_Dimension, size_t... _More_Dimensions>
    using vector_nd = details::array_nd_impl < _DataType, details::plain_array_shape<_First_Dimension, _More_Dimensions...>, array_storage_on_heap>;

}

#endif