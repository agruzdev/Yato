/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_INSTANCE_OF_H_
#define _YATO_INSTANCE_OF_H_

#include "assert.h"

namespace yato
{

    template<typename _T>
    class instance_of
    {
        static_assert(std::is_same<_T, typename std::decay<_T>::type>::value, "Type should be without any specifiers");

        using instance_type = _T;

        instance_type* m_ptr;

    public:

        template<typename _CheckType>
        YATO_CONSTEXPR_FUNC
        instance_of(_CheckType* ptr)
            : m_ptr(dynamic_cast<instance_type*>(ptr))
        {
            if (nullptr == m_ptr) {
                throw yato::assertion_error("The object is not an instance of the required type");
            }
        }

        template<typename _CheckType>
        YATO_CONSTEXPR_FUNC
        instance_of(_CheckType& ptr)
            : m_ptr(dynamic_cast<instance_type*>(&ptr))
        {
            if (nullptr == m_ptr) {
                throw yato::assertion_error("The object is not an instance of the required type");
            }
        }

        YATO_CONSTEXPR_FUNC
        const instance_type* operator->() const YATO_NOEXCEPT_KEYWORD
        {
            return m_ptr;
        }

        instance_type* operator->() YATO_NOEXCEPT_KEYWORD
        {
            return m_ptr;
        }


        YATO_CONSTEXPR_FUNC
        const instance_type* get() const YATO_NOEXCEPT_KEYWORD
        {
            return m_ptr;
        }

        instance_type* get() YATO_NOEXCEPT_KEYWORD
        {
            return m_ptr;
        }

        YATO_CONSTEXPR_FUNC
        const instance_type& ref() const
        {
            return *m_ptr;
        }

        instance_type& ref()
        {
            return *m_ptr;
        }
    };



}

#endif