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
    class actor_system;

    struct actor_context
    {
        actor_system* system;
        actor_ref self;
        logger_ptr log;

        std::vector<actor_ref> watchers;

        //-------------------------------------------------------
        explicit
        actor_context(actor_system* system, const actor_ref & ref)
            : system(system), self(ref)
        {
            log = logger_factory::create(std::string("Actor[") + ref.get_name() + "]");
        }
    };


}// namespace actors

}// namespace yato

#endif

