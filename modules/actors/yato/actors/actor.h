/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_H_
#define _YATO_ACTOR_H_

#include <stack>

#include <yato/any.h>

#include "actor_ref.h"
#include "behaviour.h"
#include "cell_builder.h"
#include "logger.h"

namespace yato
{
namespace actors
{

    namespace details
    {
        enum class process_result
        {
            keep_running,
            request_stop
        };
    }

    struct message;
    class actor_cell;

    /**
     * Base class for actor
     * Implements generic interface for receiveing messages
     */
    class basic_actor
        : public message_consumer
    {
    private:
        using process_result = details::process_result;

        /**
         * Pointer to actor's context.
         * Becomes valid only after registraction in the actors system.
         */
        actor_cell* m_context = nullptr;

        /**
         * Behaviours stack
         */
        std::stack<message_consumer*> m_behaviours{};

        /**
         * Cache for current sender ref
         */
        mutable const actor_ref* m_sender = nullptr;
        //-------------------------------------------------------

        void stop_impl() noexcept;

        actor_ref create_child_impl_(const std::string & name, const details::cell_builder & builder);

        actor_cell & context_();
        //-------------------------------------------------------

    protected:
        // Interface to be implemented by actor

        /**
         * Optional pre-start hook
         * Is called before first message
         */
        virtual void pre_start() { }

        /**
         * Main method for processing all incoming messages
         */
        virtual void receive(yato::any && message) override = 0;

        /**
        * Optional post-stop hook
        * Is called after the last message
        */
        virtual void post_stop() { }

        //-------------------------------------------------------
        // Interface for using by actor

        /**
         * Get actor logger
         */
        const logger & log() const;

        /**
         * Get sender of the current message
         */
        const actor_ref & sender() const {
            assert(m_sender != nullptr);
            return *m_sender;
        }

        /**
         * Forward message keeping original sender
         */
        void forward(yato::any & message, const actor_ref & target) const {
            target.tell(message, sender());
        }

        /**
         * Forward message keeping original sender
         */
        void forward(yato::any && message, const actor_ref & target) const {
            target.tell(std::move(message), sender());
        }

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

        /**
         * Replace the actor's behavior
         * @param new_behaviour
         * @param discard_old if true rewrite old behaviour, otherwise put on the top of behaviours stack
         * @return if discard_old is true then returns replaced behaviour
         */
        message_consumer* become(message_consumer* behaviour, bool discard_old = true) noexcept;

        /**
         * Remove behavior from the top of stack and return it
         */
        message_consumer* unbecome() noexcept;

        //-------------------------------------------------------

    public:
        basic_actor() = default;

        virtual ~basic_actor() = default;

        basic_actor(const basic_actor&) = delete;
        basic_actor(basic_actor&&) = delete;

        basic_actor& operator= (const basic_actor&) = delete;
        basic_actor& operator= (basic_actor&&) = delete;

        /**
         * Get self reference
         */
        const actor_ref & self() const;

        // Internal methods 
        // ToDo (a.gruzdev): To be hidden from user

        /**
         * Handle message
         */
        void receive_message_(message && msg) noexcept;

        /**
         * Handle system message
         */
        process_result receive_system_message_(message && msg) noexcept;

        /**
         * Used by actor system to initialize the actor
         */
        void set_context_(actor_cell* cell);
    };
    //-------------------------------------------------------

    // General actor type
    using actor = basic_actor;

}// namespace actors

}// namespace yato

#endif // _YATO_ACTOR_H_

