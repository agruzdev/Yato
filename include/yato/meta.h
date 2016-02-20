/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_META_H_
#define _YATO_META_H_

#include "type_traits.h"
#include "types.h"

namespace yato
{
    namespace meta
    {

        /**
         *  Wrapper around unsigned integer value
         */
        template<uint32_t _Num>
        struct Number: public Number<_Num - 1>
        {
            static YATO_CONSTEXPR_VAR uint32_t value = _Num;
        };

        template<>
        struct Number<0>
        {
            static YATO_CONSTEXPR_VAR uint32_t value = 0;
        };

    }
}

#endif