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
#include "cell_builder.h"

// ToDo (a.gruzdev): Remove private includes from the public interface
#include "private/message.h"

namespace yato
{
namespace actors
{
    class actor_cell;

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
        /**
         * Pointer to actor's context.
         * Becomes valid only after registraction in the actors system.
         */
        actor_cell* m_context = nullptr; 
        //-------------------------------------------------------

        void stop_impl() noexcept;

        actor_ref create_child_impl_(const std::string & name, const details::cell_builder & builder);

    protected:
        /**
         * Unwrap message and check dynamic type of the payload
         * Apply filter if specified and invoke receive() 
         */
        virtual void do_unwrap_message(message & message) = 0;

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
         * Start watching another actor.
         * If watchee doesn't exist then terminated is sent immediately.
         */
        void watch(const actor_ref & watchee) const;

        /**
         * Stop watching another actor.
         */
        void unwatch(const actor_ref & watchee) const;

        /**
         * Create a child actor
         */
        template <typename ActorType_, typename... Args_>
        actor_ref create_child(const std::string & name, Args_ && ... args) {
            return create_child_impl_(name, details::make_cell_builder<ActorType_>(std::forward<Args_>(args)...));
        }

        // Internal methods 
        // ToDo (a.gruzdev): To be hidden from user

        /**
         * Handle message
         */
        void receive_message(message & msg) noexcept;

        /**
         * Handle system message
         */
        bool receive_system_message(message & msg) noexcept;

        /**
         * Used by actor system to initialize the actor
         */
        void init_base_(actor_cell* cell);

        // Temporal
        actor_cell & context();
    };
    //-------------------------------------------------------



    template <typename MailboxFilter_ = mailbox_no_filter>
    class basic_actor
        : public actor_base
    {
        using this_type = basic_actor<MailboxFilter_>;
    public:

        using filter_type = MailboxFilter_;
        using received_type = typename filter_type::received_type;
        //-------------------------------------------------------

    private:
        const actor_ref* m_sender;
        //-------------------------------------------------------

        void unwrap_message_impl(mailbox_no_filter, message & message) 
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
        void unwrap_message_impl(mailbox_filter<Alternatives_...>, message & message) 
        {
            (void)message;
            //ToDo (a.gruzdev): To be implemetned
            throw std::logic_error("Filtered receive is not implemented");
        }

        /**
         * Unwrap message and check dynamic type of the payload
         * Apply filter if specified and invoke receive()
         */
        void do_unwrap_message(message & message) override final
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
        virtual void receive(received_type & message) = 0;

        //-------------------------------------------------------

        /**
         * Optional pre-start hook
         * Is called before first message
         */
        void pre_start() override { }

        /**
         * Optional post-stop hook
         * Is called after the last message
         */
        void post_stop() override { }


        //-------------------------------------------------------

        const actor_ref & sender() const {
            assert(m_sender != nullptr);
            return *m_sender;
        }

        //-------------------------------------------------------

    public:
        basic_actor() = default;
        ~basic_actor() = default;
        //-------------------------------------------------------

        basic_actor(const basic_actor&) = delete;
        basic_actor(basic_actor&&) = delete;
        //-------------------------------------------------------

        basic_actor& operator= (const basic_actor&) = delete;
        basic_actor& operator= (basic_actor&&) = delete;
        //-------------------------------------------------------
    };
    //-------------------------------------------------------

    // General actor type
    using actor = basic_actor<>;

}// namespace actors

}// namespace yato

#endif // _YATO_ACTOR_H_

