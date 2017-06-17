/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ABSTRACT_EXECUTOR_H_
#define _YATO_ABSTRACT_EXECUTOR_H_

#include "mailbox.h"

namespace yato
{
namespace actors
{

    class abstract_executor
    {
    public:
        static constexpr uint32_t DEFAULT_SYSTEM_THROUGHPUT = 10;

        virtual ~abstract_executor() = default;

        /**
         * Execute full mailbox or a part of it
         */
        virtual bool execute(mailbox* mbox) = 0;
    };


    inline 
    void process_all_system_messages(mailbox* mbox, uint32_t throughput = abstract_executor::DEFAULT_SYSTEM_THROUGHPUT) {
        while(throughput--) {
            system_signal signal = system_signal::none;
            {
                std::unique_lock<std::mutex> lock(mbox->mutex);
                if (mbox->sys_queue.empty()) {
                    break;
                }
                signal = mbox->sys_queue.front();
                mbox->sys_queue.pop();
            }
            if (signal != system_signal::none) {
                mbox->owner->recieve_system_message(signal);
            }
        }
    }

}// namespace actors

}// namespace yato

#endif //_YATO_ABSTRACT_EXECUTOR_H_

