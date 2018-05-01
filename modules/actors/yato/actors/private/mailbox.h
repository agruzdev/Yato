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

        std::unique_ptr<message> try_pop_prioritized_message_(bool* is_system);
        //---------------------------------------------------------

        /**
         * Add message to user queue
         * This methos is locking. Do not call during schduling.
         * @return true if mailbox is ready to be scheduled
         */
        bool enqueue_user_message(std::unique_ptr<message> && msg);

        /**
         * Add message to system queue
         * This methos is locking. Do not call during schduling.
         * @return true if mailbox is ready to be scheduled
         */
        bool enqueue_system_message(std::unique_ptr<message> && msg);

        /**
         * Try taking message in priority order
         * Firstly system message, then user message.
         * This methos is locking. Do not call during schduling.
         */
        std::unique_ptr<message> pop_prioritized_message(bool* is_system);

        /**
         * Blocking version of pop_prioritized_message.
         * Waits until there is a message to pop.
         */
        std::unique_ptr<message> pop_prioritized_message_sync(bool* is_system);

        /**
         * Try taking message from the system queue
         * This methos is locking. Do not call during schduling.
         */
        std::unique_ptr<message> pop_system_message();

        /**
         * Schedule mailbox to execution.
         * @param reschedule Execute mailbox even if is already in teh scheduled state.
         *                   This flag is used for rescheduing mailbox inside executor without releasing it.
         * @return flag if mailbox is added to execution schedule
         */
        bool schedule_for_execution(bool reschedule = false);

        /**
         * Close mailbox for any new messages.
         * Mailbox cant be reused after closing.
         * This methos is locking. Do not call during schduling.
         */
        void close();

    };

} // namespace actors

} // namespace yato


#endif //_YATO_ACTORS_MAILBOX_H_
