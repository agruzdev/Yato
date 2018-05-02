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

#include "../actor_common.h"
#include "message.h"

namespace yato
{
namespace actors
{
    class basic_actor;
    class actor_cell;

    // ToDo (a.gruzdev): simplest implementation. Queue + mutex
    class mailbox
        : public std::enable_shared_from_this<mailbox>
    {
        std::queue<std::unique_ptr<message>> m_usr_queue;
        std::queue<std::unique_ptr<message>> m_sys_queue;
        std::mutex m_mutex;
        std::condition_variable m_condition;

        basic_actor* m_owner = nullptr;
        actor_cell* m_owner_node = nullptr;

        //mailbox_status status = mailbox_status::opened;
        bool m_is_open = true;
        bool m_is_scheduled = false;
        //---------------------------------------------------------

        std::unique_ptr<message> try_pop_prioritized_message_(bool* is_system);
        //---------------------------------------------------------

    public:
        /**
         * @param node Related actor_cell
         * @param manual_mode Prevents it from adding to any executor. All messages will be fetched manually.
         */
        mailbox(actor_cell* node, bool manual_mode = false);

        ~mailbox();

        mailbox(const mailbox&) = delete;
        mailbox(mailbox&&) = delete;

        mailbox& operator=(const mailbox&) = delete;
        mailbox& operator=(mailbox&&) = delete;

        /**
         * Get owner actor
         */
        basic_actor* owner_actor() const {
            return m_owner;
        }

        /**
         * Get owner actor cell
         */
        actor_cell* owner_node() const {
            return m_owner_node;
        }

        /**
         * Add message to user queue
         * This method is locking. Do not call during schduling.
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
         * @ToDo Add timeout parameter
         */
        std::unique_ptr<message> pop_prioritized_message_sync(bool* is_system);

        /**
         * Try taking message from the system queue
         * This method is locking. Do not call during schduling.
         */
        //std::unique_ptr<message> pop_system_message();

        /**
         * Try taking message from the user queue
         * This method is locking. Do not call during schduling.
         */
        std::unique_ptr<message> pop_user_message_sync(const timeout_type & timeout);

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
         * This method is locking. Do not call during schduling.
         */
        void close();

    };

} // namespace actors

} // namespace yato


#endif //_YATO_ACTORS_MAILBOX_H_
