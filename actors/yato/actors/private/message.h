/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_MESSAGE_H_
#define _YATO_ACTORS_MESSAGE_H_

#include <memory>

#include <yato/any.h>

#include "../actor_ref.h"

namespace yato
{
namespace actors
{

    enum class system_signal
    {
        none,
        start,
        stop
    };

    struct message
    {
        yato::any payload;
        actor_ref sender;

        message(const yato::any & payload, const actor_ref & sender)
            : payload(payload), sender(sender)
        { }

        message(yato::any && payload, const actor_ref & sender)
            : payload(std::move(payload)), sender(sender)
        { }
    };


}// namespace actors

}// namespace yato

#endif

