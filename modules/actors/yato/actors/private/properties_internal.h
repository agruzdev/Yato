/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
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
