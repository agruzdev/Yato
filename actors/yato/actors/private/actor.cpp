/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include "../actor.h"
#include "../message.h"
#include "../logger.h"

namespace yato
{
namespace actors
{

    void actor_base::init_base(const std::string & name) {
        m_name = name;
        m_log  = logger_factory::create(m_name);
    }
    //-------------------------------------------------------

    void actor_base::receive_message(const message & message) noexcept
    {
        assert(!m_name.empty());
        assert(m_log != nullptr);
        try {
            unwrap_message(message);
        }
        catch(std::exception & e) {
            m_log->error("actor_base[receive_message]: Unhandled exception: %s", e.what());
        }
        catch (...) {
            m_log->error("actor_base[receive_message]: Unknown exception!");
        }
    }
    //-------------------------------------------------------



} // namespace actors

} // namespace yato
