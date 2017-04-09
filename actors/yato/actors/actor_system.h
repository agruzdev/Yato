/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_SYSTEM_H_
#define _YATO_ACTOR_SYSTEM_H_

#include <memory>

#include <yato/any.h>

#include "actor.h"
#include <map>

namespace yato
{
namespace actors
{
    class pinned_thread_pool;

    struct actor_context;

    class actor_system
    {
    private:
        std::string m_name;
        std::map<std::string, std::unique_ptr<actor_context>> m_contexts;
        std::unique_ptr<pinned_thread_pool> m_pool;
        //-------------------------------------------------------

        actor_ref create_actor_impl(std::unique_ptr<actor_base> && a, const std::string & name);
        void tell_impl(const actor_ref & toActor, yato::any && message);
        //-------------------------------------------------------

    public:
        explicit
        actor_system(const std::string & name);

        ~actor_system();

        template <typename ActorType_, typename ... Args_>
        actor_ref create_actor(const std::string & name, Args_ && ... args) {
            return create_actor_impl(std::make_unique<ActorType_>(std::forward<Args_>(args)...), name);
        }

        template <typename Ty_>
        void tell(const actor_ref & toActor, Ty_ && message) {
            tell_impl(toActor, yato::any(message));
        }
    };


}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_SYSTEM_H_

