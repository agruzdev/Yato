/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <cassert>

#include "../actor_ref.h"
#include "../actor_system.h"

namespace yato
{
namespace actors
{

    actor_ref::actor_ref(actor_system* system, const std::string & name)
        : m_system(system), m_name(name)
    {
        if (m_system != nullptr) {
            m_path = m_system->name() + ":" + m_name;
        } else {
            m_path = m_name;
        }
    }
    //-------------------------------------------------------


} // namespace actors

} // namespace yato
