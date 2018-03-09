/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_SYSTEM_MESSAGE_H_
#define _YATO_ACTORS_SYSTEM_MESSAGE_H_

#include <memory>

#include <yato/any.h>

#include "../actor_ref.h"
#include "actor_cell.h"

namespace yato
{
namespace actors
{
    struct system_message
    {
        /**
         * First message an actor gets. Invokes pre_start()
         */
        struct start {};

        /**
         * Terminate actor. Broadcasts stop to all children.
         */
        struct stop {};

        /**
         * Terminate actor after there  are no children left. Don't broadcast stop to the children.
         */
        struct stop_after_children {};

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

        /**
         * Attach new child actor to the addressee
         */
        struct attach_child
        {
            std::unique_ptr<actor_cell> cell;

            explicit
            attach_child(std::unique_ptr<actor_cell> && cell)
                : cell(std::move(cell))
            { }
        };

        /**
         * Remove child and destroy.
         * Is sent after child is stopped.
         */
        struct detach_child
        {
            actor_ref ref;

            explicit
            detach_child(const actor_ref & ref)
                : ref(ref)
            { }
        };


        /**
         * Message for searching an actor
         */
        struct selection
        {
            actor_ref sender;
            actor_scope scope;
            std::vector<std::string> path;

            selection(const actor_ref & sender, const actor_scope & scope, const std::vector<std::string> & path)
                : sender(sender), scope(scope), path(path)
            { }

            selection(const actor_ref & sender, const actor_scope & scope, std::vector<std::string> && path)
                : sender(sender), scope(scope), path(std::move(path))
            { }
        };

    };

}// namespace actors

}// namespace yato

#endif

