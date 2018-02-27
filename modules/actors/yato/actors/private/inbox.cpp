/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
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
        m_mailbox = std::make_shared<mailbox>();
        m_mailbox->is_open = true;
        m_mailbox->is_scheduled = true; // Prevents it from adding to any executor.
                                        // All messages will be fetched manually.

        m_ref.set_mailbox(m_mailbox);
    }
    //---------------------------------------------------------------

    yato::any inbox::receive_impl_(const timeout_type & timeout)
    {
        std::unique_ptr<message> msg = nullptr;
        {
            std::unique_lock<std::mutex> lock(m_mailbox->mutex);
            
            if(m_mailbox->queue.empty()) {
                const auto due_time = std::chrono::high_resolution_clock::now() + timeout;
                while(m_mailbox->queue.empty() && std::chrono::high_resolution_clock::now() < due_time) {
                    m_mailbox->condition.wait_until(lock, due_time);
                }
            }

            if(!m_mailbox->queue.empty()) {
                msg = std::move(m_mailbox->queue.front());
                m_mailbox->queue.pop();
            }
        }
        return (msg != nullptr) ? std::move(msg->payload) : yato::nullany;
    }

}// namespace actors

}// namespace yato

