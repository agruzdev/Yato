/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include "../actor.h"
#include "../actor_ref.h"
#include "mailbox.h"

#include "actor_cell.h"

namespace yato
{
namespace actors
{

    actor_cell::actor_cell(actor_system & system, const actor_path & path, std::unique_ptr<actor_base> && instance)
        : m_system(system), m_self(&system, path)
    {
        m_log = logger_factory::create(std::string("Actor[") + m_self.name() + "]");

        // setup actor
        m_actor = std::move(instance);
        m_actor->init_base_(this);

        // create mailbox
        m_mailbox = std::make_shared<mailbox>();
        m_mailbox->owner = m_actor.get();

        m_self.set_mailbox(m_mailbox);
    }
    //--------------------------------------------

    actor_cell::~actor_cell()
    { }
    //--------------------------------------------

} // namespace actors

} // namespace yato
