/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_H_
#define _YATO_ACTOR_H_

#include <yato/any.h>
#include <yato/variant.h>
#include "message.h"

namespace yato
{
namespace actors
{

    /**
     * Unique handle of an actor
     */
    struct actor_ref
    {
        std::string name;
        std::string path;
    };
    //-------------------------------------------------------


    /**
     * Helper structs to specify mailbox filter for the actor
     */
    template <typename... Types_>
    struct mailbox_filter
    {
        using received_type = yato::variant<Types_...>;
    };

    struct mailbox_no_filter
    {
        using received_type = yato::any;
    };
    //-------------------------------------------------------


    /**
     * Base class for actor
     * Implements generic interface for receiveing messages
     */
    class actor_base
    {
    private:
        std::string m_name = "Unnamed";
        //-------------------------------------------------------

    protected:
        /**
         * Unwrap message and check dynamic type of the payload
         * Apply filter if specified and invoke receive() 
         */
        virtual void unwrap_message(const message & message) = 0;
        //-------------------------------------------------------

    public:
        actor_base() = default;
        virtual ~actor_base() = default;

        /**
         * Handle message
         */
        void receive_message(const message & message) noexcept;


        void set_name(const std::string & name) {
            m_name = name;
        }
    };
    //-------------------------------------------------------



    template <typename MailboxFilter_ = mailbox_no_filter>
    class actor
        : public actor_base
    {
        using this_type = actor<MailboxFilter_>;
    public:

        using filter_type = MailboxFilter_;
        using received_type = typename filter_type::received_type;
        //-------------------------------------------------------

    private:


        void unwrap_message_impl(mailbox_no_filter, const message & message) 
        {
            receive(message.payload);
        }

        template <typename... Alternatives_>
        void unwrap_message_impl(mailbox_filter<Alternatives_...>, const message & message) 
        {
            (void)message;
            //ToDo (a.gruzdev): To be implemetned
        }

        /**
         * Unwrap message and check dynamic type of the payload
         * Apply filter if specified and invoke receive()
         */
        void unwrap_message(const message & message) override
        {
            unwrap_message_impl(filter_type{}, message);
        }
    protected:

        virtual void receive(const received_type & message) = 0;
        //-------------------------------------------------------

    public:
        actor() = default;
        //explicit
        //actor(const std::string & name) 
        //    : m_name(name)
        //{ }
        //-------------------------------------------------------

        ~actor() = default;
        //-------------------------------------------------------

        actor(const actor&) = delete;
        actor(actor&&) = delete;
        //-------------------------------------------------------

        actor& operator= (const actor&) = delete;
        actor& operator= (actor&&) = delete;
        //-------------------------------------------------------
    };
    //-------------------------------------------------------

}// namespace actors

}// namespace yato

#endif // _YATO_ACTOR_H_

