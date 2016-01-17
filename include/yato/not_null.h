/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016 Alexey Gruzdev
 */

#ifndef _YATO_NOT_NULL_
#define _YATO_NOT_NULL_

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
        T m_pointer;
    public:
        constexpr not_null(T ptr):
            m_pointer(ptr) 
        {
            
            if(ptr == nullptr){
                throw std::runtime_error("not_null: null pointer exception!");
            }
        }
        
        not_null(nullptr_t ptr) = delete;
        
        constexpr operator T () const noexcept {
            return m_pointer;
        }
        
        constexpr const auto get() const noexcept {
            return m_pointer;
        }

        auto get() noexcept {
            return m_pointer;
        }
        
        constexpr const auto operator->() const noexcept {
            return m_pointer;
        }

        auto operator->() noexcept {
            return m_pointer;
        }
        
        constexpr const auto & operator*() const noexcept {
            return *m_pointer;
        }

        auto & operator*() noexcept {
            return *m_pointer;
        }
    };
    /**
     * Implementation for smart pointers
     */
    template<typename T>
    class not_null<T, typename std::enable_if< is_smart_ptr<T>::value >::type >
    {    
        const T & m_smart_pointer;
    public:
        constexpr not_null(const T & ptr):
            m_smart_pointer(ptr) 
        {
            if(m_smart_pointer.get() == nullptr){
                throw std::runtime_error("not_null: null pointer exception!");
            }
        }

        not_null(nullptr_t ptr) = delete;
        
        operator T& () noexcept {
            return m_smart_pointer;
        }
        
        constexpr const auto get() const noexcept {
            return m_smart_pointer.get();
        }

        auto get() noexcept {
            return m_smart_pointer.get();
        }
        
        constexpr const auto operator->() const noexcept {
            return m_smart_pointer.operator->();
        }

        auto operator->() noexcept {
            return m_smart_pointer.operator->();
        }
        
        constexpr const auto& operator*() const noexcept {
            return *m_smart_pointer;
        }

        auto & operator*() noexcept {
            return *m_smart_pointer;
        }
    };

}
#endif