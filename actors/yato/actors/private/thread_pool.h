/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_THREADPOOL_H_
#define _YATO_ACTORS_THREADPOOL_H_

#include <vector>
#include <thread>
#include <queue>
#include <future>

#include "abstract_executor.h"

namespace yato
{
namespace actors
{

    /**
     * Simplest thread pool.
     * Creates a new thread for the each mailbox
     */
    class pinned_thread_pool
        : public abstract_executor
    {
    private:
        std::vector<std::thread> m_threads;

    public:
        pinned_thread_pool();
        ~pinned_thread_pool();

        pinned_thread_pool(const pinned_thread_pool&) = delete;
        pinned_thread_pool& operator=(const pinned_thread_pool&) = delete;

        bool execute(mailbox* mbox) override;
    };

} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_THREADPOOL_H_
