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

    template<class _Ty>
    struct is_iterator<_Ty, typename test_param<
        typename _Ty::iterator_category,
        typename _Ty::value_type,
        typename _Ty::difference_type,
        typename _Ty::pointer,
        typename _Ty::reference
    >::type> 
        : std::true_type
    { };
}

#endif
