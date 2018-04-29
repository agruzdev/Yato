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

    struct execution_context;

    // ToDo (a.gruzdev): simplest implementation. Queue + mutex
    // ToDo (a.gruzdev) : make class
    struct mailbox
        : public std::enable_shared_from_this<mailbox>
    {
        std::queue<std::unique_ptr<message>> queue;
        std::queue<std::unique_ptr<message>> sys_queue;
        std::mutex mutex;
        std::condition_variable condition;

        basic_actor* owner = nullptr;
        actor_cell* owner_node = nullptr;

        //mailbox_status status = mailbox_status::opened;
        bool is_open = true;
        bool is_scheduled = false;
        //---------------------------------------------------------

        /**
         * Add message to user queue
         * @return true if mailbox is ready to be scheduled
         */
        bool enqueue_user_message(std::unique_ptr<message> && msg);

        /**
         * Add message to system queue
         * @return true if mailbox is ready to be scheduled
         */
        bool enqueue_system_message(std::unique_ptr<message> && msg);

        /**
         * Schedule mailbox to execution
         */
        void schedule_for_execution();
    };

} // namespace actors

} // namespace yato


#endif //_YATO_ACTORS_MAILBOX_H_
