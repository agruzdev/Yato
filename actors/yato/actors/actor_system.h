/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_SYSTEM_H_
#define _YATO_ACTOR_SYSTEM_H_

#include <chrono>
#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <mutex>

#include <yato/any.h>

#include "actor.h"
#include "config.h"

namespace yato
{
namespace actors
{

    struct actor_cell;
    struct mailbox;

    class abstract_executor;
    class actor_system;
    class scheduler;
    class name_generator;

    class actor_system final
    {
    private:
        using timeout_type = std::chrono::microseconds;
        using actor_builder = std::function<std::unique_ptr<actor_base>()>;

        std::string m_name;
        logger_ptr m_logger;

        mutable std::mutex m_cells_mutex;
        std::condition_variable m_cells_condition;
        std::map<actor_path, std::unique_ptr<actor_cell>> m_actors;
        size_t m_user_priority_actors_num;

        std::unique_ptr<abstract_executor> m_executor;

        std::unique_ptr<scheduler> m_scheduler;
        std::unique_ptr<name_generator> m_name_generator;

        std::unique_ptr<actor_ref> m_dead_letters;
        //-------------------------------------------------------

        template<typename Ty_, typename... Args_>
        static
        actor_builder make_builder(Args_ && ... args) {
            //return [&] { return std::make_unique<Ty_>(std::forward<Args_>(args)...); };
            return [&] { return std::unique_ptr<Ty_>(new Ty_(std::forward<Args_>(args)...)); };
        }

        actor_ref init_cell_(actor_cell* cell, const actor_builder & builder, const actor_path & path);
        actor_ref create_actor_impl_(const actor_builder & builder, const actor_path & name);
        void send_impl_(const actor_ref & toActor, const actor_ref & fromActor, yato::any && message) const;
        void stop_impl_(mailbox* mbox) const;

        std::future<yato::any> ask_impl_(const actor_ref & addressee, yato::any && message, const timeout_type & timeout) const;

        //-------------------------------------------------------

    private: // Extended inferface for internal usage
        /**
         * Create actor in specified scope
         */
        template <typename ActorType_, typename ... Args_>
        actor_ref create_actor_(const actor_scope & scope, const std::string & name, Args_ && ... args) {
            return create_actor_impl_(make_builder<ActorType_>(std::forward<Args_>(args)...), actor_path(*this, scope, name));
        }

        /**
         * Notify system that actor is topped and can be destroyed
         */
        void notify_on_stop_(const actor_ref & ref);

    public:
        actor_system(const std::string & name, const config & conf);

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
            return create_actor_impl_(make_builder<ActorType_>(std::forward<Args_>(args)...), actor_path(*this, actor_scope::user, name));
        }

        template <typename Ty_>
        void send_message(const actor_ref & addressee, Ty_ && message) const {
            send_impl_(addressee, dead_letters(), yato::any(message));
        }

        template <typename Ty_>
        void send_message(const actor_ref & addressee, Ty_ && message, const actor_ref & sender) const {
            send_impl_(addressee, sender, yato::any(message));
        }

        template <typename Ty_, typename Rep_, typename Period_>
        std::future<yato::any> ask(const actor_ref & addressee, Ty_ && message, const std::chrono::duration<Rep_, Period_> & timeout) const {
            return ask_impl_(addressee, yato::any(message), std::chrono::duration_cast<timeout_type>(timeout));
        }

        const std::string & name() const {
            return m_name;
        }

        const logger_ptr & logger() const {
            return m_logger;
        }

        const actor_ref & dead_letters() const {
            assert(m_dead_letters != nullptr);
            return *m_dead_letters;
        }

        /**
         * Send stop signal to actor and terminate it right after the current message
         */
        void stop(const actor_ref & addressee) const;

        /**
         * Stop all actors
         * Sends stop() to all created actors
         */
        void stop_all();

        /**
         * Find actor by name and get reference to it
         */
        actor_ref select(const actor_path & path) const;

        /**
         * Find actor in user scope of the system
         */
        actor_ref select(const std::string & name) const {
            return select(actor_path(*this, actor_scope::user, name));
        }

        /**
         * Start watch of an actor
         */
        void watch(const actor_ref & watchee, const actor_ref & watcher) const;

        /**
         * Stop watch of an actor
         */
        void unwatch(const actor_ref & watchee, const actor_ref & watcher) const;

        /**
         * Attorney class for accessing extended interface
         */
        friend class actor_system_ex;
    };
    //-------------------------------------------------------

    template <typename Ty_>
    inline
    void actor_ref::tell(Ty_ && message) const {
        m_system->send_message(*this, std::forward<Ty_>(message));
    }

    template <typename Ty_>
    inline
    void actor_ref::tell(Ty_ && message, const actor_ref & sender) const {
        m_system->send_message(*this, std::forward<Ty_>(message), sender);
    }

    template <typename Ty_, typename Rep_, typename Period_>
    inline
    std::future<yato::any> actor_ref::ask(Ty_ && message, const std::chrono::duration<Rep_, Period_> & timeout) const {
        return m_system->ask(*this, std::forward<Ty_>(message), timeout);
    }

    inline
    void actor_ref::stop() const {
        m_system->stop(*this);
    }

}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_SYSTEM_H_

