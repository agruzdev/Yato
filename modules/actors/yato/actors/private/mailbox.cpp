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

    void mailbox::schedule_for_execution()
    {
        YATO_REQUIRES(owner_node != nullptr);
        owner_node->executor().execute(shared_from_this());
    }
    //-------------------------------------------------------------------------

} // namespace actors

} // namespace yato

