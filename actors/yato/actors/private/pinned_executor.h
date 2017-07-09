/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_PINNED_EXECUTOR_H_
#define _YATO_ACTORS_PINNED_EXECUTOR_H_

#include <vector>
#include <thread>
#include <future>

#include "abstract_executor.h"

namespace yato
{
namespace actors
{

    struct thread_context;

    /**
     * Simplest thread pool.
     * Creates a new thread for the each mailbox
     */
    class pinned_executor
        : public abstract_executor
    {
    private:
        actor_system* m_system;
        std::vector<std::thread> m_threads;
        logger_ptr m_logger;

        static void pinned_thread_function(pinned_executor* executor, mailbox* mbox) noexcept;

    public:
        pinned_executor(actor_system* system);
        ~pinned_executor();

        pinned_executor(const pinned_executor&) = delete;
        pinned_executor& operator=(const pinned_executor&) = delete;

        bool execute(mailbox* mbox) override;
    };




} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_PINNED_EXECUTOR_H_
