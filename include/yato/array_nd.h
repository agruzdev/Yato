/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
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
        struct null_shape {};

        template<size_t _Dim_Head, size_t... _Dim_Tail>
        struct plain_array_shape 
        {
            static_assert(_Dim_Head > 0, "The each array dimension should be greater than 0");

            using hyper_shape = plain_array_shape<_Dim_Tail...>;
            static YATO_CONSTEXPR_VAR size_t total_size = _Dim_Head * hyper_shape::total_size;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = sizeof...(_Dim_Tail) + 1;
            static YATO_CONSTEXPR_VAR size_t top_dimension = _Dim_Head;
        };

        template<size_t _Dim_Head>
        struct plain_array_shape<_Dim_Head> 
        {
            static_assert(_Dim_Head > 0, "The each array dimension should be greater than 0");

            using hyper_shape = null_shape;
            static YATO_CONSTEXPR_VAR size_t total_size = _Dim_Head;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;
            static YATO_CONSTEXPR_VAR size_t top_dimension = _Dim_Head;
        };

        template<typename _Shape, size_t _Num>
        struct get_dimension
        {
            static YATO_CONSTEXPR_VAR size_t value = (_Num == 0)
                ? _Shape::top_dimension
                : get_dimension<typename _Shape::hyper_shape, _Num - 1>::value;
        };

        template<size_t _Num>
        struct get_dimension<null_shape, _Num>
        {
            static YATO_CONSTEXPR_VAR size_t value = 0;
        };

        template<typename _Iterator, typename _Shape>
        class sub_array 
        {
        public:
            /**
             *	Iterator for elements of the sub-array
             *  May be const or not
             */
            using iterator = _Iterator;

        private:
            using _my_shape = _Shape;
            using _my_hyper_shape = typename _my_shape::hyper_shape;
            iterator m_iter;

        public:
            YATO_CONSTEXPR_FUNC
            explicit sub_array(const _Iterator & iter) YATO_NOEXCEPT_KEYWORD
                : m_iter(iter)
            { }

            YATO_CONSTEXPR_FUNC
            explicit sub_array(_Iterator && iter) YATO_NOEXCEPT_KEYWORD
                : m_iter(std::move(iter))
            { }

            YATO_CONSTEXPR_FUNC
            sub_array(const sub_array & other) YATO_NOEXCEPT_KEYWORD 
                : m_iter(other.m_iter)
            { }

            sub_array& operator=(const sub_array & other) YATO_NOEXCEPT_KEYWORD
            {
                if (&other != this) {
                    m_iter = other.m_iter;
                }
            }

            ~sub_array() = default;

            /**
             *	Get begin iterator
             */
            YATO_CONSTEXPR_FUNC
            iterator begin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_iter;
            }

            /**
             * Get end iterator
             */
            YATO_CONSTEXPR_FUNC
            iterator end() const {
                return std::next(m_iter, _my_shape::total_size);
            }

            /**
            *    case of Nd shape
            */
            template <typename _HyperShape = typename _my_shape::hyper_shape>
            YATO_CONSTEXPR_FUNC_CXX14
            typename std::enable_if<!std::is_same<_HyperShape, null_shape>::value,
                sub_array<iterator, _HyperShape> >::type
            operator[](size_t idx) const 
            {
                YATO_REQUIRES(idx < _my_shape::top_dimension);
                return sub_array<iterator, _HyperShape>{ std::next(m_iter, idx * _HyperShape::total_size) };
            }

            template<typename _HyperShape = typename _my_shape::hyper_shape, typename... _Tail>
            typename std::enable_if<
                !std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
            at(size_t firstIdx, _Tail... indexes)
            {
                if (firstIdx >= _my_shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx].at(std::forward<_Tail>(indexes)...);
            }

            /**
             *    case of 1d shape 
             */
            template <typename _HyperShape = typename _my_shape::hyper_shape>
            YATO_CONSTEXPR_FUNC_CXX14
            typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
            operator[](size_t idx) const 
            {
                YATO_REQUIRES(idx < _my_shape::top_dimension);
                return *std::next(m_iter, idx);
            }

            template<typename _HyperShape = typename _my_shape::hyper_shape>
            typename std::enable_if<
                std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
            at(size_t firstIdx)
            {
                if (firstIdx >= _my_shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx];
            }


        };

        template<typename _DataType, typename _Shape>
        class array_nd_impl 
        {
        public:
            using data_type = _DataType;
            using pointer = std::add_pointer<data_type>;
            using shape = _Shape;

            using container_type = std::array<data_type, shape::total_size>;
            /**
             *	Iterator allowing to pass through all elements of the multidimensional array 
             */
            using iterator = typename container_type::iterator;
            /**
            *	Const iterator allowing to pass through all elements of the multidimensional array
            */
            using const_iterator = typename container_type::const_iterator;
            //-------------------------------------------------------

        private:
            container_type m_plain_array;

        public:
            YATO_CONSTEXPR_FUNC
            array_nd_impl() YATO_NOEXCEPT_KEYWORD
                : m_plain_array()
            { }

            array_nd_impl(const array_nd_impl & other) 
                YATO_NOEXCEPT_KEYWORD_EXP(std::is_nothrow_copy_constructible<container_type>::value)
                : m_plain_array(other.m_plain_array)
            { }

            array_nd_impl& operator=(const array_nd_impl & other) 
                YATO_NOEXCEPT_KEYWORD_EXP(std::is_nothrow_copy_assignable<container_type>::value)
            {
                if (this != &other) {
                    m_plain_array = other.m_plain_array;
                }
                return *this;
            }

            array_nd_impl(array_nd_impl && other) 
                YATO_NOEXCEPT_KEYWORD_EXP(std::is_nothrow_move_constructible<container_type>::value)
                : m_plain_array(std::move(other))
            { }

            array_nd_impl& operator=(array_nd_impl && other) 
                YATO_NOEXCEPT_KEYWORD_EXP(std::is_nothrow_move_assignable<container_type>::value)
            {
                if (this != &other) {
                    m_plain_array = std::move(other);
                }
                return *this;
            }

            ~array_nd_impl() = default;

            /**
             *	Swap arrays data
             */
            void swap(array_nd_impl & other) YATO_NOEXCEPT_KEYWORD 
            {
                m_plain_array.swap(other.m_plain_array);
            }

            /**
             *  Get constant iterator to the begin of the array
             *  Will pass through all elements of the multidimensional array
             */
            YATO_CONSTEXPR_FUNC
            const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD 
            {
                return m_plain_array.cbegin();
            }
            /**
            *  Get iterator to the begin of the array
            *  Will pass through all elements of the multidimensional array
            */
            iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_array.begin();
            }
            /**
             *	Get const iterator to the end of the array
             */
            YATO_CONSTEXPR_FUNC 
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD 
            {
                return m_plain_array.cend();
            }
            /**
            *	Get iterator to the end of the array
            */
            iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_array.end();
            }


            /**
            *    case of Nd shape
            */
            template <typename _HyperShape = typename shape::hyper_shape>
            YATO_CONSTEXPR_FUNC_CXX14
            typename std::enable_if<!std::is_same<_HyperShape, null_shape>::value,
                sub_array<const_iterator, _HyperShape> >::type
            operator[](size_t idx) const 
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return sub_array<const_iterator, _HyperShape>{ std::next(cbegin(), idx * _HyperShape::total_size) };
            }

            template <typename _HyperShape = typename shape::hyper_shape>
            typename std::enable_if<!std::is_same<_HyperShape, null_shape>::value,
                sub_array<iterator, _HyperShape> >::type
            operator[](size_t idx)
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return sub_array<iterator, _HyperShape>{ std::next(begin(), idx * _HyperShape::total_size) };
            }


            template<typename _HyperShape = typename shape::hyper_shape, typename... _Tail>
            typename std::enable_if<
                !std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
            at(size_t firstIdx, _Tail... indexes)
            {
                if (firstIdx >= shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx].at(std::forward<_Tail>(indexes)...);
            }

            /**
            *    case of 1d shape
            */
            template <typename _HyperShape = typename shape::hyper_shape>
            YATO_CONSTEXPR_FUNC_CXX14
            typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<const_iterator>())>::type
            operator[](size_t idx) const 
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return *std::next(cbegin(), idx);
            }

            template <typename _HyperShape = typename shape::hyper_shape>
            YATO_CONSTEXPR_FUNC_CXX14
            typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
            operator[](size_t idx) 
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return *std::next(begin(), idx);
            }

            template<typename _HyperShape = typename shape::hyper_shape>
            typename std::enable_if<
                std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
            at(size_t firstIdx) 
            {
                if (firstIdx >= shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx];
            }

            /**
             * Get size along one dimension	
             */
            template<size_t _Dimension>
            YATO_CONSTEXPR_FUNC 
            size_t size() const YATO_NOEXCEPT_KEYWORD 
            {
                static_assert(_Dimension < shape::dimensions_number, "yato::array_nd: dimension index is out of range!");
                return get_dimension<shape, _Dimension>::value;
            }

            /**
             *	Get total size of the array
             */
            YATO_CONSTEXPR_FUNC
            size_t total_size() const YATO_NOEXCEPT_KEYWORD 
            {
                return shape::total_size;
            }

            /**
             *	Get raw pointer to data
             *  Points to valid continuous storage with all elements
             *  The order of elements is same like for native array T[][]..[]
             */
            YATO_CONSTEXPR_FUNC 
            const data_type* data() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_array.data();
            }

            /**
             *	Get raw pointer to data
             *  Points to valid continuous storage with all elements
             *  The order of elements is same like for native array T[][]..[]
             */
            data_type* data() YATO_NOEXCEPT_KEYWORD 
            {
                return m_plain_array.data();
            }


            /**
             *	Fill array with constant value
             */
            template<typename _T>
            typename std::enable_if<std::is_convertible<_T, data_type>::value, void>::type 
            fill(const _T& value) 
                YATO_NOEXCEPT_KEYWORD_EXP(std::is_nothrow_copy_assignable<_T>::value) 
            {
                std::fill(begin(), end(), value);
            }
        };

        template <typename _T, typename _Enable = void>
        struct is_array_nd 
            : std::false_type
        { };

        template <typename _T>
        struct is_array_nd<_T, typename std::enable_if<
            std::is_same<_T, array_nd_impl<typename _T::data_type, typename _T::shape> >::value>::type >
            : std::true_type
        { };

        template<typename _Value, typename _Shape>
        void swap(array_nd_impl<_Value, _Shape> & one, array_nd_impl<_Value, _Shape> & another) YATO_NOEXCEPT_KEYWORD
        {
            one.swap(another);
        } 
    }

    using details::is_array_nd;
    using details::swap;

    /**
     *  Create multidimensional array on stack
     */
    template<typename _DataType, size_t _First_Dimension, size_t... _More_Dimensions> 
    using array_nd = details::array_nd_impl < _DataType, details::plain_array_shape<_First_Dimension, _More_Dimensions...> >;

    // Overload size accessors
    template <typename _DataType, typename _Shape>
    YATO_CONSTEXPR_FUNC
    size_t length(const details::array_nd_impl<_DataType, _Shape> & /*array*/)
    {
        static_assert(_Shape::dimensions_number == 1, "Invalid shape");
        return details::get_dimension<_Shape, 0>::value;
    }

    template <typename _DataType, typename _Shape>
    YATO_CONSTEXPR_FUNC
    size_t height_2d(const details::array_nd_impl<_DataType, _Shape> & /*array*/)
    {
        static_assert(_Shape::dimensions_number == 2, "Invalid shape");
        return details::get_dimension<_Shape, 0>::value;
    }

    template <typename _DataType, typename _Shape>
    YATO_CONSTEXPR_FUNC
    size_t width_2d(const details::array_nd_impl<_DataType, _Shape> & /*array*/)
    {
        static_assert(_Shape::dimensions_number == 2, "Invalid shape");
        return details::get_dimension<_Shape, 1>::value;
    }

    template <typename _DataType, typename _Shape>
    YATO_CONSTEXPR_FUNC
    size_t depth_3d(const details::array_nd_impl<_DataType, _Shape> & /*array*/)
    {
        static_assert(_Shape::dimensions_number == 3, "Invalid shape");
        return details::get_dimension<_Shape, 0>::value;
    }

    template <typename _DataType, typename _Shape>
    YATO_CONSTEXPR_FUNC
    size_t height_3d(const details::array_nd_impl<_DataType, _Shape> & /*array*/)
    {
        static_assert(_Shape::dimensions_number == 3, "Invalid shape");
        return details::get_dimension<_Shape, 1>::value;
    }

    template <typename _DataType, typename _Shape>
    YATO_CONSTEXPR_FUNC
    size_t width_3d(const details::array_nd_impl<_DataType, _Shape> & /*array*/)
    {
        static_assert(_Shape::dimensions_number == 3, "Invalid shape");
        return details::get_dimension<_Shape, 2>::value;
    }
}

#endif
