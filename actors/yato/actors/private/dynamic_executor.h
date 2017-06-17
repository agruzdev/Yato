/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
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
        std::unique_ptr<thread_pool> m_tpool;
        uint32_t m_throughput;

    public:
        dynamic_executor(uint32_t threads_num, uint32_t throughput);
        ~dynamic_executor();

        dynamic_executor(const dynamic_executor&) = delete;
        dynamic_executor& operator=(const dynamic_executor&) = delete;

        bool execute(mailbox* mbox) override;
    };


} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_DYNAMIC_EXECUTOR_H_
