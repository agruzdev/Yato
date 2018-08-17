/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_SYSTEM_EX_H_
#define _YATO_ACTOR_SYSTEM_EX_H_

#include "../actor_system.h"
#include "../cell_builder.h"

namespace yato
{
namespace actors
{

    /**
     * Extended interface for actor system. 
     * For internal usage.
     */
    class actor_system_ex
    {
    public:
        static
        const actor_ref & root(const actor_system & sys) {
            return sys.root();
        }

        static
        actor_ref create_actor(actor_system & sys, const actor_scope & scope, const std::string & name, const details::cell_builder & builder) {
            return sys.create_actor_(scope, name, builder);
        }

        static
        actor_ref create_actor(actor_system & sys, const actor_ref & parent, const std::string & name, const details::cell_builder & builder) {
            return sys.create_actor_(parent, name, builder);
        }

        template <typename ActorType_, typename ... Args_>
        static
        actor_ref create_actor(actor_system & sys, const actor_scope & scope, const std::string & name, Args_ && ... args) {
            return sys.create_actor_(scope, name, details::make_cell_builder<ActorType_>(std::forward<Args_>(args)...));
        }

        template <typename ActorType_, typename ... Args_>
        static
        actor_ref create_actor(actor_system & sys, const actor_ref & parent, const std::string & name, Args_ && ... args) {
            return sys.create_actor_(parent, name, details::make_cell_builder<ActorType_>(std::forward<Args_>(args)...));
        }

        static
        void notify_on_stop(actor_system & sys, const actor_ref & ref) {
            sys.notify_on_stop_(ref);
        }

        /**
         * Send system message
         */
        template <typename Ty_>
        static
        void send_system_message(const actor_system & sys, const actor_ref & addressee, Ty_ && message) {
            sys.send_system_impl_(addressee, sys.dead_letters(), yato::any(std::forward<Ty_>(message)));
        }

        template <typename Ty_>
        static
        void send_system_message(const actor_system & sys, const actor_ref & addressee, Ty_ && message, const actor_ref & sender) {
            sys.send_system_impl_(addressee, sender, yato::any(std::forward<Ty_>(message)));
        }
    };

} // namespacea actors

} // namespace yato

#endif //_YATO_ACTOR_SYSTEM_EX_H_

