/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include "../actor.h"

#include "../actor_system.h"
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

    void actor_base::init_base_(actor_system* system, const actor_ref & ref) 
    {
        m_context = std::make_unique<actor_context>(system, ref);
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
        if (message.payload.type() == typeid(poison_pill_t)) {
            m_context->self.stop();
            return;
        }
        do_unwrap_message(message);
    }
    //-------------------------------------------------------

    void actor_base::recieve_system_message(const system_signal& signal) noexcept
    {
        assert(m_context != nullptr);
        
        switch (signal)
        {
        case yato::actors::system_signal::start:
            try {
                pre_start();
            }
            catch (std::exception & e) {
                log().error("actor[system_signal]: Unhandled exception: %s", e.what());
            }
            catch (...) {
                log().error("actor[system_signal]: Unknown exception!");
            }
            break;
        case yato::actors::system_signal::stop:
            try {
                post_stop();
            }
            catch (std::exception & e) {
                log().error("actor[system_signal]: Unhandled exception: %s", e.what());
            }
            catch (...) {
                log().error("actor[system_signal]: Unknown exception!");
            }
            m_context->system->notify_on_stop_();
            break;
        default:
            assert(false);
            break;
        }
    }
    //-------------------------------------------------------

} // namespace actors

} // namespace yato
