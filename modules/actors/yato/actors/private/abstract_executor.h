/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
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
         * @return false in the case of error
         */
        virtual bool execute(const std::shared_ptr<mailbox> & mbox) = 0;
    };


    inline 
    details::process_result process_all_system_messages(const std::shared_ptr<mailbox> & mbox) {
        using details::process_result;
        for(;;) {
            auto sys_msg = mbox->pop_system_message();
            if(sys_msg) {
                const process_result res = mbox->owner->receive_system_message_(std::move(*sys_msg));
                if(process_result::keep_running != res) {
                    return res;
                }
            } else {
                // no messages anymore
                break;
            }
        }
        return process_result::keep_running;
    }

}// namespace actors

}// namespace yato

#endif //_YATO_ABSTRACT_EXECUTOR_H_

