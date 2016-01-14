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

    //----------------------------------------------------------
    // Smart pointer traits
    //
    namespace details
    {
        template<typename T, template<typename...> class SmartPtr, typename Enable = void>
        struct is_smart_ptr_impl 
        {
            static constexpr bool value = false;
        };

        template<typename T, template<typename...> class SmartPtr>
        struct is_smart_ptr_impl<T, SmartPtr, typename std::enable_if< std::is_same<typename std::remove_cv<T>::type, SmartPtr< typename T::element_type> >::value >::type > 
        {
            static constexpr bool value = true;
        };
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
        static constexpr bool value = is_shared_ptr<T>::value || is_unique_ptr<T>::value;
    };
    
    //----------------------------------------------------------
    // Iterators traits
    //
    template<typename T, typename Enable = void>
    struct is_iterator
    {
        static constexpr bool value = false;
    };
    
    template<typename T>
    struct is_iterator<T, typename enable<typename std::iterator_traits<T>::iterator_category>::type >
    {
        static constexpr bool value = true;
    };  
}

#endif
