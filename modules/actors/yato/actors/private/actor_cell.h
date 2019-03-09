/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_ACTOR_CELL_H_
#define _YATO_ACTORS_ACTOR_CELL_H_

#include <memory>
#include <vector>

#include "../logger.h"

#include "execution_context.h"
#include "properties_internal.h"

namespace yato
{
namespace actors
{
    class basic_actor;
    class actor_ref;
    class mailbox;

    class actor_cell
    {
        /**
         * Parent system
         */
        actor_system & m_system;

        /**
         * Stored actor
         */
        actor_ref m_self;
        std::unique_ptr<basic_actor> m_actor;
        std::shared_ptr<mailbox> m_mailbox;

        /**
         * Context
         */
        execution_context* m_execution_context = nullptr;

        /**
         * Actor's logger
         */
        logger_ptr m_log;

        /**
         * Started
         */
        bool m_started;

        /**
         * Stop message is received
         */
        bool m_stop;

        /**
         * Actor's watchers
         */
        std::vector<actor_ref> m_watchers;

        /**
         * Tree structure
         */
        std::unique_ptr<actor_ref> m_parent;
        std::vector<std::unique_ptr<actor_cell>> m_children;

        //----------------------------------------------
    public:
        actor_cell(actor_system & system, const actor_path & path, const properties_internal & props, std::unique_ptr<basic_actor> && instance);

        ~actor_cell();

        actor_cell(const actor_cell&) = delete;
        actor_cell& operator= (const actor_cell&) = delete;

        actor_system & system() const {
            return m_system;
        }

        const actor_ref & ref() const {
            return m_self;
        }

        bool has_parent() const {
            return (m_parent != nullptr);
        }

        const actor_ref & parent() const {
            assert(m_parent != nullptr);
            return *m_parent;
        }

        basic_actor* actor() const {
            return m_actor.get();
        }

        logger & log() const {
            assert(m_log != nullptr);
            return *m_log;
        }

        std::vector<actor_ref> & watchers() {
            return m_watchers;
        }

        mailbox* mail() const {
            return m_mailbox.get();
        }

        bool is_started() const {
            return m_started;
        }

        void set_started(bool val) {
            m_started = val;
        }

        void set_stop(bool val) {
            m_stop = val;
        }

        bool stopping() const {
            return m_stop;
        }

        /**
         * Add child to the cell.
         * Sets correct parent ref to the child as well.
         * @return child ref
         */
        actor_ref add_child(std::unique_ptr<actor_cell> && child);

        /**
         * Destroys child actor.
         * Child has to be stopped.
         */
        void remove_child(const actor_ref & ref);

        const std::vector<std::unique_ptr<actor_cell>> & children() const {
            return m_children;
        }


        abstract_executor & executor() const
        {
            YATO_REQUIRES(m_execution_context != nullptr);
            return *(m_execution_context->executor);
        }

    };


} // namespace actors

} // namespace yato


#endif // _YATO_ACTORS_ACTOR_CELL_H_
