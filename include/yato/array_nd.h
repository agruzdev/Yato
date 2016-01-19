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
            constexpr static const size_t dimensions_number = sizeof...(_Dim_Tail) + 1;
            constexpr static const size_t dimensions[dimensions_number] = { _Dim_Head, _Dim_Tail... };
            constexpr static const size_t top_dimension = dimensions[0];
        };

        template<size_t _Dim_Head>
        struct plain_array_shape<_Dim_Head> 
        {
            static_assert(_Dim_Head > 0, "The each array dimension should be greater than 0");

            using hyper_shape = null_shape;
            constexpr static const size_t total_size = _Dim_Head;
            constexpr static const size_t dimensions_number = 1;
            constexpr static const size_t dimensions[dimensions_number] = { _Dim_Head };
            constexpr static const size_t top_dimension = dimensions[0];
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
            explicit constexpr sub_array(const _Iterator & iter) noexcept
                : m_iter(iter)
            { }

            explicit constexpr sub_array(_Iterator && iter) noexcept
                : m_iter(std::move(iter))
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
             *	Get begin iterator
             */
            iterator begin() noexcept {
                return m_iter;
            }

            /**
             * Get end iterator
             */
            iterator end() noexcept {
                return std::next(m_iter, _my_shape::total_size);
            }

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
                    idx < _my_shape::top_dimension ? std::next(m_iter, idx * _HyperShape::total_size) :
                        (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), m_iter)
                };
#else
                return sub_array<iterator, _HyperShape>{ std::next(m_iter, idx * _HyperShape::total_size) };
#endif
            }

            template<typename _HyperShape = _my_hyper_shape, typename _FirstIdx, typename... _Tail>
            typename std::enable_if<
                !std::is_same<_HyperShape, null_shape>::value && std::is_convertible<_FirstIdx, size_t>::value,
                decltype(*std::declval<iterator>())>::type
            at(_FirstIdx firstIdx, _Tail... indexes) {
                if (firstIdx >= _my_shape::top_dimension) {
                    throw std::runtime_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx].at(std::forward<_Tail>(indexes)...);
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
                return (idx < _my_shape::top_dimension) ? *std::next(m_iter, idx) :
                    (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), *m_iter);
#else
                return *std::next(m_iter, idx);
#endif
            }

            template<typename _HyperShape = _my_hyper_shape, typename _FirstIdx>
            typename std::enable_if<
                std::is_same<_HyperShape, null_shape>::value && std::is_convertible<_FirstIdx, size_t>::value,
                decltype(*std::declval<iterator>())>::type
            at(_FirstIdx firstIdx) {
                if (firstIdx >= _my_shape::top_dimension) {
                    throw std::runtime_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx];
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
            constexpr array_nd_impl() noexcept(noexcept(_my_container_trait::do_init(m_plain_array)))
                : m_plain_array{_my_container_trait::initializer}
            {
                _my_container_trait::do_init(m_plain_array);
            }

            array_nd_impl(const array_nd_impl & other) noexcept(std::is_nothrow_copy_constructible<container_type>::value)
                : m_plain_array(other.m_plain_array)
            { }

            array_nd_impl& operator=(const array_nd_impl & other) noexcept(std::is_nothrow_copy_assignable<container_type>::value)
            {
                if (this != &other) {
                    m_plain_array = other.m_plain_array;
                }
                return *this;
            }

            array_nd_impl(array_nd_impl && other) noexcept(std::is_nothrow_move_constructible<container_type>::value)
                : m_plain_array(std::move(other))
            { }

            array_nd_impl& operator=(array_nd_impl && other) noexcept(std::is_nothrow_move_assignable<container_type>::value)
            {
                if (this != &other) {
                    m_plain_array = std::move(other);
                }
                return *this;
            }

            ~array_nd_impl()
            { }

            /**
             *	Swap arrays data
             */
            void swap(array_nd_impl & other) noexcept {
                m_plain_array.swap(other.m_plain_array);
            }

            /**
             *  Get constant iterator to the begin of the array
             *  Will pass through all elements of the multidimensional array
             */
            constexpr const_iterator cbegin() const noexcept {
                return std::cbegin(m_plain_array);
            }
            /**
            *  Get iterator to the begin of the array
            *  Will pass through all elements of the multidimensional array
            */
            iterator begin() noexcept {
                return std::begin(m_plain_array);
            }
            /**
             *	Get const iterator to the end of the array
             */
            constexpr const_iterator cend() const noexcept {
                return std::cend(m_plain_array);
            }
            /**
            *	Get iterator to the end of the array
            */
            iterator end() noexcept {
                return std::end(m_plain_array);
            }


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
                    (idx < shape::top_dimension) ? std::next(cbegin(), idx * _HyperShape::total_size) :
                        (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), cbegin())
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
                YATO_ASSERT(idx < shape::top_dimension, "yato::array_nd: out of range!");
#endif
                return sub_array<iterator, _HyperShape>{ std::next(begin(), idx * _HyperShape::total_size) };
            }


            template<typename _HyperShape = typename shape::hyper_shape, typename _FirstIdx, typename... _Tail>
            typename std::enable_if<
                !std::is_same<_HyperShape, null_shape>::value && std::is_convertible<_FirstIdx, size_t>::value,
                decltype(*std::declval<iterator>())>::type
            at(_FirstIdx firstIdx, _Tail... indexes) {
                if (firstIdx >= shape::top_dimension) {
                    throw std::runtime_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx].at(std::forward<_Tail>(indexes)...);
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
                return idx < shape::top_dimension ? *std::next(cbegin(), idx) :
                    (YATO_THROW_ASSERT_EXCEPT("yato::array_nd: out of range!"), *std::cbegin(m_plain_array));
#else
                return *std::next(cbegin(), idx);
#endif
            }

            template <typename _HyperShape = typename shape::hyper_shape>
                typename std::enable_if<std::is_same<_HyperShape, null_shape>::value,
                decltype(*std::declval<iterator>())>::type
            operator[](size_t idx) noexcept(YATO_RELEASE_BOOL)
            {
#if YATO_DEBUG
                YATO_ASSERT(idx < shape::top_dimension, "yato::array_nd: out of range!");
#endif
                return *std::next(begin(), idx);
            }

            template<typename _HyperShape = typename shape::hyper_shape, typename _FirstIdx>
            typename std::enable_if<
                std::is_same<_HyperShape, null_shape>::value && std::is_convertible<_FirstIdx, size_t>::value,
                decltype(*std::declval<iterator>())>::type
            at(_FirstIdx firstIdx) {
                if (firstIdx >= shape::top_dimension) {
                    throw std::runtime_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx];
            }

            /**
             * Get size along one dimension	
             */
            template<size_t _Dimension>
            constexpr size_t size() const noexcept {
                static_assert(_Dimension < shape::dimensions_number, "yato::array_nd: dimension index is out of range!");
                return shape::dimensions[_Dimension];
            }

            /**
             *	Get total size of the array
             */
            constexpr size_t total_size() const noexcept {
                return shape::total_size;
            }

            /**
             *	Get raw pointer to data
             *  Points to valid continuous storage with all elements
             *  The order of elements is same like for native array T[][]..[]
             */
            constexpr const data_type* data() const noexcept {
                //ToDo: fix it later
                static_assert(!std::is_same<data_type, bool>::value, "data() can't be used of bool");
                return &m_plain_array[0];
            }

            /**
             *	Get raw pointer to data
             *  Points to valid continuous storage with all elements
             *  The order of elements is same like for native array T[][]..[]
             */
            data_type* data() noexcept {
                //ToDo: fix it later
                static_assert(!std::is_same<data_type, bool>::value, "data() can't be used of bool");
                return &m_plain_array[0];
            }


            /**
             *	Fill array with constant value
             */
            template<typename _T>
                typename std::enable_if<std::is_convertible<_T, data_type>::value, void>::type 
            fill(const _T& value) noexcept(std::is_nothrow_copy_assignable<_T>::value) {
                std::fill(begin(), end(), value);
            }
        };

        template <typename _T, typename _Enable = void>
        struct is_array_nd 
        {
            static constexpr bool value = false;
        };

        template <typename _T>
        struct is_array_nd<_T, typename std::enable_if<
            std::is_same<_T, array_nd_impl<typename _T::data_type, typename _T::shape, array_storage_on_stack> >::value>::type >
        {
            static constexpr bool value = true;
        };

        template<typename _Value, typename _Shape, typename _Storage>
        void swap(array_nd_impl<_Value, _Shape, _Storage> & one, array_nd_impl<_Value, _Shape, _Storage> & another) {
            one.swap(another);
        }
    }

    using details::is_array_nd;
    using details::swap;

    /**
     *	Create zero initialized array on stack
     */
    template<typename _DataType, size_t _First_Dimension, size_t... _More_Dimensions> 
    using array_nd = details::array_nd_impl < _DataType, details::plain_array_shape<_First_Dimension, _More_Dimensions...>, array_storage_on_stack>;
    /**
    *	Create zero initialized array on heap
    */
    template<typename _DataType, size_t _First_Dimension, size_t... _More_Dimensions>
    using vector_nd = details::array_nd_impl < _DataType, details::plain_array_shape<_First_Dimension, _More_Dimensions...>, array_storage_on_heap>;

}

#endif