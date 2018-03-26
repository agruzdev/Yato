/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#include <future>

#include "../actor_system.h"
#include "../logger.h"
#include "actor_system_ex.h"
#include "actor_cell.h"
#include "pinned_executor.h"

namespace yato
{
namespace actors
{

    void pinned_executor::pinned_thread_function(pinned_executor* executor, const std::shared_ptr<mailbox> & mbox) noexcept
    {
        try {
            const actor_ref ref = mbox->owner->self();
            for (;;) {
                if(process_all_system_messages(mbox)) {
                    // Terminate actor
                    {
                        std::unique_lock<std::mutex> lock(mbox->mutex);
                        mbox->is_open = false;
                        mbox->is_scheduled = false;
                    }
                    actor_system_ex::notify_on_stop(*executor->m_system, ref);
                    return;
                }

                if (!mbox->owner->context_().is_started()) {
                    // ToDo (a.gruzdev): Quick solution
                    // dont process user messages untill started
                    continue;
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
                    mbox->owner->receive_message_(std::move(*msg));
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

    bool pinned_executor::execute(const std::shared_ptr<mailbox> & mbox) {
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
