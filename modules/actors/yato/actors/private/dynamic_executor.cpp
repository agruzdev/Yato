/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#include "../actor_system.h"
#include "actor_system_ex.h"
#include "thread_pool.h"
#include "actor_cell.h"
#include "dynamic_executor.h"

namespace yato
{
namespace actors
{

    void dynamic_executor::mailbox_function(dynamic_executor* executor, const std::shared_ptr<mailbox> & mbox, uint32_t throughput)
    {
        using details::process_result;
        const actor_ref ref = mbox->owner->self();
        bool ignore_user_queue = false;
        for(uint32_t count = 0;;) {
            if(process_result::request_stop == process_all_system_messages(mbox)) {
                // Terminate actor
                {
                    std::unique_lock<std::mutex> lock(mbox->mutex);
                    mbox->is_open = false;
                    mbox->is_scheduled = false;
                }
                actor_system_ex::notify_on_stop(*executor->m_system, ref);
                return;
            }

            if(!mbox->owner_node->is_started()) {
                // ToDo (a.gruzdev): Quick solution
                // dont process user messages until started
                ignore_user_queue = true;
                break;
            }

            std::unique_ptr<message> msg = nullptr;
            {
                std::unique_lock<std::mutex> lock(mbox->mutex);

                if(!mbox->sys_queue.empty()) {
                    continue;
                }

                if (mbox->queue.empty() || count >= throughput) {
                    // Terminate batch
                    break;
                }

                msg = std::move(mbox->queue.front());
                mbox->queue.pop();
                ++count;
            }
            if (msg) {
                mbox->owner->receive_message_(std::move(*msg));
            }
        }
        executor->reschedule(mbox, ignore_user_queue);
    }
    //----------------------------------------------------------

    /**
     * If only_system is true, then ignore user queue, reschedule only if there are new system messages in the mailbox
     */
    void dynamic_executor::reschedule(const std::shared_ptr<mailbox> & mbox, bool ignore_user)
    {
        YATO_REQUIRES(mbox != nullptr);
        YATO_REQUIRES(mbox->is_scheduled);
        std::unique_lock<std::mutex> lock(mbox->mutex);
        const bool need_schedule = ignore_user
            ? !mbox->sys_queue.empty()
            : (!mbox->queue.empty() && mbox->is_open) || (!mbox->sys_queue.empty());
        if (need_schedule) {
            m_tpool->enqueue(mailbox_function, this, mbox, m_throughput);
        } else {
            mbox->is_scheduled = false;
        }
    }
    //----------------------------------------------------------

    dynamic_executor::dynamic_executor(actor_system* system, uint32_t threads_num, uint32_t throughput)
        : m_system(system), m_throughput(throughput)
    {
        m_tpool = std::make_unique<thread_pool>(threads_num);
    }
    //-----------------------------------------------------------

    dynamic_executor::~dynamic_executor() = default;
    //-----------------------------------------------------------

    bool dynamic_executor::execute(const std::shared_ptr<mailbox> & mbox)
    {
        YATO_REQUIRES(mbox != nullptr);
        {
            std::unique_lock<std::mutex> lock(mbox->mutex);
            if (!mbox->is_scheduled) {
                if ((!mbox->queue.empty() && mbox->is_open) || (!mbox->sys_queue.empty())) {
                    mbox->is_scheduled = true;
                    m_tpool->enqueue(mailbox_function, this, mbox, m_throughput);
                }
            }
        }
        return true;
    }

} // namespace actors

} // namespace yato

