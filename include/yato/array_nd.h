/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_ND_H_
#define _YATO_ARRAY_ND_H_

#include <array>
#include <algorithm>
#include <type_traits>
#include <vector>
#include "assertion.h"
#include "array_view.h"
#include "iterator_nd.h"
#include "range.h"

namespace yato
{

    namespace details
    {
        struct null_shape {};

YATO_PRAGMA_WARNING_PUSH
#if YATO_MSVC == YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4268)
#endif
        template <size_t DimHead_, size_t... DimTail_>
        struct plain_array_shape
        {
            static_assert(DimHead_ > 0, "The each array dimension should be greater than 0");

            using sub_shape = plain_array_shape<DimTail_...>;
            static YATO_CONSTEXPR_VAR size_t total_size = DimHead_ * sub_shape::total_size;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = sizeof...(DimTail_) + 1;
            static YATO_CONSTEXPR_VAR size_t top_dimension = DimHead_;

            static YATO_CONSTEXPR_VAR std::array<size_t, sizeof...(DimTail_) + 1> extents = { DimHead_, DimTail_... };
        };

        template <size_t DimHead_>
        struct plain_array_shape<DimHead_> 
        {
            static_assert(DimHead_ > 0, "The each array dimension should be greater than 0");

            using sub_shape = null_shape;
            static YATO_CONSTEXPR_VAR size_t total_size = DimHead_;
            static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;
            static YATO_CONSTEXPR_VAR size_t top_dimension = DimHead_;

            static YATO_CONSTEXPR_VAR std::array<size_t, 1> extents = { DimHead_ };
        };

        template <typename VTy_, typename Shape_, size_t... Offsets_>
        struct plain_array_strides
            : public plain_array_strides<VTy_, typename Shape_::sub_shape, Offsets_..., Shape_::total_size>
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


        template <typename ValueType_, typename Shape_>
        struct deduce_nd_type
        {
            using type = typename deduce_nd_type<ValueType_, typename Shape_::sub_shape>::type[Shape_::top_dimension];
        };

        template <typename ValueType_>
        struct deduce_nd_type<ValueType_, null_shape>
        {
            using type = ValueType_;
        };


        template<typename ValueType_, typename Shape_, proxy_access_policy AccessPolicy_ = proxy_access_policy::lvalue_ref>
        class sub_array
        {
            static_assert(!std::is_reference<ValueType_>::value, "ValueType can't be reference");
        public:
            using value_type = ValueType_;
            using shape      = Shape_;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = Shape_::dimensions_number;
            static YATO_CONSTEXPR_VAR proxy_access_policy access_policy = AccessPolicy_;

            using value_reference       = typename proxy_access_traits<value_type, access_policy>::reference;
            using const_value_reference = typename proxy_access_traits<value_type, access_policy>::const_reference;

            using sub_proxy       = sub_array<value_type, typename shape::sub_shape, access_policy>;
            using const_sub_proxy = sub_array<typename proxy_access_traits<value_type, access_policy>::const_value_type, typename shape::sub_shape, access_policy>;

            using iterator       = iterator_nd<sub_proxy>;
            using const_iterator = iterator_nd<const_sub_proxy>;

            using reference       = sub_proxy;
            using const_reference = const_sub_proxy;

            using plain_iterator       = typename proxy_access_traits<value_type, access_policy>::plain_iterator;
            using const_plain_iterator = typename proxy_access_traits<value_type, access_policy>::const_plain_iterator;

        private:
            value_type* m_iter;

            const_sub_proxy create_cproxy_(size_t idx) const
            {
                return const_sub_proxy{ std::next(m_iter, idx * shape::sub_shape::total_size) };
            }

            sub_proxy create_proxy_(size_t idx) const
            {
                return sub_proxy{ std::next(m_iter, idx * shape::sub_shape::total_size) };
            }

        public:
            YATO_CONSTEXPR_FUNC
            explicit sub_array(std::add_pointer_t<ValueType_> iter) YATO_NOEXCEPT_KEYWORD
                : m_iter(iter)
            { }

            YATO_CONSTEXPR_FUNC
            sub_array(const sub_array & other) = default;
            sub_array(sub_array && other) noexcept = default;

            sub_array& operator=(const sub_array & other) = default;
            sub_array& operator=(sub_array && other) noexcept = default;

            ~sub_array() = default;

            YATO_CONSTEXPR_FUNC_CXX14
            reference operator[](size_t idx) const
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return create_proxy_(idx);
            }

            template <typename... Tail_>
            value_reference at(size_t firstIdx, Tail_ &&... indexes)
            {
                if (firstIdx >= shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx].at(std::forward<Tail_>(indexes)...);
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
             * Get size along 0 dimension
             */
            YATO_CONSTEXPR_FUNC
            size_t size() const YATO_NOEXCEPT_KEYWORD
            {
                return shape::top_dimension;
            }

            YATO_CONSTEXPR_FUNC
            size_t total_size() const
            {
                return shape::total_size; 
            }

            YATO_CONSTEXPR_FUNC
            size_t total_stored() const
            {
                return shape::total_size * sizeof(value_type); 
            }

            YATO_CONSTEXPR_FUNC
            bool continuous() const
            {
                return true;
            }

            YATO_CONSTEXPR_FUNC
            const_iterator cbegin() const
            {
                return static_cast<const_iterator>(create_cproxy_(0));
            }

            YATO_CONSTEXPR_FUNC
            const_iterator begin() const
            {
                return static_cast<const_iterator>(create_cproxy_(0));
            }

            YATO_CONSTEXPR_FUNC
            const_iterator cend() const
            {
                return static_cast<const_iterator>(create_cproxy_(shape::top_dimension));
            }

            YATO_CONSTEXPR_FUNC
            const_iterator end() const
            {
                return static_cast<const_iterator>(create_cproxy_(shape::top_dimension));
            }

            YATO_CONSTEXPR_FUNC
            iterator begin()
            {
                return static_cast<iterator>(create_proxy_(0));
            }

            YATO_CONSTEXPR_FUNC
            iterator end()
            {
                return static_cast<iterator>(create_proxy_(shape::top_dimension));
            }

            auto range()
            {
                return yato::make_range(begin(), end());
            }

YATO_PRAGMA_WARNING_PUSH
#if YATO_MSVC == YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4100)
#endif
            YATO_CONSTEXPR_FUNC
            auto crange() const
            {
                return yato::make_range(cbegin(), cend());
            }
YATO_PRAGMA_WARNING_POP

            YATO_CONSTEXPR_FUNC
            const_plain_iterator plain_cbegin() const
            {
                return m_iter;
            }

            YATO_CONSTEXPR_FUNC
            const_plain_iterator plain_cend() const
            {
                return std::next(m_iter, shape::total_size);
            }

            plain_iterator plain_begin() const
            {
                return m_iter;
            }

            plain_iterator plain_end() const
            {
                return std::next(m_iter, shape::total_size);
            }

            auto plain_range()
            {
                return yato::make_range(plain_begin(), plain_end());
            }

YATO_PRAGMA_WARNING_PUSH
#if YATO_MSVC == YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4100)
#endif
            YATO_CONSTEXPR_FUNC
            auto plain_crange() const
            {
                return yato::make_range(plain_cbegin(), plain_cend());
            }
YATO_PRAGMA_WARNING_POP

            value_type* data()
            {
                return m_iter;
            }

            std::add_const_t<value_type>* cdata() const
            {
                return m_iter;
            }

            // private interface
            std::add_pointer_t<ValueType_> & raw_ptr_()
            {
                return m_iter;
            }

            // private interface
            std::add_const_t<std::add_pointer_t<ValueType_>> & raw_ptr_() const
            {
                return m_iter;
            }
        };


        template<typename ValueType_, size_t Length_, proxy_access_policy AccessPolicy_>
        class sub_array<ValueType_, plain_array_shape<Length_>, AccessPolicy_>
        {
            static_assert(!std::is_reference<ValueType_>::value, "ValueType can't be reference");
        public:
            using value_type = ValueType_;
            using shape      = plain_array_shape<Length_>;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = shape::dimensions_number;
            static YATO_CONSTEXPR_VAR proxy_access_policy access_policy = AccessPolicy_;

            using value_reference       = typename proxy_access_traits<value_type, access_policy>::reference;
            using const_value_reference = typename proxy_access_traits<value_type, access_policy>::const_reference;

            using plain_iterator       = typename proxy_access_traits<value_type, access_policy>::plain_iterator;
            using const_plain_iterator = typename proxy_access_traits<value_type, access_policy>::const_plain_iterator;

            using reference         = value_reference;
            using const_reference   = const_value_reference;

            using iterator          = plain_iterator;
            using const_iterator    = const_plain_iterator;

        private:
            value_type* m_iter;

        public:
            YATO_CONSTEXPR_FUNC
            explicit sub_array(value_type* iter) YATO_NOEXCEPT_KEYWORD
                : m_iter(iter)
            { }

            YATO_CONSTEXPR_FUNC
            sub_array(const sub_array & other) = default;
            sub_array(sub_array && other) noexcept = default;

            sub_array& operator=(const sub_array & other) = default;
            sub_array& operator=(sub_array && other) noexcept = default;

            ~sub_array() = default;

            YATO_CONSTEXPR_FUNC_CXX14
            reference operator[](size_t idx) const
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return *std::next(m_iter, idx);
            }

            value_reference at(size_t idx) const
            {
                if (idx >= shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[idx];
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
             * Get size along 0 dimension
             */
            YATO_CONSTEXPR_FUNC
            size_t size() const YATO_NOEXCEPT_KEYWORD
            {
                return shape::top_dimension;
            }

            /**
             * Total elements number
             */
            YATO_CONSTEXPR_FUNC
            size_t total_size() const
            {
                return shape::total_size;
            }

            /**
             * Total bytes number
             */
            YATO_CONSTEXPR_FUNC
            size_t total_stored() const
            {
                return shape::total_size * sizeof(value_type);
            }

            YATO_CONSTEXPR_FUNC
            bool continuous() const
            {
                return true;
            }

            YATO_CONSTEXPR_FUNC
            const_iterator cbegin() const
            {
                return plain_cbegin();
            }

            YATO_CONSTEXPR_FUNC
            const_iterator begin() const
            {
                return plain_cbegin();
            }

            YATO_CONSTEXPR_FUNC
            const_iterator cend() const
            {
                return plain_cend();
            }

            YATO_CONSTEXPR_FUNC
            const_iterator end() const
            {
                return plain_cend();
            }

            YATO_CONSTEXPR_FUNC
            iterator begin()
            {
                return plain_begin();
            }

            YATO_CONSTEXPR_FUNC
            iterator end()
            {
                return plain_end();
            }

            YATO_CONSTEXPR_FUNC
            auto range() const
            {
                return plain_range();
            }

YATO_PRAGMA_WARNING_PUSH
#if YATO_MSVC == YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4100)
#endif
            YATO_CONSTEXPR_FUNC
            auto crange() const
            {
                return plain_crange();
            }
YATO_PRAGMA_WARNING_POP

            YATO_CONSTEXPR_FUNC
            const_plain_iterator plain_cbegin() const
            {
                return m_iter;
            }

            YATO_CONSTEXPR_FUNC
            const_plain_iterator plain_cend() const
            {
                return std::next(m_iter, shape::total_size);
            }

            plain_iterator plain_begin() const
            {
                return m_iter;
            }

            plain_iterator plain_end() const
            {
                return std::next(m_iter, shape::total_size);
            }

            auto plain_range() const
            {
                return yato::make_range(plain_begin(), plain_end());
            }

YATO_PRAGMA_WARNING_PUSH
#if YATO_MSVC == YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4100)
#endif
            YATO_CONSTEXPR_FUNC
            auto plain_crange() const
            {
                return yato::make_range(plain_cbegin(), plain_cend());
            }
YATO_PRAGMA_WARNING_POP

            value_type* data()
            {
                return m_iter;
            }

            std::add_const_t<value_type>* cdata() const
            {
                return m_iter;
            }

            // private interface
            std::add_pointer_t<ValueType_> & raw_ptr_()
            {
                return m_iter;
            }

            // private interface
            std::add_const_t<std::add_pointer_t<ValueType_>> & raw_ptr_() const
            {
                return m_iter;
            }
        };



        template<typename ValueType_, typename Shape_>
        class array_nd_impl
        {
        public:
            using value_type   = ValueType_;
            using size_type    = size_t;
            using pointer_type = std::add_pointer<value_type>;
            using shape        = Shape_;
            using sub_shape    = typename Shape_::sub_shape;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = shape::dimensions_number;

            using value_reference       = std::add_lvalue_reference_t<value_type>;
            using const_value_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;

            using sub_proxy       = sub_array<value_type, sub_shape>;
            using const_sub_proxy = sub_array<std::add_const_t<value_type>, sub_shape>;

            using reference       = sub_proxy;
            using const_reference = const_sub_proxy;

            using iterator        = iterator_nd<sub_proxy>;
            using const_iterator  = iterator_nd<const_sub_proxy>;

            /**
             * Plain Iterator allowing to pass through all elements of the multidimensional array 
             */
            using plain_iterator       = std::add_pointer_t<value_type>;
            using const_plain_iterator = std::add_pointer_t<std::add_const_t<value_type>>;
            //-------------------------------------------------------

        private:
            value_type* ptr_()
            {
                return yato::pointer_cast<value_type*>(&m_array);
            }

            const value_type* cptr_() const
            {
                return yato::pointer_cast<const value_type*>(&m_array);
            }

            sub_proxy create_proxy_(size_t idx)
            {
                return sub_proxy{ ptr_() + idx * sub_shape::total_size };
            }

            const_sub_proxy create_cproxy_(size_t idx) const
            {
                return const_sub_proxy{ cptr_() + idx * sub_shape::total_size };
            }

        public:
            /**
             * Public internal array for aggregate initilization
             */
            typename deduce_nd_type<value_type, shape>::type m_array;

            /**
             *	Swap arrays data
             */
            void swap(array_nd_impl & other)
#ifdef YATO_CXX17
                YATO_NOEXCEPT_OPERATOR(std::is_nothrow_swappable<value_type>::value)
#endif
            {
                value_type* it1  = ptr_();
                value_type* it2  = other.ptr_();
                value_type* const end1 = ptr_() + total_size();
                while(it1 != end1) {
                    std::iter_swap(it1++, it2++);
                }
            }

            YATO_CONSTEXPR_FUNC
            const_iterator cbegin() const
            {
                return static_cast<const_iterator>(create_cproxy_(0));
            }

            YATO_CONSTEXPR_FUNC
            const_iterator begin() const
            {
                return static_cast<const_iterator>(create_cproxy_(0));
            }

            YATO_CONSTEXPR_FUNC
            iterator begin()
            {
                return static_cast<iterator>(create_proxy_(0));
            }

            YATO_CONSTEXPR_FUNC
            const_iterator cend() const
            {
                return static_cast<const_iterator>(create_cproxy_(shape::top_dimension));
            }

            YATO_CONSTEXPR_FUNC
            const_iterator end() const
            {
                return static_cast<const_iterator>(create_cproxy_(shape::top_dimension));
            }

            YATO_CONSTEXPR_FUNC
            iterator end()
            {
                return static_cast<iterator>(create_proxy_(shape::top_dimension));
            }

YATO_PRAGMA_WARNING_PUSH
#if YATO_MSVC == YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4100)
#endif
            YATO_CONSTEXPR_FUNC
            auto crange() const
            {
                return yato::make_range(cbegin(), cend());
            }
YATO_PRAGMA_WARNING_POP

            auto range()
            {
                return yato::make_range(begin(), end());
            }

            /**
             *  Get constant iterator to the begin of the array
             *  Will pass through all elements of the multidimensional array
             */
            YATO_CONSTEXPR_FUNC
            const_plain_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD 
            {
                return cptr_();
            }

            /**
            *  Get iterator to the begin of the array
            *  Will pass through all elements of the multidimensional array
            */
            plain_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return ptr_();
            }

            /**
             *  Get const iterator to the end of the array
             */
            YATO_CONSTEXPR_FUNC 
            const_plain_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD 
            {
                return cptr_() + total_size();
            }

            /**
            *  Get iterator to the end of the array
            */
            plain_iterator plain_end() YATO_NOEXCEPT_KEYWORD
            {
                return ptr_() + total_size();
            }

            auto plain_range()
            {
                return yato::make_range(plain_begin(), plain_end());
            }

YATO_PRAGMA_WARNING_PUSH
#if YATO_MSVC == YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4100)
#endif
            YATO_CONSTEXPR_FUNC
            auto plain_crange() const
            {
                return yato::make_range(plain_cbegin(), plain_cend());
            }
YATO_PRAGMA_WARNING_POP

            YATO_CONSTEXPR_FUNC_CXX14
            const_reference operator[](size_t idx) const 
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return create_cproxy_(idx);
            }

            reference operator[](size_t idx)
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return create_proxy_(idx);
            }

            template <typename... Tail_>
            const_value_reference at(size_t firstIdx, Tail_ &&... indexes) const
            {
                if (firstIdx >= shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx].at(std::forward<Tail_>(indexes)...);
            }

            template <typename... Tail_>
            value_reference at(size_t firstIdx, Tail_ &&... indexes)
            {
                if (firstIdx >= shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[firstIdx].at(std::forward<Tail_>(indexes)...);
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
             * Get size along 0 dimension
             */
            YATO_CONSTEXPR_FUNC
            size_t size() const YATO_NOEXCEPT_KEYWORD
            {
                return shape::top_dimension;
            }

            /**
             * Get total size of the array
             */
            YATO_CONSTEXPR_FUNC
            size_t total_size() const YATO_NOEXCEPT_KEYWORD 
            {
                return shape::total_size;
            }

            /**
             * Total bytes number
             */
            YATO_CONSTEXPR_FUNC
            size_t total_stored() const
            {
                return shape::total_size * sizeof(value_type); 
            }

            /**
             * array_nd is always continuous
             */
            YATO_CONSTEXPR_FUNC
            bool continuous() const
            {
                return true;
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
            value_type* data() YATO_NOEXCEPT_KEYWORD 
            {
                return ptr_();
            }

            /**
             *  Get raw pointer to data
             *  Points to valid continuous storage with all elements
             *  The order of elements is same like for native array T[][]..[]
             */
            YATO_CONSTEXPR_FUNC 
            std::add_const_t<value_type>* cdata() const YATO_NOEXCEPT_KEYWORD
            {
                return cptr_();
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

        // 1D case
        template<typename ValueType_, size_t Length_>
        class array_nd_impl<ValueType_, plain_array_shape<Length_>>
        {
        public:
            using value_type   = ValueType_;
            using size_type    = size_t;
            using pointer_type = std::add_pointer<value_type>;
            using shape        = plain_array_shape<Length_>;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = shape::dimensions_number;

            using value_reference       = std::add_lvalue_reference_t<value_type>;
            using const_value_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;

            using plain_iterator       = std::add_pointer_t<value_type>;
            using const_plain_iterator = std::add_pointer_t<std::add_const_t<value_type>>;

            using reference         = value_reference;
            using const_reference   = const_value_reference;

            using iterator          = plain_iterator;
            using const_iterator    = const_plain_iterator;
            //-------------------------------------------------------

        private:
            value_type* ptr_()
            {
                return yato::pointer_cast<value_type*>(&m_array);
            }

            const value_type* cptr_() const
            {
                return yato::pointer_cast<const value_type*>(&m_array);
            }


        public:
            /**
             * Public internal array for aggregate initilization
             */
            typename deduce_nd_type<value_type, shape>::type m_array;

            /**
             *	Swap arrays data
             */
            void swap(array_nd_impl & other)
#ifdef YATO_CXX17
                YATO_NOEXCEPT_OPERATOR(std::is_nothrow_swappable<value_type>::value)
#endif
            {
                value_type* it1  = ptr_();
                value_type* it2  = other.ptr_();
                value_type* const end1 = ptr_() + total_size();
                while(it1 != end1) {
                    std::iter_swap(it1++, it2++);
                }
            }

            /**
             *  Get constant iterator to the begin of the array
             *  Will pass through all elements of the multidimensional array
             */
            YATO_CONSTEXPR_FUNC
            const_plain_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD 
            {
                return cptr_();
            }

            /**
            *  Get iterator to the begin of the array
            *  Will pass through all elements of the multidimensional array
            */
            plain_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return ptr_();
            }

            /**
             *  Get const iterator to the end of the array
             */
            YATO_CONSTEXPR_FUNC 
            const_plain_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD 
            {
                return cptr_() + total_size();
            }

            /**
            *  Get iterator to the end of the array
            */
            plain_iterator plain_end() YATO_NOEXCEPT_KEYWORD
            {
                return ptr_() + total_size();
            }

            auto plain_range()
            {
                return yato::make_range(plain_begin(), plain_end());
            }

YATO_PRAGMA_WARNING_PUSH
#if YATO_MSVC == YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4100)
#endif
            YATO_CONSTEXPR_FUNC
            auto plain_crange() const
            {
                return yato::make_range(plain_cbegin(), plain_cend());
            }
YATO_PRAGMA_WARNING_POP


            YATO_CONSTEXPR_FUNC
            const_iterator cbegin() const
            {
                return plain_cbegin();
            }

            YATO_CONSTEXPR_FUNC
            const_iterator begin() const
            {
                return plain_cbegin();
            }

            YATO_CONSTEXPR_FUNC
            iterator begin()
            {
                return plain_begin();
            }

            YATO_CONSTEXPR_FUNC
            const_iterator cend() const
            {
                return plain_cend();
            }

            YATO_CONSTEXPR_FUNC
            const_iterator end() const
            {
                return plain_cend();
            }

            YATO_CONSTEXPR_FUNC
            iterator end()
            {
                return plain_end();
            }

YATO_PRAGMA_WARNING_PUSH
#if YATO_MSVC == YATO_MSVC_2015
 YATO_MSCV_WARNING_IGNORE(4100)
#endif
            YATO_CONSTEXPR_FUNC
            auto crange() const
            {
                return yato::make_range(cbegin(), cend());
            }
YATO_PRAGMA_WARNING_POP

            auto range()
            {
                return yato::make_range(begin(), end());
            }


            YATO_CONSTEXPR_FUNC_CXX14
            const_reference operator[](size_t idx) const 
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return m_array[idx];
            }

            YATO_CONSTEXPR_FUNC_CXX14
            reference operator[](size_t idx)
            {
                YATO_REQUIRES(idx < shape::top_dimension);
                return m_array[idx];
            }

            const_value_reference at(size_t idx) const
            {
                if (idx >= shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return m_array[idx];
            }

            value_reference at(size_t idx)
            {
                if (idx >= shape::top_dimension) {
                    throw yato::out_of_range_error("yato::array_nd[at]: index is out of range!");
                }
                return (*this)[idx];
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
             * Get size along 0 dimension
             */
            YATO_CONSTEXPR_FUNC
            size_t size() const YATO_NOEXCEPT_KEYWORD
            {
                return shape::top_dimension;
            }

            /**
             * Get total size of the array
             */
            YATO_CONSTEXPR_FUNC
            size_t total_size() const YATO_NOEXCEPT_KEYWORD 
            {
                return shape::total_size;
            }

            /**
             * Total bytes number
             */
            YATO_CONSTEXPR_FUNC
            size_t total_stored() const
            {
                return shape::total_size * sizeof(value_type); 
            }

            /**
             * array_nd is always continuous
             */
            YATO_CONSTEXPR_FUNC
            bool continuous() const
            {
                return true;
            }

            /**
             * Get byte offset till next sub-view
             * Returns size in bytes for 1D view
             */
            YATO_CONSTEXPR_FUNC_CXX14
            size_type stride(size_t idx) const
            {
                YATO_MAYBE_UNUSED(idx);
                YATO_REQUIRES(false && "no strides for 1D");
                return 0;
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
            value_type* data() YATO_NOEXCEPT_KEYWORD 
            {
                return ptr_();
            }

            /**
             *  Get raw pointer to data
             *  Points to valid continuous storage with all elements
             *  The order of elements is same like for native array T[][]..[]
             */
            YATO_CONSTEXPR_FUNC 
            const value_type* cdata() const YATO_NOEXCEPT_KEYWORD
            {
                return cptr_();
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
    auto cview(const details::array_nd_impl<Ty_, Shape_> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view_nd<std::add_const_t<Ty_>, Shape_::dimensions_number>(arr.cdata(), arr.dimensions(), arr.strides());
    }

    template <typename Ty_, typename Shape_>
    YATO_CONSTEXPR_FUNC
    auto view(details::array_nd_impl<Ty_, Shape_> & arr) YATO_NOEXCEPT_KEYWORD
    {
        return array_view_nd<Ty_, Shape_::dimensions_number>(arr.data(), arr.dimensions(), arr.strides());
    }
}

#endif
