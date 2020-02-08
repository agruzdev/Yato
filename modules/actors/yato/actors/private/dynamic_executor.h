/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_DYNAMIC_EXECUTOR_H_
#define _YATO_ACTORS_DYNAMIC_EXECUTOR_H_

#include "abstract_executor.h"

namespace yato
{
namespace actors
{

    class thread_pool;

    /**
     * Executes actors on a thread pool of fixed size
     */
    class dynamic_executor
        : public abstract_executor
    {
    private:
        actor_system* m_system;
        std::unique_ptr<thread_pool> m_tpool;
        uint32_t m_throughput;
        //------------------------------------------------------------

        static void mailbox_function(dynamic_executor* executor, const std::shared_ptr<mailbox> & mbox, uint32_t throughput);
        //------------------------------------------------------------

    public:
        dynamic_executor(actor_system* system, uint32_t threads_num, uint32_t throughput);
        ~dynamic_executor();

        dynamic_executor(const dynamic_executor&) = delete;
        dynamic_executor& operator=(const dynamic_executor&) = delete;

        dynamic_executor(dynamic_executor&&) = delete;
        dynamic_executor& operator=(dynamic_executor&&) = delete;

        bool execute(const std::shared_ptr<mailbox> & mbox) override;
    };


} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_DYNAMIC_EXECUTOR_H_
