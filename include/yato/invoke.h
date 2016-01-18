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
    template<typename _Class, typename _MethodPtr, typename... _MethodArgs>
    constexpr inline auto invoke(const _Class & obj, _MethodPtr && method, _MethodArgs &&... args)
        -> decltype(((*obj).*method)(std::forward<_MethodArgs>(args)...))
    {
        return ((*obj).*method)(std::forward<_MethodArgs>(args)...);
    }

    template<typename _Class, typename _MethodPtr, typename... _MethodArgs>
    constexpr inline auto invoke(_Class & obj, _MethodPtr && method, _MethodArgs &&... args)
        -> decltype(((*obj).*method)(std::forward<_MethodArgs>(args)...))
    {
        return ((*obj).*method)(std::forward<_MethodArgs>(args)...);
    }

    template<typename _Class, typename _MethodPtr, typename... _MethodArgs>
    constexpr inline auto invoke(_Class && obj, _MethodPtr && method, _MethodArgs &&... args)
        -> decltype((obj.*method)(std::forward<_MethodArgs>(args)...))
    {
        return (obj.*method)(std::forward<_MethodArgs>(args)...);
    }

}

#endif