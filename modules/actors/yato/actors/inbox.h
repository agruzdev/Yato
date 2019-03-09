/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_INBOX_H_
#define _YATO_ACTOR_INBOX_H_

#include <chrono>

#include "actor_common.h"
#include "actor_ref.h"

namespace yato
{
namespace actors
{

    class mailbox;
    class actor_system;

    /**
     *  An actor-like object which can be used for accepting messages from actors.
     *  Inbox's ref can be used to send messages locally, but cant be looked up by actor_system::find.
     *  Inbox can't be watched.
     */
    class inbox
    {
        std::shared_ptr<mailbox> m_mailbox;
        actor_ref m_ref;
        //--------------------------------------------------

        yato::any receive_impl_(const timeout_type & timeout);
        //--------------------------------------------------

    public:
        /**
         * Create a new inbox for an actor_system
         */
        inbox(actor_system & system, const std::string & name);

        ~inbox() = default;

        inbox(const inbox&) = delete;
        inbox(inbox&&) = default;

        inbox& operator=(const inbox&) = delete;
        inbox& operator=(inbox&&) = delete;

        /**
         * Obtain a reference to the internal actor, which can be used as addressee for messages.
         */
        const actor_ref & ref() const {
            return m_ref;
        }

        /**
         * Receive the next message from this inbox.
         */
        yato::any receive() {
            return receive_impl_(std::chrono::milliseconds(0));
        }

        /**
         * Receive the next message from this inbox.
         */
        template <typename Rep_, typename Period_>
        yato::any receive(const std::chrono::duration<Rep_, Period_> & timeout) {
            return receive_impl_(timeout);
        }

        /**
         * Send the given message to a target with this inbox as sender
         */
        template <typename Ty_>
        void send(const actor_ref & target, Ty_ && message) const {
            target.tell(std::forward<Ty_>(message), ref());
        }

        /**
         * Have the inbox watch an actor.
         */
        void watch(const actor_ref & watchee) const {
            watchee.watch(ref());
        }
    };


} // namespacea actors

} // namespace yato

#endif //_YATO_ACTOR_INBOX_H_

