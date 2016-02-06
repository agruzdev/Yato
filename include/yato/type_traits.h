/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016 Alexey Gruzdev
 */

#ifndef _YATO_TYPE_TRAITS_H_
#define _YATO_TYPE_TRAITS_H_

#include <type_traits>
#include <memory>
#include <iterator>

#include "prerequisites.h"

namespace yato
{
    //----------------------------------------------------------
    // Helper class for SFINAE; Unconditional version of std::enable_if
    //
    template <typename T, typename U = void>
    struct enable 
    {
        using type = U;
    };


    template <typename...>
    struct test_param
    {
        using type = void;
    };

    //----------------------------------------------------------
    // Smart pointer traits
    //
    namespace details
    {
        template<typename T, template<typename...> class SmartPtr, typename Enable = void>
        struct is_smart_ptr_impl 
            : std::false_type
        { };

        template<typename T, template<typename...> class SmartPtr>
        struct is_smart_ptr_impl<T, SmartPtr, typename std::enable_if< std::is_same<typename std::remove_cv<T>::type, SmartPtr< typename T::element_type> >::value >::type > 
            : std::true_type
        { };
    }
    
    /**
     * Detect shared_ptr
     * is_shared_ptr<T>::value is true when T is shared_ptr<V> for some V
     */
    template<typename T>
    using is_shared_ptr = details::is_smart_ptr_impl<T, std::shared_ptr>;
    
    /**
     * Detect unique_ptr
     * is_unique_ptr<T>::value is true when T is is_unique_ptr<V> for some V
     */
    template<typename T>
    using is_unique_ptr = details::is_smart_ptr_impl<T, std::unique_ptr>;
    
    /**
     * Detect shared_ptr or unique_ptr
     * auto_ptr is evil - don't use it
     * weak_ptr has a little bit different behavior... so I dont want to mix it with others
     */
    template<typename T>
    struct is_smart_ptr 
    {
        static YATO_CONSTEXPR_VAR bool value = is_shared_ptr<T>::value || is_unique_ptr<T>::value;
    };
    
    //----------------------------------------------------------
    // Iterators traits
    //

    template<class, class = void>
    struct is_iterator
        : std::false_type
    {};

    template<class _T>
    struct is_iterator<_T, typename test_param<
        typename _T::iterator_category,
        typename _T::value_type,
        typename _T::difference_type,
        typename _T::pointer,
        typename _T::reference
    >::type> 
        : std::true_type
    { };

    template<class _T>
    struct is_iterator <_T, typename std::enable_if<
        std::is_pointer<typename std::remove_const<_T>::type>::value>::type
    > 
        : std::true_type
    { };


    //-------------------------------------------------------
    //

    template<typename _T1, typename _T2, typename... _T>
    struct is_same
    {
        static YATO_CONSTEXPR_VAR bool value = std::is_same<_T1, _T2>::value && yato::is_same<_T2, _T...>::value;
    };

    template<typename _T1, typename _T2>
    struct is_same<_T1, _T2>
    {
        static YATO_CONSTEXPR_VAR bool value = std::is_same<_T1, _T2>::value;
    };


    template<template <typename> class _Trait, typename _T1, typename... _T>
    struct has_trait
    {
        static YATO_CONSTEXPR_VAR bool value = _Trait<_T1>::value && has_trait<_Trait, _T...>::value;
    };

    template<template <typename> class _Trait, typename _T>
    struct has_trait<_Trait, _T>
    {
        static YATO_CONSTEXPR_VAR bool value = _Trait<_T>::value;
    };

    /**
     * Return length of arguments pack
     * Equal to sizeof...() but sizeof...() inside of std::enable_if behaves incorrect in MSVC 2013
     */
#ifdef YATO_MSVC_2013
    template<typename _T, typename... _Tail>
    struct length
        : std::integral_constant<size_t, length<_Tail...>::value + 1>
    { };

    template<typename _T>
    struct length<_T>
        : std::integral_constant<size_t, 1>
    { };
#else
    template<typename ..._T>
    struct length
        : std::integral_constant<size_t, sizeof...(_T)>
    { };
#endif
}

#endif
