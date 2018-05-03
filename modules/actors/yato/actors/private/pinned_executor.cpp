/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#include <future>

#include "../actor_system.h"
#include "../logger.h"

#include "pinned_executor.h"

#include "actor_cell.h"
#include "actor_system_ex.h"
#include "mailbox.h"

namespace yato
{
namespace actors
{

    void pinned_executor::pinned_thread_function(pinned_executor* executor, const std::shared_ptr<mailbox> & mbox) noexcept
    {
        using details::process_result;
        const actor_ref ref = mbox->owner_actor()->self();
        try {
            for (;;) {
                bool is_system_message = false;
                std::unique_ptr<message> message = mbox->pop_prioritized_message_sync(&is_system_message);
                YATO_ASSERT(message != nullptr, "Sync pop cant return null!");
                if(is_system_message) {
                    if(process_result::request_stop == mbox->owner_actor()->receive_system_message_(std::move(*message))) {
                        // Terminate loop
                        break;
                    }
                }
                else {
                    mbox->owner_actor()->receive_message_(std::move(*message));
                }
            }
            mbox->close();
            actor_system_ex::notify_on_stop(*executor->m_system, ref);
        }
        catch(std::exception & e) {
            executor->m_logger->error("pinned_executor[pinned_thread_function]: Thread failed with exception: %s", e.what());
        }
        catch (...) {
            executor->m_logger->error("pinned_executor[pinned_thread_function]: Thread failed with unknown exception!");
        }
    }
    //-------------------------------------------------------

    pinned_executor::pinned_executor(actor_system* system, uint32_t max_threads)
        : m_system(system), m_threads_limit(max_threads)
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

    bool pinned_executor::execute(const std::shared_ptr<mailbox> & mbox)
    {
        if(m_threads.size() >= m_threads_limit) {
            YATO_REQUIRES(mbox->owner_actor() != nullptr);
            m_logger->error("Failed to start thread for actor \"%s\". Threads limit in the pinned_executor is reached!", 
                mbox->owner_actor()->self().get_path().c_str());
            return false;
        }
        m_threads.emplace_back(&pinned_thread_function, this, mbox);
        return true;
    }
    //-------------------------------------------------------





} // namespace actors

} // namespace yato

