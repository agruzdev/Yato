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

        virtual ~abstract_executor() = default;

        /**
         * Execute full mailbox or a part of it
         */
        virtual bool execute(const std::shared_ptr<mailbox> & mbox) = 0;
    };


    inline 
    bool process_all_system_messages(const std::shared_ptr<mailbox> & mbox) {
        for(;;) {
            std::unique_ptr<message> sys_msg = nullptr;
            {
                std::unique_lock<std::mutex> lock(mbox->mutex);
                if (mbox->sys_queue.empty()) {
                    break;
                }
                sys_msg = std::move(mbox->sys_queue.front());
                mbox->sys_queue.pop();
            }
            if (sys_msg) {
                if(mbox->owner->receive_system_message(*sys_msg)) {
                    return true;
                }
            }
        }
        return false;
    }

}// namespace actors

}// namespace yato

#endif //_YATO_ABSTRACT_EXECUTOR_H_

