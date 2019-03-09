/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/

#include "../actor_system.h"

#include "dynamic_executor.h"

#include "actor_system_ex.h"
#include "actor_cell.h"
#include "mailbox.h"
#include "thread_pool.h"

namespace yato
{
namespace actors
{

    void dynamic_executor::mailbox_function(dynamic_executor* executor, const std::shared_ptr<mailbox> & mbox, uint32_t throughput)
    {
        using details::process_result;
        const actor_ref ref = mbox->owner_actor()->self();
        for(uint32_t count = 0;;) {
            bool is_system_message = false;
            std::unique_ptr<message> message = mbox->pop_prioritized_message(&is_system_message);
            if(!message) {
                // Empty mailbox
                // Terminate batch
                break;
            }
            if(is_system_message) {
                if(process_result::request_stop == mbox->owner_actor()->receive_system_message_(std::move(*message))) {
                    // Terminate actor
                    mbox->close();
                    actor_system_ex::notify_on_stop(*executor->m_system, ref);
                    return;
                }
            }
            else { // user message
                mbox->owner_actor()->receive_message_(std::move(*message));
                if(++count >= throughput) {
                    // Terminate batch
                    break;
                }
            }
        }
        // Try reschedule if not empty
        mbox->schedule_for_execution(true);
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
        m_tpool->enqueue(mailbox_function, this, mbox, m_throughput);
        return true;
    }

} // namespace actors

} // namespace yato

