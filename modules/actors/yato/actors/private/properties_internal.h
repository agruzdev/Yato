/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_PROPS_INTERNAL_H_
#define _YATO_ACTOR_PROPS_INTERNAL_H_

namespace yato
{
namespace actors
{
    struct execution_context;

    struct properties_internal
    {
        execution_context* execution = nullptr;
    };

}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_PROPS_INTERNAL_H_
