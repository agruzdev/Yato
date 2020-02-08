/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_PROPS_H_
#define _YATO_ACTOR_PROPS_H_

#include <string>

namespace yato
{
namespace actors
{

    struct execution_context;

    struct properties
    {
        std::string execution_name = "default";
    };


}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_PROPS_H_
