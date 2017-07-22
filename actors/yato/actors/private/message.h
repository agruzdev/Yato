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
    class actor_cell;

    struct system_message
    {
        /**
         * First message an actor gets. Invokes pre_start()
         */
        struct start {};

        /**
         * Last message an actor gets. Terminatesactor and invokes post_stop()
         */
        struct stop {};

        /**
         * Adds watcher
         */
        struct watch
        {
            actor_ref watcher;

            explicit
            watch(const actor_ref & watcher)
                : watcher(watcher)
            { }
        };

        /**
         * Removes watcher
         */
        struct unwatch
        {
            actor_ref watcher;

            explicit
            unwatch(const actor_ref & watcher)
                : watcher(watcher)
            { }
        };

        struct attach_child;
    };

    struct message
    {
        yato::any payload;
        actor_ref sender;
        //-----------------------------------------------------------

        message(const yato::any & payload, const actor_ref & sender)
            : payload(payload), sender(sender)
        { }

        message(yato::any && payload, const actor_ref & sender)
            : payload(std::move(payload)), sender(sender)
        { }
    };


    /**
    * Wraps a message for finding addressee in the actors tree
    */
    struct envelop
    {
        message msg;
        std::vector<std::string> route;

        explicit
        envelop(message && msg)
            : msg(msg) 
        { }
    };


}// namespace actors

}// namespace yato

#endif

