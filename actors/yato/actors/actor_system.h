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
#include "cell_builder.h"

namespace yato
{
namespace actors
{

    class actor_cell;
    struct mailbox;

    class abstract_executor;
    class actor_system;
    class scheduler;
    class name_generator;

    class actor_system
    {
    private:
        using timeout_type = std::chrono::microseconds;

        std::string m_name;
        logger_ptr m_logger;

        std::unique_ptr<actor_cell> m_root;
        std::unique_ptr<actor_ref> m_dead_letters;

        std::mutex m_terminate_mutex;
        std::condition_variable m_terminate_cv;
        bool m_root_stopped;

        std::unique_ptr<name_generator> m_name_generator;

        std::unique_ptr<scheduler> m_scheduler;
        std::unique_ptr<abstract_executor> m_executor;
        
        //-------------------------------------------------------

        actor_ref create_actor_impl_(const details::cell_builder & builder, const actor_path & name, const actor_ref & parent);

        void send_impl_(const actor_ref & toActor, const actor_ref & fromActor, yato::any && message) const;
        void send_system_impl_(const actor_ref & addressee, const actor_ref & sender, yato::any && userMessage) const;
        void stop_impl_(const std::shared_ptr<mailbox> & mbox) const;

        std::future<yato::any> ask_impl_(const actor_ref & addressee, yato::any && message, const timeout_type & timeout) const;

        std::future<actor_ref> find_impl_(const actor_path & path, const timeout_type & timeout) const;

        /**
         * If forced then terminates all actors, otherwise only after all user scope actors are stopped by user.
         */
        void shutdown_impl_(bool forced);

        //-------------------------------------------------------

    private: // Extended inferface for internal usage
        /**
         * Get root actor reference
         */
        const actor_ref & root() const;

        /**
         * Create actor in specified scope
         */
        actor_ref create_actor_(const actor_scope & scope, const std::string & name, const details::cell_builder & builder) {
            if(!actor_path::is_valid_actor_name(name)) {
                throw yato::argument_error("Invalid actor name!");
            }
            return create_actor_impl_(builder, actor_path(*this, scope, name), actor_ref{});
        }

        /**
         * Create a child actor
         */
        actor_ref create_actor_(const actor_ref & parent, const std::string & name, const details::cell_builder & builder) {
            YATO_REQUIRES(!parent.empty());
            if(!actor_path::is_valid_actor_name(name)) {
                throw yato::argument_error("Invalid actor name!");
            }
            const auto child_path = actor_path::join(parent.get_path(), name);
            return create_actor_impl_(builder, child_path, parent);
        }

        /**
         * Send system message
         */
        template <typename Ty_>
        void send_system_message(const actor_ref & addressee, Ty_ && message) const {
            send_system_impl_(addressee, dead_letters(), yato::any(std::forward<Ty_>(message)));
        }

        template <typename Ty_>
        void send_system_message(const actor_ref & addressee, Ty_ && message, const actor_ref & sender) const {
            send_system_impl_(addressee, sender, yato::any(std::forward<Ty_>(message)));
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
         * Waits for all user actors to finish.
         * Use method stop() or stopAll() for terminating actors
         * Use poison_pill for terminating actor after processing current messages
         */
        ~actor_system();

        /**
         * Create a user actor
         */
        template <typename ActorType_, typename ... Args_>
        actor_ref create_actor(const std::string & name, Args_ && ... args) {
            if(!actor_path::is_valid_actor_name(name)) {
                throw yato::argument_error("Invalid actor name!");
            }
            return create_actor_impl_(details::make_cell_builder<ActorType_>(std::forward<Args_>(args)...), actor_path(*this, actor_scope::user, name), actor_ref{});
        }

        template <typename Ty_>
        void send_message(const actor_ref & addressee, Ty_ && message) const {
            send_impl_(addressee, dead_letters(), yato::any(std::forward<Ty_>(message)));
        }

        template <typename Ty_>
        void send_message(const actor_ref & addressee, Ty_ && message, const actor_ref & sender) const {
            send_impl_(addressee, sender, yato::any(std::forward<Ty_>(message)));
        }

        template <typename Ty_, typename Rep_, typename Period_>
        std::future<yato::any> ask(const actor_ref & addressee, Ty_ && message, const std::chrono::duration<Rep_, Period_> & timeout) const {
            return ask_impl_(addressee, yato::any(std::forward<Ty_>(message)), std::chrono::duration_cast<timeout_type>(timeout));
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
         * Find actor by path
         */
        template <typename Rep_, typename Period_>
        std::future<actor_ref> find(const actor_path & path, const std::chrono::duration<Rep_, Period_> & timeout) const {
            return find_impl_(path, std::chrono::duration_cast<timeout_type>(timeout));
        }

        /**
         * Find actor by name in user scope
         */
        template <typename Rep_, typename Period_>
        std::future<actor_ref> find(const std::string & name, const std::chrono::duration<Rep_, Period_> & timeout) const {
            return find_impl_(actor_path(*this, actor_scope::user, name), std::chrono::duration_cast<timeout_type>(timeout));
        }

        /**
         * Start watch of an actor.
         * If watchee doesn't exist then terminated is sent immediately.
         */
        void watch(const actor_ref & watchee, const actor_ref & watcher) const;

        /**
         * Stop watch of an actor.
         */
        void unwatch(const actor_ref & watchee, const actor_ref & watcher) const;

        // ToDo (a.gruzdev): Make non-blocking
        /**
         * Terminates the actor system, stopping all actors.
         * Blocks until the system is stopped.
         */
        void shutdown();

        /**
         * Attorney class for accessing extended interface
         */
        friend class actor_system_ex;
    };
    //-------------------------------------------------------

    template <typename Ty_>
    inline
    void actor_ref::tell(Ty_ && message) const {
        YATO_REQUIRES(!empty());
        m_system->send_message(*this, std::forward<Ty_>(message));
    }

    template <typename Ty_>
    inline
    void actor_ref::tell(Ty_ && message, const actor_ref & sender) const {
        YATO_REQUIRES(!empty());
        m_system->send_message(*this, std::forward<Ty_>(message), sender);
    }

    template <typename Ty_, typename Rep_, typename Period_>
    inline
    std::future<yato::any> actor_ref::ask(Ty_ && message, const std::chrono::duration<Rep_, Period_> & timeout) const {
        YATO_REQUIRES(!empty());
        return m_system->ask(*this, std::forward<Ty_>(message), timeout);
    }

    inline
    void actor_ref::watch(const actor_ref & watcher) const {
        YATO_REQUIRES(!empty());
        if(!watcher.empty() && watcher != m_system->dead_letters()) {
            m_system->watch(*this, watcher);
        }
    }

    inline
    void actor_ref::stop() const {
        YATO_REQUIRES(!empty());
        m_system->stop(*this);
    }

}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_SYSTEM_H_

