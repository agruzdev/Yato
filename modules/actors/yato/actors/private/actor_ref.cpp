/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#include <cassert>

#include "../actor_ref.h"
#include "../actor_system.h"
#include "mailbox.h"

namespace yato
{
namespace actors
{

    actor_ref::actor_ref()
        : m_system(nullptr), m_path(std::string())
    { }
    //-------------------------------------------------------

    actor_ref::actor_ref(actor_system* system, const actor_path & path)
        : m_system(system), m_path(path) 
    { }
    //-------------------------------------------------------

    actor_ref::~actor_ref()
    { }
    //-------------------------------------------------------

    void actor_ref::set_mailbox(const std::shared_ptr<mailbox> & ptr) {
        m_mailbox = std::weak_ptr<mailbox>(ptr);
    }
    //-------------------------------------------------------

} // namespace actors

} // namespace yato
