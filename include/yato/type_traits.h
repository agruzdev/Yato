/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016 Alexey Gruzdev
 */

#ifndef _YATO_TYPE_TRAITS_H_
#define _YATO_TYPE_TRAITS_H_

#include <array>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

#include "prerequisites.h"
#include "meta.h"

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
    struct test_type
    {
        using type = void;
    };

    //----------------------------------------------------------
    // Smart pointer traits
    //
    namespace details
    {
        template<typename _T, template <typename...> class _PtrType, typename _Enable = void>
        struct is_smart_ptr_impl 
            : std::false_type
        { };

        template<typename _T, template <typename...> class _PtrType, typename... _Args>
        struct is_smart_ptr_impl<_PtrType<_T, _Args...>, _PtrType, void>
            : std::true_type
        { };
    }
    
    /**
     * Detect shared_ptr
     * is_shared_ptr<T>::value is true when T is shared_ptr<V> for some V
     */
    template<typename _T>
    using is_shared_ptr = details::is_smart_ptr_impl<_T, std::shared_ptr>;
    
    /**
     * Detect unique_ptr
     * is_unique_ptr<T>::value is true when T is is_unique_ptr<V> for some V
     */
    template<typename _T>
    using is_unique_ptr = details::is_smart_ptr_impl<_T, std::unique_ptr>;
    
    /**
     * Detect weak_ptr
     * is_weak_ptr<T>::value is true when T is is_weak_ptr<V> for some V
     */
    template<typename _T>
    using is_weak_ptr = details::is_smart_ptr_impl<_T, std::weak_ptr>;
    
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
    struct is_iterator<_T, typename test_type<
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


    /**
     *  Check if type T is one of listed types
     */
    template<typename _T, typename _Some, typename... _Others>
    struct one_of
        : std::integral_constant<bool, std::is_same<_T, _Some>::value || one_of<_T, _Others...>::value >
    { };
    
    template<typename _T, typename _Some>
    struct one_of<_T, _Some>
        : std::integral_constant<bool, std::is_same<_T, _Some>::value >
    { };


    /**
     *  Check if types have a trait
     */
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
    struct args_length
        : std::integral_constant<size_t, args_length<_Tail...>::value + 1>
    { };

    template<typename _T>
    struct args_length<_T>
        : std::integral_constant<size_t, 1>
    { };
#else
    template<typename ..._T>
    struct args_length
        : std::integral_constant<size_t, sizeof...(_T)>
    { };
#endif


    //-------------------------------------------------------
    // functional traits

    template <typename... T>
    struct is_function_pointer
        : std::false_type
    { };

    template <typename Ret, typename... Args>
    struct is_function_pointer<Ret(*)(Args...)>
        : std::true_type
    { };

    template <typename Ret, typename Clazz, typename... Args>
    struct is_function_pointer<Ret(Clazz::*)(Args...)>
        : std::true_type
    { };

    template <typename Ret, typename Clazz, typename ...Args>
    struct is_function_pointer<Ret(Clazz::*)(Args...) const>
        : std::true_type
    { };

    template <typename Ret, typename Clazz, typename ...Args>
    struct is_function_pointer<Ret(Clazz::*)(Args...) volatile>
        : std::true_type
    { };

    template <typename Ret, typename Clazz, typename ...Args>
    struct is_function_pointer<Ret(Clazz::*)(Args...) const volatile>
        : std::true_type
    { };

#ifdef YATO_MSVC_2013
    /**
     *  For some reasons MSVC 2013 can't work with expression 'decltype(&_T::operator())' correctly
     *  Here is some workaround with generally incorrect expression, but it works fine for MSVC 2013
     */
    namespace details
    {
        template <typename> 
        struct sfinae_true 
            : std::true_type
        { };

        template <typename T> 
        static auto vc12_test_operator_round_brackets(int)
            -> sfinae_true<decltype(std::declval<T>().operator())>;

        template <typename> 
        static auto vc12_test_operator_round_brackets(long)
            -> std::false_type;
    }

    template <typename T> 
    struct has_operator_round_brackets 
        : decltype(details::vc12_test_operator_round_brackets<T>(0))
    { };

#else
    template <typename _T, typename _Enable = void>
    struct has_operator_round_brackets
        : std::false_type
    { };

    template <typename _T>
    struct has_operator_round_brackets <_T, typename test_type< decltype(&_T::operator()) >::type >
        : std::true_type
    { };
#endif

    template <typename _T>
    struct is_callable
        : std::integral_constant<bool, has_operator_round_brackets<_T>::value || is_function_pointer<_T>::value>
    { };

    /**
     *  Converts function member type to function type
     */

    namespace details
    {
        template <typename _T>
        struct remove_class_impl {};

        template <typename _Class, typename _Result, typename... _Args>
        struct remove_class_impl<_Result(_Class::*)(_Args...)>
        {
            using type = _Result(*)(_Args...);
        };

        template <typename _Class, typename _Result, typename... _Args>
        struct remove_class_impl<_Result(_Class::*)(_Args...) const>
        {
            using type = _Result(*)(_Args...);
        };

        template <typename _Class, typename _Result, typename... _Args>
        struct remove_class_impl<_Result(_Class::*)(_Args...) volatile>
        {
            using type = _Result(*)(_Args...);
        };

        template <typename _Class, typename _Result, typename... _Args>
        struct remove_class_impl<_Result(_Class::*)(_Args...) const volatile>
        {
            using type = _Result(*)(_Args...);
        };

        template <typename _Result, typename... _Args>
        struct remove_class_impl<_Result(*)(_Args...)>
        {
            using type = _Result(*)(_Args...);
        };
    }

    template <typename _T> 
    using remove_class = details::remove_class_impl< typename std::remove_cv<_T>::type >;

    namespace details
    {
        template<typename T>
        struct function_trait {};

        template<typename _R, typename... _Args>
        struct function_trait< std::function<_R(_Args...)> >
        {
            static YATO_CONSTEXPR_VAR size_t arguments_num = sizeof...(_Args);

            using result_type = _R;
            using arguments_list = typename meta::make_list<_Args...>::type;

            template <size_t _Idx>
            struct arg
            {
                using type = typename meta::list_at<arguments_list, _Idx>::type;
            };

            using pointer_type = _R(*)(_Args...);
            using function_type = std::function<_R(_Args...)>;
        };

        template<typename _R, typename... _Args>
        struct function_trait< _R(*)(_Args...) >
        {
            static YATO_CONSTEXPR_VAR size_t arguments_num = sizeof...(_Args);

            using result_type = _R;
            using arguments_list = typename meta::make_list<_Args...>::type;

            template <size_t _Idx>
            struct arg
            {
                using type = typename meta::list_at<arguments_list, _Idx>::type;
            };

            using pointer_type = _R(*)(_Args...);
            using function_type = std::function<_R(_Args...)>;
        };

        /**
        *  For class members
        */
        template<typename T>
        struct function_member_trait {};

        template<class _Class, typename _R, typename... _Args>
        struct function_member_trait< _R( _Class::* )(_Args...) >
        {
            static YATO_CONSTEXPR_VAR size_t arguments_num = sizeof...(_Args);

            using my_class = _Class;
            using result_type = _R;
            using arguments_list = typename meta::make_list<_Args...>::type;

            template <size_t _Idx>
            struct arg
            {
                using type = typename meta::list_at<arguments_list, _Idx>::type;
            };

            using pointer_type = _R(*)(_Args...);
            using function_type = std::function<_R(_Args...)>;
        };
    }

    /**
     *  Deduce information about arguments and return type for a callable entity
     */
    template <typename _T, typename _Enable = void>
    struct callable_trait
    { };

    template <typename _T>
    struct callable_trait <_T, typename std::enable_if<is_function_pointer<_T>::value>::type>
        : details::function_trait< typename remove_class<_T>::type >
    { };

    template <typename _T>
    struct callable_trait <_T, typename std::enable_if<has_operator_round_brackets<_T>::value>::type>
        : details::function_trait< typename remove_class<decltype(&_T::operator())>::type >
    { };

    /**
     *  Make std::function from a callable entity
     */
    template <typename _Callable>
    auto make_function(_Callable && callable)
        -> typename callable_trait<typename std::remove_reference<_Callable>::type>::function_type
    {
        return typename callable_trait<typename std::remove_reference<_Callable>::type>::function_type(std::forward<_Callable>(callable));
    }


    /**
     * Maps any type to destination type
     */
    template <typename TypeFrom, typename TypeTo>
    struct cvt_type
    {
        using type = TypeTo;
    };



#ifndef YATO_MSVC_2013
    /**
     * Get the narrowest type to store a value
     */
    namespace details
    {
        template <typename DstType, typename ValueType, ValueType Value>
        struct fits_type
            : std::integral_constant<bool, 
                (Value >= static_cast<ValueType>(std::numeric_limits<DstType>::min())) && 
                (Value <= static_cast<ValueType>(std::numeric_limits<DstType>::max()))>
        { };

        template <typename Tag, typename ValueType, ValueType Value, size_t Size, typename = void>
        struct narrowest_fit_impl
        {
            using type = typename std::conditional< fits_type<typename make_type<Tag, Size>::type, ValueType, Value>::value,
                    typename make_type<Tag, Size>::type,
                    typename narrowest_fit_impl<Tag, ValueType, Value, 2 * Size>::type
                >::type;
        };

        template <typename Tag, typename ValueType, ValueType Value, size_t Size>
        struct narrowest_fit_impl <Tag, ValueType, Value, Size, typename std::enable_if<
            (Size > 64)
        >::type>
        {
            using type = void;
        };
    }

    template <uint64_t Value>
    using narrowest_fit_unsigned = details::narrowest_fit_impl<unsigned_type_tag, uint64_t, Value, 8>;
    
    template <int64_t Value>
    using narrowest_fit_signed = details::narrowest_fit_impl<signed_type_tag, int64_t, Value, 8>;

    template <uint64_t Value>
    using narrowest_fit_unsigned_t = typename narrowest_fit_unsigned<Value>::type;

    template <int64_t Value>
    using narrowest_fit_signed_t = typename narrowest_fit_signed<Value>::type;
#endif
}

#endif
