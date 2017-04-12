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
    class abstract_executor;
    class actor_system;

    /**
     * Unique handle of an actor
     */
    class actor_ref final
    {
    private:
        // Not owning pointer. Can copy
        actor_system* m_system;

        std::string m_name;
        std::string m_path;

        actor_ref(actor_system* system, const std::string & name);
    public:

        ~actor_ref() = default;

        actor_ref(const actor_ref&) = default;
        actor_ref(actor_ref&&) = default;

        actor_ref& operator=(const actor_ref&) = default;
        actor_ref& operator=(actor_ref&&) = default;

        const std::string & get_name() const {
            return m_name;
        }

        const std::string & get_path() const {
            return m_path;
        }

        template <typename Ty_>
        void tell(Ty_ && message);

        friend class actor_system;
    };
    //-------------------------------------------------------


    class actor_system final
    {
    private:
        std::string m_name;
        std::map<std::string, std::unique_ptr<actor_context>> m_contexts;
        std::unique_ptr<abstract_executor> m_executor;
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

        const std::string & get_name() const {
            return m_name;
        }
    };


    template <typename Ty_>
    inline
    void actor_ref::tell(Ty_ && message) {
        m_system->tell(*this, message);
    }


}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_SYSTEM_H_

