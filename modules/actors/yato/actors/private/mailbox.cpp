/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#include "mailbox.h"
#include "actor_cell.h"

namespace yato
{
namespace actors
{
    bool mailbox::enqueue_user_message(std::unique_ptr<message> && msg)
    {
        if(msg == nullptr) {
            return false;
        }

        bool need_process = false;
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (is_open) {
                queue.push(std::move(msg));
                condition.notify_one();
                need_process = !is_scheduled;
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
            std::unique_lock<std::mutex> lock(mutex);
            if (is_open) {
                sys_queue.push(std::move(msg));
                condition.notify_one();
                need_process = !is_scheduled;
            }
        }
        return need_process;
    }
    //-------------------------------------------------------------------------

    bool mailbox::schedule_for_execution(bool reschedule)
    {
        YATO_REQUIRES(owner_node != nullptr);
        std::unique_lock<std::mutex> lock(mutex);
        if (!is_scheduled || reschedule) {
            // User messages can't be processed before actor start, 
            // so if actor is not started, then postpone all user messages
            const bool has_something_to_process = owner_node->is_started()
                ? (!queue.empty() && is_open) || (!sys_queue.empty())
                : !sys_queue.empty();
            if(has_something_to_process) {
                is_scheduled = owner_node->executor().execute(shared_from_this());
            }
            else {
                is_scheduled = false;
            }
        }
        return is_scheduled;
    }
    //-------------------------------------------------------------------------

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
    //-------------------------------------------------------------------------

    void mailbox::close()
    {
        std::unique_lock<std::mutex> lock(mutex);
        is_open = false;
        is_scheduled = false;
    }
    //-------------------------------------------------------------------------

    std::unique_ptr<message> mailbox::try_pop_prioritized_message_(bool* is_system)
    {
        YATO_REQUIRES(is_system != nullptr);
        std::unique_ptr<message> msg = nullptr;
        do
        {
            if (!sys_queue.empty()) {
                msg = std::move(sys_queue.front());
                sys_queue.pop();
                *is_system = true;
                break;
            }
            if(!owner_node->is_started()) {
                // dont process user messages until started
                break;
            }
            if(!queue.empty()) {
                msg = std::move(queue.front());
                queue.pop();
                *is_system = false;
            }
        } while(false);
        return msg;
    }
    //-------------------------------------------------------------------------

    std::unique_ptr<message> mailbox::pop_prioritized_message(bool* is_system)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return try_pop_prioritized_message_(is_system);
    }
    //-------------------------------------------------------------------------

    std::unique_ptr<message> mailbox::pop_prioritized_message_sync(bool* is_system)
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::unique_ptr<message> msg = nullptr;
        for(;;) {
            msg = try_pop_prioritized_message_(is_system);
            if(msg != nullptr) {
                break;
            }
            condition.wait(lock);
        }
        return msg;
    }
    //-------------------------------------------------------------------------

} // namespace actors

} // namespace yato

