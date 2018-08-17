/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#include "../inbox.h"
#include "../actor_system.h"
#include "mailbox.h"

namespace yato
{
namespace actors
{
    static const std::string inbox_prefix = "mailbox_";

    inbox::inbox(actor_system & system, const std::string & name)
        : m_ref(&system, actor_path(system, actor_scope::system, inbox_prefix + name))
    {
        // All messages will be fetched manually.
        m_mailbox = std::make_shared<mailbox>(nullptr, /*manual_mode=*/true);
        m_ref.set_mailbox(m_mailbox);
    }
    //---------------------------------------------------------------

    yato::any inbox::receive_impl_(const timeout_type & timeout)
    {
        auto msg = m_mailbox->pop_user_message_sync(timeout);
        return (msg != nullptr) ? std::move(msg->payload) : yato::nullany;
    }

}// namespace actors

}// namespace yato

