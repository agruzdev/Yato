/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <cassert>

#include "../actor_ref.h"
#include "../actor_system.h"
#include "mailbox.h"

namespace yato
{
namespace actors
{

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
