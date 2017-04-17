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
    actor_base::~actor_base()
    { }
    //-------------------------------------------------------

    void actor_base::init_base(const actor_ref & ref) {
        //m_context = std::make_unique<actor_context>(ref);
        m_context = new actor_context{ ref };
    }
    //-------------------------------------------------------

    const actor_ref & actor_base::self() const {
        if(m_context == nullptr) {
            throw yato::bad_state_error("Actor is not initialized yet");
        }
        return m_context->self;
    }
    //-------------------------------------------------------

    void actor_base::receive_message(const message & message) noexcept
    {
        assert(m_context != nullptr);
        try {
            unwrap_message(message);
        }
        catch(std::exception & e) {
            m_context->log->error("actor_base[receive_message]: Unhandled exception: %s", e.what());
        }
        catch (...) {
            m_context->log->error("actor_base[receive_message]: Unknown exception!");
        }
    }
    //-------------------------------------------------------


} // namespace actors

} // namespace yato
