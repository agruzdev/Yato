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
#include "logger.h"
#include "actor_ref.h"

// ToDo (a.gruzdev): Remove private includes from the public interface
#include "private/message.h"

namespace yato
{
namespace actors
{
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

    struct actor_context;

    /**
     * Base class for actor
     * Implements generic interface for receiveing messages
     */
    class actor_base
    {
    private:
        std::unique_ptr<actor_context> m_context;
        //-------------------------------------------------------

    protected:
        /**
         * Unwrap message and check dynamic type of the payload
         * Apply filter if specified and invoke receive() 
         */
        virtual void do_unwrap_message(const message & message) = 0;

        //-------------------------------------------------------

        /**
        * Optional pre-start hook
        * Is called before first message
        */
        virtual void pre_start() = 0;

        /**
        * Optional post-stop hook
        * Is called after the last message
        */
        virtual void post_stop() = 0;

        /**
         * Get actor logger
         */
        const logger & log() const;
        //-------------------------------------------------------

    public:
        actor_base();

        virtual ~actor_base();

        /**
         * Get self reference
         */
        const actor_ref & self() const;

        /**
         * Get parent actor system
         */
        const actor_system & system() const;

        /**
         * Get parent actor system
         */
        actor_system & system();

        /**
         * Start watching another actor
         */
        void watch(const actor_ref & watchee) const;

        /**
         * Stop watching another actor
         */
        void unwatch(const actor_ref & watchee) const;

        /**
         * Handle message
         */
        void receive_message(const message & msg) noexcept;

        /**
         * Handle system message
         */
        bool recieve_system_message(const message & msg) noexcept;

        /**
         * Used by actor system to initialize the actor
         */
        void init_base_(actor_system* system, const actor_ref & ref);
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
        const actor_ref* m_sender;
        //-------------------------------------------------------

        void unwrap_message_impl(mailbox_no_filter, const message & message) 
        {
            try {
                receive(message.payload);
            }
            catch(std::exception & e) {
                log().error("actor[receive]: Unhandled exception: %s", e.what());
            }
            catch (...) {
                log().error("actor[receive]: Unknown exception!");
            }
        }

        template <typename... Alternatives_>
        void unwrap_message_impl(mailbox_filter<Alternatives_...>, const message & message) 
        {
            (void)message;
            //ToDo (a.gruzdev): To be implemetned
            throw std::logic_error("Filtered receive is not implemented");
        }

        /**
         * Unwrap message and check dynamic type of the payload
         * Apply filter if specified and invoke receive()
         */
        void do_unwrap_message(const message & message) override final
        {
            m_sender = &message.sender;
            unwrap_message_impl(filter_type{}, message);
            // ToDo (a.gruzdev): Use finally
            m_sender = nullptr; 
        }

    protected:

        /**
         * Main method for processing all incoming messages
         */
        virtual void receive(const received_type & message) = 0;

        //-------------------------------------------------------

        /**
         * Optional pre-start hook
         * Is called before first message
         */
        virtual void pre_start() override { }

        /**
         * Optional post-stop hook
         * Is called after the last message
         */
        virtual void post_stop() override { }


        //-------------------------------------------------------

        const actor_ref & sender() const {
            assert(m_sender != nullptr);
            return *m_sender;
        }

        //-------------------------------------------------------

    public:
        actor() = default;
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

