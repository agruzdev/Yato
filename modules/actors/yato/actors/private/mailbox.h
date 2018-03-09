/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_MAILBOX_H_
#define _YATO_ACTORS_MAILBOX_H_


#include <queue>
#include <mutex>
#include <condition_variable>

#include "../actor.h"
#include "message.h"

namespace yato
{
namespace actors
{

#if 0
    enum class mailbox_status
    {
        opened,     ///< Is to be passed to executor
        closed,     ///< Will not get more messages. The rest should be processed
        scheduled   ///< Is being executed
    };
#endif

    // ToDo (a.gruzdev): simplest implementation. Queue + mutex
    struct mailbox
    {
        std::queue<std::unique_ptr<message>> queue;
        std::queue<std::unique_ptr<message>> sys_queue;
        std::mutex mutex;
        std::condition_variable condition;

        basic_actor* owner = nullptr;
        //mailbox_status status = mailbox_status::opened;
        bool is_open = true;
        bool is_scheduled = false;
    };

} // namespace actors

} // namespace yato


#endif //_YATO_ACTORS_MAILBOX_H_
