/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <future>

#include "../logger.h"
#include "pinned_executor.h"

namespace yato
{
namespace actors
{
    static
    void pinned_thread_function(const logger_ptr & log, mailbox* mbox) noexcept
    {
        try {
            for (;;) {
                process_all_system_messages(mbox);

                std::unique_ptr<message> msg = nullptr;
                {
                    std::unique_lock<std::mutex> lock(mbox->mutex);

                    while (mbox->queue.empty() && mbox->is_open) {
                        mbox->condition.wait(lock);
                    }

                    if ((mbox->queue.empty() && (!mbox->is_open))) {
                        // Terminate task
                        mbox->is_scheduled = false;
                        break;
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
            log->error("pinned_executor[pinned_thread_function]: Thread failed with exception: %s", e.what());
        }
        catch (...) {
            log->error("pinned_executor[pinned_thread_function]: Thread failed with unknown exception!");
        }
    }
    //-------------------------------------------------------

    pinned_executor::pinned_executor() {
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
        if(!mbox->is_scheduled) {
            mbox->is_scheduled = true;
            m_threads.emplace_back([this, mbox]{ pinned_thread_function(m_logger, mbox); });
        }
        return true;
    }
    //-------------------------------------------------------





} // namespace actors

} // namespace yato

