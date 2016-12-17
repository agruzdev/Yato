/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ANY_PTR_H_
#define _YATO_ANY_PTR_H_

#include <typeindex>
#include "type_traits.h"

namespace yato
{
    namespace details
    {
        void* make_void_ptr_helper(void*);
        const void* make_void_ptr_helper(const void*);
        volatile void* make_void_ptr_helper(volatile void*);
        const volatile void* make_void_ptr_helper(const volatile void*);

        template <typename PTy>
        struct make_void_ptr
        {
            using type = decltype(make_void_ptr_helper(std::declval<PTy>()));
        };

    }

    /**
     *  Type safe wrapper for void*
     *  Simpler and more effective than yato::any
     */
    class any_ptr
    {
    private:
        const volatile void* m_pointer;
        const std::type_info* m_type;

    public:
        template <typename Ty>
        any_ptr(Ty* pointer)
            : m_pointer(pointer), m_type(&typeid(Ty*))
        { }

        any_ptr(std::nullptr_t)
            : m_pointer(nullptr), m_type(&typeid(std::nullptr_t))
        { }

        any_ptr(const any_ptr &) = default;

        any_ptr & operator = (const any_ptr &) = default;

        ~any_ptr() = default;

        const std::type_info & type() const
        {
            return *m_type;
        }

        operator bool() const
        {
            return m_pointer != nullptr;
        }

        template <typename Ty>
        Ty get_as(Ty default_value) const
        {
            if (std::type_index(typeid(Ty)) == std::type_index(type())) {
                return static_cast<Ty>(const_cast<typename details::make_void_ptr<Ty>::type>(m_pointer));
            }
            else {
                return default_value;
            }
        }

        template <typename Ty>
        Ty get_as() const
        {
            return get_as<Ty>(nullptr);
        }
    };


}

#endif

