/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_MESSAGE_H_
#define _YATO_ACTORS_MESSAGE_H_

#include <yato/any.h>

#include"actor.h"

namespace yato
{
namespace actors
{

    struct message
    {
        yato::any payload;
        //actor_ref sender;
    };


}// namespace actors

}// namespace yato

#endif

