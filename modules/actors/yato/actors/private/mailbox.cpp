/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#include "mailbox.h"

#include "actor_cell.h"

namespace yato
{
namespace actors
{
    mailbox::mailbox(actor_cell* node, bool manual_mode)
    {
        if(node != nullptr){
            m_owner_node = node;
            m_owner = node->actor();
        }
        if(manual_mode) {
            m_is_open = true;
            m_is_scheduled = true;
        }
    }
    //-------------------------------------------------------------------------

    mailbox::~mailbox() = default;
    //-------------------------------------------------------------------------

    bool mailbox::enqueue_user_message(std::unique_ptr<message> && msg)
    {
        if(msg == nullptr) {
            return false;
        }

        bool need_process = false;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_is_open) {
                m_usr_queue.push(std::move(msg));
                m_condition.notify_one();
                need_process = !m_is_scheduled;
            }
        }
        return need_process;
    }
    //-------------------------------------------------------------------------

    bool mailbox::enqueue_system_message(std::unique_ptr<message> && msg)
    {
        if(msg == nullptr) {
            return false;
        }

        bool need_process = false;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_is_open) {
                m_sys_queue.push(std::move(msg));
                m_condition.notify_one();
                need_process = !m_is_scheduled;
            }
        }
        return need_process;
    }
    //-------------------------------------------------------------------------

    bool mailbox::schedule_for_execution(bool reschedule)
    {
        YATO_REQUIRES(m_owner_node != nullptr);
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_is_scheduled || reschedule) {
            // User messages can't be processed before actor start, 
            // so if actor is not started, then postpone all user messages
            const bool has_something_to_process = m_owner_node->is_started()
                ? (!m_usr_queue.empty() && m_is_open) || (!m_sys_queue.empty())
                : !m_sys_queue.empty();
            if(has_something_to_process) {
                m_is_scheduled = m_owner_node->executor().execute(shared_from_this());
            }
            else {
                m_is_scheduled = false;
            }
        }
        return m_is_scheduled;
    }
    //-------------------------------------------------------------------------
#if 0
    std::unique_ptr<message> mailbox::pop_system_message()
    {
        std::unique_ptr<message> msg = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!sys_queue.empty()) {
                msg = std::move(sys_queue.front());
                sys_queue.pop();
            }
        }
        return msg;
    }
#endif
    //-------------------------------------------------------------------------

    std::unique_ptr<message> mailbox::pop_user_message_sync(const timeout_type & timeout)
    {
        std::unique_ptr<message> msg = nullptr;
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_usr_queue.empty()) {
            const auto due_time = std::chrono::high_resolution_clock::now() + timeout;
            while(m_usr_queue.empty() && std::chrono::high_resolution_clock::now() < due_time) {
                m_condition.wait_until(lock, due_time);
            }
        }
        if(!m_usr_queue.empty()) {
            msg = std::move(m_usr_queue.front());
            m_usr_queue.pop();
        }
        return msg;
    }

    //-------------------------------------------------------------------------

    void mailbox::close()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_is_open = false;
        m_is_scheduled = false;
    }
    //-------------------------------------------------------------------------

    std::unique_ptr<message> mailbox::try_pop_prioritized_message_(bool* is_system)
    {
        YATO_REQUIRES(is_system != nullptr);
        std::unique_ptr<message> msg = nullptr;
        do
        {
            if (!m_sys_queue.empty()) {
                msg = std::move(m_sys_queue.front());
                m_sys_queue.pop();
                *is_system = true;
                break;
            }
            if(!m_owner_node->is_started()) {
                // dont process user messages until started
                break;
            }
            if(!m_usr_queue.empty()) {
                msg = std::move(m_usr_queue.front());
                m_usr_queue.pop();
                *is_system = false;
            }
        } while(false);
        return msg;
    }
    //-------------------------------------------------------------------------

    std::unique_ptr<message> mailbox::pop_prioritized_message(bool* is_system)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return try_pop_prioritized_message_(is_system);
    }
    //-------------------------------------------------------------------------

    std::unique_ptr<message> mailbox::pop_prioritized_message_sync(bool* is_system)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::unique_ptr<message> msg = nullptr;
        for(;;) {
            msg = try_pop_prioritized_message_(is_system);
            if(msg != nullptr) {
                break;
            }
            m_condition.wait(lock);
        }
        return msg;
    }
    //-------------------------------------------------------------------------

} // namespace actors

} // namespace yato

