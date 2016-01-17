/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_ND_H_
#define _YATO_ARRAY_ND_H_

#include <array>
#include "assert.h"
#include "type_traits.h"

namespace yato
{
    struct array_layout_plain {};
    struct array_layout_dynamic {};

    namespace details
    {
        struct null_shape {};

        template<size_t _Dim_Head, size_t... _Dim_Tail>
        struct plain_array_shape {
            using hyper_shape = plain_array_shape<_Dim_Tail...>;
            constexpr static const size_t total_size = _Dim_Head * hyper_shape::total_size;
            constexpr static const size_t dimensions[sizeof...(_Dim_Tail) + 1] = { _Dim_Head, _Dim_Tail... };
        };

        template<size_t _Dim_Head>
        struct plain_array_shape<_Dim_Head> {
            using hyper_shape = null_shape;
            constexpr static const size_t total_size = _Dim_Head;
            constexpr static const size_t dimensions[1] = { _Dim_Head };
        };

        template<typename _DataType, typename _Shape>
        class sub_array 
        {
        public:
            using data_type = _DataType;

        private:
            using _my_shape = _Shape;
            using _my_hyper_shape = typename _my_shape::hyper_shape;
            data_type* m_base_ptr;

        public:
            explicit constexpr sub_array(data_type* base_ptr) noexcept
                : m_base_ptr(base_ptr)
            { }

            constexpr sub_array(const sub_array & other) noexcept 
                : m_base_ptr(other.m_base_ptr)
            { }

            sub_array& operator=(const sub_array & other) noexcept {
                m_base_ptr = other.m_base_ptr;
            }

            ~sub_array()
            { }

            /**
            *    case of Nd shape
            */
            template <typename _HyperShape = _my_hyper_shape>
            constexpr 
                typename std::enable_if<!std::is_same<_HyperShape, null_shape>::value,
                sub_array<data_type, _HyperShape> >::type
                operator[](size_t idx) const noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                return sub_array<data_type, _HyperShape>{
                    idx < _my_shape::dimensions[0] ? m_base_ptr + idx * _HyperShape::total_size :
                        (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), nullptr) 
                };
#else
                return sub_array<data_type, _HyperShape>{ m_base_ptr + idx * _HyperShape::total_size };
#endif
            }

            /**
             *    case of 1d shape 
             */
            template <typename _HyperShape = _my_hyper_shape>
            constexpr
                typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                _DataType&>::type
                operator[](size_t idx) const noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                return (idx < _my_shape::dimensions[0]) ? *(m_base_ptr + idx) :
                    (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), *m_base_ptr);
#else
                return *(m_base_ptr + idx);
#endif
            }

        };


        
        template<typename _DataType, typename _Shape, typename _Layout, typename _Allocator>
        class array_nd_impl 
        {
        public:
            using data_type = _DataType;
            using pointer = std::add_pointer<data_type>;
            using shape = _Shape;
            using layout_type = _Layout;
            using allocator_type = _Allocator;

            static_assert(std::is_same<layout_type, array_layout_plain>::value, "Only plain layout is supported for now!");

        private:
            std::array<data_type, shape::total_size> m_plain_array;

        public:
            constexpr array_nd_impl()
            { }

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
                sub_array<const data_type, _HyperShape> >::type
            operator[](size_t idx) const noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                return sub_array<const data_type, _HyperShape>{
                    (idx < shape::dimensions[0]) ? &m_plain_array[0] + idx * _HyperShape::total_size :
                        (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), nullptr) 
                };
#else
                return sub_array<const data_type, _HyperShape>{ &m_plain_array[0] + idx * _HyperShape::total_size };
#endif
            }

            template <typename _HyperShape = typename shape::hyper_shape>
                typename std::enable_if<!std::is_same<_HyperShape, null_shape>::value,
                sub_array<data_type, _HyperShape> >::type
            operator[](size_t idx) noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                YATO_ASSERT(idx < shape::dimensions[0], "yato::array_nd: out of range!");
#endif
                return sub_array<data_type, _HyperShape>{ &m_plain_array[0] + idx * _HyperShape::total_size };
            }

            /**
            *    case of 1d shape
            */
            template <typename _HyperShape = typename shape::hyper_shape>
            constexpr
                typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                const data_type&>::type
            operator[](size_t idx) const noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                return idx < shape::dimensions[0] ? m_plain_array[idx] :
                    (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), m_plain_array[0]);
#else
                return m_plain_array[idx];
#endif
            }

            template <typename _HyperShape = typename shape::hyper_shape>
                typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                data_type&>::type
            operator[](size_t idx) noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                YATO_ASSERT(idx < shape::dimensions[0], "yato::array_nd: out of range!");
#endif
                return m_plain_array[idx];
            }

        };
    }

    template<typename _DataType, size_t _First_Dimension, size_t... _More_Dimensions> 
    using array_nd = details::array_nd_impl < _DataType, details::plain_array_shape<_First_Dimension, _More_Dimensions...>, array_layout_plain, std::allocator<_DataType>>;

}

#endif