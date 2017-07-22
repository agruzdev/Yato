/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_ACTOR_CELL_H_
#define _YATO_ACTORS_ACTOR_CELL_H_

#include <memory>
#include <vector>

namespace yato
{
namespace actors
{
    class actor_base;
    class actor_ref;
    struct mailbox;

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
        std::unique_ptr<actor_base> m_actor;
        std::shared_ptr<mailbox> m_mailbox;

        /**
         * Actor's logger
         */
        logger_ptr m_log;

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
        actor_cell(actor_system & system, const actor_path & path, std::unique_ptr<actor_base> && instance);

        ~actor_cell();

        actor_cell(const actor_cell&) = delete;
        actor_cell& operator= (const actor_cell&) = delete;

        actor_system & system() const {
            return m_system;
        }

        const actor_ref & ref() const {
            return m_self;
        }

        actor_base* actor() const {
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
    };


} // namespace actors

} // namespace yato


#endif // _YATO_ACTORS_ACTOR_CELL_H_
