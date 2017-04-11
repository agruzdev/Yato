/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <iostream>

#include "thread_pool.h"

namespace yato
{
namespace actors
{
    static
    void pinned_thread_function(mailbox* mbox) noexcept
    {
        try {
            for (;;) {
                std::unique_ptr<message> msg = nullptr;
                {
                    std::unique_lock<std::mutex> lock(mbox->mutex);

                    while (mbox->queue.empty() && mbox->isOpen) {
                        mbox->condition.wait(lock);
                    }

                    if (mbox->queue.empty() && (!mbox->isOpen)) {
                        // Terminate thread
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
        catch (...) {
            // ToDo (a.gruzdev): Add logging
            std::cout << "Exception!" << std::endl;
        }
    }
    //-------------------------------------------------------

    pinned_thread_pool::pinned_thread_pool()
    { }

    pinned_thread_pool::~pinned_thread_pool() {
        for(auto & t : m_threads) {
            t.join();
        }
    }

    bool pinned_thread_pool::execute(mailbox* mbox) {
        std::unique_lock<std::mutex> lock(mbox->mutex);
        if(!mbox->isScheduled) {
            m_threads.emplace_back([mbox]{ pinned_thread_function(mbox); });
        }
        return true;
    }
    

} // namespace actors

} // namespace yato

