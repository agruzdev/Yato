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
        virtual bool execute(mailbox* mbox) = 0;
    };

}// namespace actors

}// namespace yato

#endif //_YATO_ABSTRACT_EXECUTOR_H_

