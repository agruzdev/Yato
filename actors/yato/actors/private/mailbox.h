/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_MAILBOX_H_
#define _YATO_ACTORS_MAILBOX_H_


#include <queue>
#include <mutex>

#include "../message.h"


namespace yato
{
namespace actors
{

    // ToDo (a.gruzdev): simplest implementation. Queue + mutex
    struct mailbox
    {
        std::queue<std::unique_ptr<message>> queue;
        std::mutex mutex;
        std::condition_variable condition;
    };

} // namespace actors

} // namespace yato


#endif //_YATO_ACTORS_MAILBOX_H_