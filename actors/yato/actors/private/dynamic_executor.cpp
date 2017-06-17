/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include "thread_pool.h"
#include "dynamic_executor.h"

namespace yato
{
namespace actors
{

    static
    void mailbox_function(dynamic_executor* executor, mailbox* mbox, uint32_t throughput)
    {
        for(uint32_t count = 0; count < throughput; ++count) {
            std::unique_ptr<message> msg = nullptr;
            {
                std::unique_lock<std::mutex> lock(mbox->mutex);

                //if (mbox->queue.empty() || !mbox->isOpen) {
                if (mbox->queue.empty()) {
                    // Terminate task
                    break;
                }

                msg = std::move(mbox->queue.front());
                mbox->queue.pop();
            }
            if (msg) {
                mbox->owner->receive_message(*msg);
            }
        }
        // Schedule again, moving to the end of the queue
        {
            std::unique_lock<std::mutex> lock(mbox->mutex);
            mbox->isScheduled = false;
        }
        executor->execute(mbox);
    }
    //----------------------------------------------------------



    dynamic_executor::dynamic_executor(uint32_t threads_num, uint32_t throughput)
        : m_throughput(throughput)
    {
        m_tpool = std::make_unique<thread_pool>(threads_num);
    }
    //-----------------------------------------------------------

    dynamic_executor::~dynamic_executor() 
    { }
    //-----------------------------------------------------------

    bool dynamic_executor::execute(mailbox* mbox)
    {
        assert(mbox != nullptr);
        {
            std::unique_lock<std::mutex> lock(mbox->mutex);
            if (!mbox->isScheduled && !mbox->queue.empty() && mbox->isOpen) {
                m_tpool->enqueue(mailbox_function, this, mbox, m_throughput);
            }
        }
        return true;
    }

} // namespace actors

} // namespace yato

