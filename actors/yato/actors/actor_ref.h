/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_REF_H_
#define _YATO_ACTOR_REF_H_

#include <future>
#include <memory>
#include <string>

#include <yato/prerequisites.h>
#include <yato/any.h>

#include "actor_path.h"

namespace yato
{
namespace actors
{
    class actor_system;
    struct mailbox;

    /**
     * Special message type for graceful stopping of actor
     * Actor will be stopped after processing all messages in queue before the poison_pill.
     */
    struct poison_pill_t {};

    YATO_INLINE_VARIABLE constexpr 
    poison_pill_t poison_pill;


    /**
     * Unique handle of an actor
     */
    class actor_ref final
    {
    private:
        actor_system* m_system; // Not owning pointer. Can copy
        std::weak_ptr<mailbox> m_mailbox;

        actor_path m_path;

        // Interface for actor_system

        actor_ref(actor_system* system, const actor_path & path);

        void set_mailbox(const std::shared_ptr<mailbox> & ptr);

        const std::weak_ptr<mailbox> & get_mailbox() const {
            return m_mailbox;
        }

    public:
        /**
         * Empty ref not correspondint any system
         */
        actor_ref();

        ~actor_ref();

        actor_ref(const actor_ref&) = default;
        actor_ref(actor_ref&&) = default;

        actor_ref& operator=(const actor_ref&) = default;
        actor_ref& operator=(actor_ref&&) = default;

        std::string name() const 
        {
            return m_path.get_name();
        }

        const actor_path & get_path() const 
        {
            return m_path;
        }

        /**
         * Send message to the actor
         */
        template <typename Ty_>
        void tell(Ty_ && message) const;

        template <typename Ty_>
        void tell(Ty_ && message, const actor_ref & sender) const;

        /**
         * Send message for response
         */
        template <typename Ty_, typename Rep_, typename Period_>
        std::future<yato::any> ask(Ty_ && message, const std::chrono::duration<Rep_, Period_> & timeout) const;

        /**
         * Stop actor immediately.
         * Right after processing the current message.
         */
        void stop() const;

        /**
         * Watch this actor
         */
        void watch(const actor_ref & watcher) const;

        /**
         * Check that this ref is empty, not corresponding any actor
         */
        bool empty() const 
        {
            return m_system == nullptr;
        }

        friend
        bool operator == (const actor_ref & one, const actor_ref & another)
        {
            return one.m_path == another.m_path;
        }

        friend
        bool operator != (const actor_ref & one, const actor_ref & another)
        {
            return one.m_path != another.m_path;
        }

        friend class actor_system;
        friend class actor_cell;
        friend class inbox;
    };
    //-------------------------------------------------------

    // ToDo (a.gruzdev): Group all standard messages
    /**
     * Special message type informing that actor was stopped.
     */
    struct terminated
    {
        actor_ref ref;

        explicit
        terminated(const actor_ref & ref)
            : ref(ref)
        { }
    };


}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_REF_H_

