/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_INVOKE_H_
#define _YATO_INVOKE_H_

#include "type_traits.h"

namespace yato
{

    /**
     *	Abstract call of a method by pointer for an object passed by value, reference, pointer or smart pointer
     */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
    template<typename _Class, typename _MethodPtr, typename... _MethodArgs>
    YATO_CONSTEXPR_FUNC auto invoke(const _Class & obj, _MethodPtr && method, _MethodArgs &&... args)
        -> decltype(((*obj).*method)(std::forward<_MethodArgs>(args)...))
    {
        return ((*obj).*method)(std::forward<_MethodArgs>(args)...);
    }
#else
    template<typename _Class, typename _MethodPtr, typename... _MethodArgs>
    YATO_CONSTEXPR_FUNC auto invoke(const _Class & obj, _MethodPtr && method, _MethodArgs &&... args)
        -> typename std::enable_if<std::is_pointer<_Class>::value || yato::is_smart_ptr<_Class>::value,  decltype(((*obj).*method)(std::forward<_MethodArgs>(args)...))>::type
    {
        return ((*obj).*method)(std::forward<_MethodArgs>(args)...);
    }
#endif

    template<typename _Class, typename _MethodPtr, typename... _MethodArgs>
    YATO_CONSTEXPR_FUNC auto invoke(_Class && obj, _MethodPtr && method, _MethodArgs &&... args)
        -> decltype((obj.*method)(std::forward<_MethodArgs>(args)...))
    {
        return (obj.*method)(std::forward<_MethodArgs>(args)...);
    }

}

#endif