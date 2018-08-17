/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ABSTRACT_EXECUTOR_H_
#define _YATO_ABSTRACT_EXECUTOR_H_

#include <memory>

namespace yato
{
namespace actors
{
    class mailbox;

    class abstract_executor  // NOLINT
    {
    public:
        virtual ~abstract_executor() = default;

        /**
         * Execute full mailbox or a part of it.
         * This method is to be called inside mailbox critical section.
         * It may not call any locking methods of the mailbox to avoid deadlocks.
         * The only thing it should do - to save mbox shared pointer for further processing.
         * @return true if mailbox was scheduled
         */
        virtual bool execute(const std::shared_ptr<mailbox> & mbox) = 0;
    };

}// namespace actors

}// namespace yato

#endif //_YATO_ABSTRACT_EXECUTOR_H_

