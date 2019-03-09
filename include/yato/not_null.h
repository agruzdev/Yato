/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_NOT_NULL_
#define _YATO_NOT_NULL_

#include "assertion.h"
#include "type_traits.h"

namespace yato
{ 
    /**
     * not_null wrapper forcing function argument to be not nullptr
     */
    template<typename T, typename Enable = void>
    class not_null
    {};
    
    /**
     * Implementation for raw pointers
     */
    template<typename T>
    class not_null<T, typename std::enable_if< std::is_pointer<T>::value >::type >
    {    
        using element_type = typename std::remove_reference<decltype(*(std::declval<T>()))>::type;
        T m_pointer;

    public:
#if !(defined(YATO_ANDROID) && __cplusplus < 201400L)
        YATO_CONSTEXPR_FUNC 
#endif
        not_null(T ptr)
            : m_pointer(ptr) 
        {
            YATO_REQUIRES(ptr != nullptr);
        }
        
        not_null(std::nullptr_t ptr) = delete;
        
        not_null(const not_null&) = default;
        not_null& operator=(const not_null&) = default;

        ~not_null() = default;

        YATO_CONSTEXPR_FUNC 
        operator T () const YATO_NOEXCEPT_KEYWORD
        {
            return m_pointer;
        }
        
        YATO_CONSTEXPR_FUNC 
        T get() const YATO_NOEXCEPT_KEYWORD
        {
            return m_pointer;
        }
        
        YATO_CONSTEXPR_FUNC 
        T operator->() const YATO_NOEXCEPT_KEYWORD
        {
            return m_pointer;
        }
        
        YATO_CONSTEXPR_FUNC 
        const element_type & operator*() const YATO_NOEXCEPT_KEYWORD
        {
            return *m_pointer;
        }

        element_type & operator*() YATO_NOEXCEPT_KEYWORD
        {
            return *m_pointer;
        }
    };
    /**
     * Implementation for smart pointers
     */
    template<typename T>
    class not_null<T, typename std::enable_if< is_smart_ptr<T>::value >::type >
    {    
        using element_type = typename T::element_type;
        const T & m_smart_pointer;
    public:
#if !(defined(YATO_ANDROID) && __cplusplus < 201400L)
        YATO_CONSTEXPR_FUNC
#endif
        not_null(const T & ptr) 
            : m_smart_pointer(ptr) 
        {
            YATO_REQUIRES(ptr != nullptr);
        }

        not_null(std::nullptr_t ptr) = delete;
        
        not_null(const not_null&) = default;
        not_null& operator=(const not_null&) = default;

        ~not_null() = default;

        operator T& () YATO_NOEXCEPT_KEYWORD
        {
            return m_smart_pointer;
        }
        
        YATO_CONSTEXPR_FUNC
        const element_type* get() const YATO_NOEXCEPT_KEYWORD
        {
            return m_smart_pointer.get();
        }

        element_type* get() YATO_NOEXCEPT_KEYWORD
        {
            return m_smart_pointer.get();
        }
        
        YATO_CONSTEXPR_FUNC
        const element_type* operator->() const YATO_NOEXCEPT_KEYWORD
        {
            return m_smart_pointer.operator->();
        }

        element_type* operator->() YATO_NOEXCEPT_KEYWORD
        {
            return m_smart_pointer.operator->();
        }
        
        YATO_CONSTEXPR_FUNC
        const element_type & operator*() const YATO_NOEXCEPT_KEYWORD
        {
            return *m_smart_pointer;
        }

        element_type & operator*() YATO_NOEXCEPT_KEYWORD
        {
            return *m_smart_pointer;
        }
    };

}
#endif