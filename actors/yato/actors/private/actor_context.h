/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_ACTOR_CONTEXT_H_
#define _YATO_ACTORS_ACTOR_CONTEXT_H_

#include <memory>

#include <yato/any.h>

#include "../actor_ref.h"
#include "../logger.h"

namespace yato
{
namespace actors
{

    struct actor_context
    {
        logger_ptr log;
        actor_ref self;

        //-------------------------------------------------------
        explicit
        actor_context(const actor_ref & ref)
            : self(ref) 
        {
            log = logger_factory::create(ref.get_name());
        }
    };


}// namespace actors

}// namespace yato

#endif

