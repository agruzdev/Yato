/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include "../actor.h"

#include "actor_context.h"
#include "message.h"

namespace yato
{
namespace actors
{

    actor_base::actor_base()
    { }
    //-------------------------------------------------------

    actor_base::~actor_base()
    { }
    //-------------------------------------------------------

    void actor_base::init_base(const actor_ref & ref) 
    {
        m_context = std::make_unique<actor_context>(ref);
    }
    //-------------------------------------------------------

    const actor_ref & actor_base::self() const 
    {
        if(m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return m_context->self;
    }
    //-------------------------------------------------------

    const logger & actor_base::log() const
    {
        if (m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        assert(m_context->log != nullptr);
        return *m_context->log;
    }
    //-------------------------------------------------------

    void actor_base::receive_message(const message & message) noexcept
    {
        assert(m_context != nullptr);
        do_unwrap_message(message);
    }
    //-------------------------------------------------------

    void actor_base::recieve_system_message(const system_signal& signal) noexcept
    {
        assert(m_context != nullptr);
        do_process_system_message(signal);
    }
    //-------------------------------------------------------

} // namespace actors

} // namespace yato
