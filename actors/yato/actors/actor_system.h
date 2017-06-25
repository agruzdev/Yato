/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_SYSTEM_H_
#define _YATO_ACTOR_SYSTEM_H_

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

#include <yato/any.h>

#include "actor.h"

namespace yato
{
namespace actors
{

    struct actor_cell;
    struct mailbox;

    class abstract_executor;
    class actor_system;

    class actor_system final
    {
    private:
        std::string m_name;
        logger_ptr m_logger;
        std::map<std::string, std::unique_ptr<actor_cell>> m_contexts;
        std::unique_ptr<abstract_executor> m_executor;

        uint32_t m_actors_num;
        std::mutex m_actors_mutex;
        std::condition_variable m_actors_cv;

        const actor_ref m_dead_letters;

        //-------------------------------------------------------
        static void enqueue_system_signal(mailbox* mbox, const system_signal & signal);
        void stop_impl_(actor_cell* act);

        actor_ref create_actor_impl(std::unique_ptr<actor_base> && a, const std::string & name);
        void send_impl(const actor_ref & toActor, const actor_ref & fromActor, yato::any && message);


        //-------------------------------------------------------

    public:
        explicit
        actor_system(const std::string & name);

        /**
         * By default waits for all actors to finish
         * Use method stop() or stopAll() for terminating actors
         * Use poison_pill for terminating actor after processing current messages
         */
        ~actor_system();

        template <typename ActorType_, typename ... Args_>
        actor_ref create_actor(const std::string & name, Args_ && ... args) {
            return create_actor_impl(std::make_unique<ActorType_>(std::forward<Args_>(args)...), name);
        }

        template <typename Ty_>
        void send_message(const actor_ref & addressee, Ty_ && message) {
            send_impl(addressee, dead_letters(), yato::any(message));
        }

        template <typename Ty_>
        void send_message(const actor_ref & addressee, Ty_ && message, const actor_ref & sender) {
            send_impl(addressee, sender, yato::any(message));
        }

        const std::string & name() const {
            return m_name;
        }

        const logger_ptr & logger() const {
            return m_logger;
        }


        const actor_ref & dead_letters() const {
            return m_dead_letters;
        }

        /**
         * Send stop signal to actor and terminate it right after the current message
         */
        void stop(const actor_ref & addressee);

        /**
         * Stop all actors
         * Sends stop() to all created actors
         */
        void stop_all();

        /**
         * Internal method
         */
        void notify_on_stop_();
    };
    //-------------------------------------------------------

    template <typename Ty_>
    inline
    void actor_ref::tell(Ty_ && message) const {
        m_system->send_message(*this, message);
    }

    template <typename Ty_>
    inline
    void actor_ref::tell(Ty_ && message, const actor_ref & sender) const {
        m_system->send_message(*this, sender, message);
    }

    inline
    void actor_ref::stop() const {
        m_system->stop(*this);
    }

}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_SYSTEM_H_

