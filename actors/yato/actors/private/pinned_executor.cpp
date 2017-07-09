/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <future>

#include "../actor_system.h"
#include "../logger.h"
#include "pinned_executor.h"

namespace yato
{
namespace actors
{

    void pinned_executor::pinned_thread_function(pinned_executor* executor, mailbox* mbox) noexcept
    {
        try {
            for (;;) {
                if(process_all_system_messages(mbox)) {
                    // Terminate actor
                    {
                        std::unique_lock<std::mutex> lock(mbox->mutex);
                        mbox->is_open = false;
                        mbox->is_scheduled = false;
                    }
                    actor_ref ref = mbox->owner->self();
                    executor->m_system->notify_on_stop_(ref);
                    return;
                }

                std::unique_ptr<message> msg = nullptr;
                {
                    std::unique_lock<std::mutex> lock(mbox->mutex);

                    while (mbox->queue.empty() && mbox->sys_queue.empty()) {
                        mbox->condition.wait(lock);
                    }

                    if(!mbox->sys_queue.empty()) {
                        continue;
                    }

                    if (!mbox->queue.empty()) {
                        msg = std::move(mbox->queue.front());
                        mbox->queue.pop();
                    }
                }
                if (msg) {
                    mbox->owner->receive_message(*msg);
                }
            }
        }
        catch(std::exception & e) {
            executor->m_logger->error("pinned_executor[pinned_thread_function]: Thread failed with exception: %s", e.what());
        }
        catch (...) {
            executor->m_logger->error("pinned_executor[pinned_thread_function]: Thread failed with unknown exception!");
        }
    }
    //-------------------------------------------------------

    pinned_executor::pinned_executor(actor_system* system)
        : m_system(system)
    {
        m_logger = logger_factory::create("pinned_executor");
    }
    //-------------------------------------------------------

    pinned_executor::~pinned_executor() {
        for(auto & t : m_threads) {
            t.join();
        }
    }
    //-------------------------------------------------------

    bool pinned_executor::execute(mailbox* mbox) {
        std::unique_lock<std::mutex> lock(mbox->mutex);
        if(!mbox->is_scheduled && mbox->is_open) {
            mbox->is_scheduled = true;
            m_threads.emplace_back(&pinned_thread_function, this, mbox);
        }
        mbox->condition.notify_one();
        return true;
    }
    //-------------------------------------------------------





} // namespace actors

} // namespace yato

