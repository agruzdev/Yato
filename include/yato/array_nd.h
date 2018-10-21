/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_ND_H_
#define _YATO_ARRAY_ND_H_

#include <array>
#include <type_traits>
#include <vector>
#include "assert.h"
#include "array_view.h"
#include "container_nd.h"
#include "range.h"

namespace yato
{

    namespace details
    {
        struct null_shape {};

YATO_PRAGMA_WARNING_PUSH
#ifdef YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4268)
#endif
        template <size_t DimHead_, size_t... DimTail_>
        struct plain_array_shape
        {
            static_assert(DimHead_ > 0, "The each array dimension should be greater than 0");

            using hyper_shape = plain_array_shape<DimTail_...>;
            static YATO_CONSTEXPR_VAR size_t total_size = DimHead_ * hyper_shape::total_size;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = sizeof...(DimTail_) + 1;
            static YATO_CONSTEXPR_VAR size_t top_dimension = DimHead_;

            static YATO_CONSTEXPR_VAR std::array<size_t, sizeof...(DimTail_) + 1> extents = { DimHead_, DimTail_... };
        };

        template <size_t DimHead_>
        struct plain_array_shape<DimHead_> 
        {
            static_assert(DimHead_ > 0, "The each array dimension should be greater than 0");

            using hyper_shape = null_shape;
            static YATO_CONSTEXPR_VAR size_t total_size = DimHead_;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;
            static YATO_CONSTEXPR_VAR size_t top_dimension = DimHead_;

            static YATO_CONSTEXPR_VAR std::array<size_t, 1> extents = { DimHead_ };
        };

        template <typename VTy_, typename Shape_, size_t... Offsets_>
        struct plain_array_strides
            : public plain_array_strides<VTy_, typename Shape_::hyper_shape, Offsets_..., Shape_::total_size>
        { };

        template <typename VTy_, size_t... Offsets_>
        struct plain_array_strides<VTy_, null_shape, Offsets_...>
        {
            static YATO_CONSTEXPR_VAR std::array<size_t, sizeof...(Offsets_)> values = { Offsets_ * sizeof(VTy_)... };
        };


#if !(defined(__cplusplus) && (__cplusplus >= 201700L))
        template <size_t DimHead_, size_t... DimTail_>
        YATO_CONSTEXPR_VAR std::array<size_t, sizeof...(DimTail_) + 1> plain_array_shape<DimHead_, DimTail_...>::extents;

        template <size_t DimHead_>
        YATO_CONSTEXPR_VAR std::array<size_t, 1> plain_array_shape<DimHead_>::extents;

        template <typename VTy_, size_t... Offsets_>
        YATO_CONSTEXPR_VAR std::array<size_t, sizeof...(Offsets_)> plain_array_strides<VTy_, null_shape, Offsets_...>::values;
#endif
YATO_PRAGMA_WARNING_POP

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
            iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_iter;
            }

            /**
             * Get end iterator
             */
            YATO_CONSTEXPR_FUNC
            iterator plain_end() {
                return std::next(m_iter, _my_shape::total_size);
            }

            /**
             *	Get begin iterator
             */
            YATO_CONSTEXPR_FUNC
            iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_iter;
            }

            /**
             * Get end iterator
             */
            YATO_CONSTEXPR_FUNC
            iterator plain_cend() const {
                return std::next(m_iter, _my_shape::total_size);
            }

            YATO_CONSTEXPR_FUNC
            auto plain_range()
            {
                return yato::make_range(plain_begin(), plain_end());
            }

            YATO_CONSTEXPR_FUNC
            auto plain_crange() const
            {
                return yato::make_range(plain_cbegin(), plain_cend());
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

        template<typename ValueType_, typename Shape_>
        class array_nd_impl
            //: public const_container_nd<ValueType_, Shape_::dimensions_number, array_nd_impl<ValueType_, Shape_>>
        {
        public:
            using value_type   = ValueType_;
            using size_type    = size_t;
            using pointer_type = std::add_pointer<value_type>;
            using shape        = Shape_;

            using container_type = std::array<value_type, shape::total_size>;

            /**
             * Iterator allowing to pass through all elements of the multidimensional array 
             */
            using iterator = std::add_pointer_t<value_type>;
            /**
             * Const iterator allowing to pass through all elements of the multidimensional array
             */
            using const_iterator = std::add_pointer_t<std::add_const_t<value_type>>;
            //-------------------------------------------------------

            static YATO_CONSTEXPR_VAR size_t dimensions_number = shape::dimensions_number;

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
            const_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD 
            {
                return m_plain_array.data();
            }
            /**
            *  Get iterator to the begin of the array
            *  Will pass through all elements of the multidimensional array
            */
            YATO_CONSTEXPR_FUNC
            iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_array.data();
            }
            /**
             *  Get const iterator to the end of the array
             */
            YATO_CONSTEXPR_FUNC 
            const_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD 
            {
                return m_plain_array.data() + total_size();
            }
            /**
            *  Get iterator to the end of the array
            */
            YATO_CONSTEXPR_FUNC
            iterator plain_end() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_array.data() + total_size();
            }

            YATO_CONSTEXPR_FUNC
            auto plain_range()
            {
                return yato::make_range(plain_begin(), plain_end());
            }

            YATO_CONSTEXPR_FUNC
            auto plain_crange() const
            {
                return yato::make_range(plain_cbegin(), plain_cend());
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
                return sub_array<const_iterator, _HyperShape>{ m_plain_array.data() + idx * _HyperShape::total_size };
            }

            template <typename _HyperShape = typename shape::hyper_shape>
            typename std::enable_if<!std::is_same<_HyperShape, null_shape>::value,
                sub_array<iterator, _HyperShape> >::type
            operator[](size_t idx)
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return sub_array<iterator, _HyperShape>{ m_plain_array.data() + idx * _HyperShape::total_size };
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
                return m_plain_array[idx];
            }

            template <typename _HyperShape = typename shape::hyper_shape>
            YATO_CONSTEXPR_FUNC_CXX14
            typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
            operator[](size_t idx) 
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return m_plain_array[idx];
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
            YATO_CONSTEXPR_FUNC_CXX14
            size_t size(size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < shape::dimensions_number);
                return shape::extents[idx];
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
             * Get byte offset till next sub-view
             * Returns size in bytes for 1D view
             */
            YATO_CONSTEXPR_FUNC_CXX14
            size_type stride(size_t idx) const
            {
                YATO_REQUIRES(idx < shape::dimensions_number - 1);
                return plain_array_strides<value_type, shape>::values[idx + 1];
            }

            /**
             * Get dimensions
             */
            dimensionality<dimensions_number, size_type> dimensions() const
            {
                return dimensionality<dimensions_number, size_type>(dimensions_range());
            }

            /**
             * Get dimensions
             */
            strides_array<dimensions_number - 1, size_type> strides() const
            {
                return strides_array<dimensions_number - 1, size_type>(strides_range());
            }

            /**
             * Get number of dimensions
             */
            size_t dimensions_num() const
            {
                return dimensions_number;
            }
      
            /**
             *  Get dimensions range
             */
            auto dimensions_range() const
            {
                return yato::make_range(std::cbegin(shape::extents), std::cend(shape::extents));
            }

            /**
             *  Get dimensions range
             */
            auto strides_range() const
            {
                return yato::make_range(std::next(std::cbegin(plain_array_strides<value_type, shape>::values)), std::cend(plain_array_strides<value_type, shape>::values));
            }

            /**
             *  Get raw pointer to data
             *  Points to valid continuous storage with all elements
             *  The order of elements is same like for native array T[][]..[]
             */
            YATO_CONSTEXPR_FUNC 
            const value_type* data() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_array.data();
            }

            /**
             *  Get raw pointer to data
             *  Points to valid continuous storage with all elements
             *  The order of elements is same like for native array T[][]..[]
             */
            value_type* data() YATO_NOEXCEPT_KEYWORD 
            {
                return m_plain_array.data();
            }


            /**
             *  Get raw pointer to data
             *  Points to valid continuous storage with all elements
             *  The order of elements is same like for native array T[][]..[]
             */
            YATO_CONSTEXPR_FUNC 
            const value_type* cdata() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_array.data();
            }

            /**
             *  Fill array with constant value
             */
            template<typename _T>
            typename std::enable_if<std::is_convertible<_T, value_type>::value, void>::type 
            fill(const _T& value) 
                YATO_NOEXCEPT_KEYWORD_EXP(std::is_nothrow_copy_assignable<_T>::value) 
            {
                std::fill(plain_begin(), plain_end(), value);
            }
        };

        template <typename _T, typename _Enable = void>
        struct is_array_nd 
            : std::false_type
        { };

        template <typename _T>
        struct is_array_nd<_T, typename std::enable_if<
            std::is_same<_T, array_nd_impl<typename _T::value_type, typename _T::shape> >::value>::type >
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



    template <typename Ty_, typename Shape_>
    YATO_CONSTEXPR_FUNC
    array_view_nd<std::add_const_t<Ty_>, Shape_::dimensions_number> make_view(const details::array_nd_impl<Ty_, Shape_> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view_nd<std::add_const_t<Ty_>, Shape_::dimensions_number>(arr.cdata(), arr.dimensions(), arr.strides());
    }

    template <typename Ty_, typename Shape_>
    YATO_CONSTEXPR_FUNC
    array_view_nd<Ty_, Shape_::dimensions_number> make_view(details::array_nd_impl<Ty_, Shape_> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view_nd<Ty_, Shape_::dimensions_number>(arr.data(), arr.dimensions(), arr.strides());
    }
}

#endif
