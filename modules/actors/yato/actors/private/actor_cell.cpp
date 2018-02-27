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
#include "../actor_system.h"

namespace yato
{
namespace actors
{

    actor_cell::actor_cell(actor_system & system, const actor_path & path, std::unique_ptr<basic_actor> && instance)
        : m_system(system), m_self(&system, path), m_started(false), m_stop(false)
    {
        m_log = logger_factory::create(std::string("Actor[") + m_self.name() + "]");
        m_log->set_filter(m_system.logger()->get_filter());

        // setup actor
        m_actor = std::move(instance);
        m_actor->set_context_(this);

        // create mailbox
        m_mailbox = std::make_shared<mailbox>();
        m_mailbox->owner = m_actor.get();

        m_self.set_mailbox(m_mailbox);
    }
    //--------------------------------------------

    actor_cell::~actor_cell()
    { }
    //--------------------------------------------

    actor_ref actor_cell::add_child(std::unique_ptr<actor_cell> && child)
    {
        assert(child->m_parent == nullptr);

        child->m_parent = std::make_unique<actor_ref>(m_self);
        auto child_ref = child->ref();
        m_children.push_back(std::move(child));
        return child_ref;
    }
    //--------------------------------------------

    void actor_cell::remove_child(const actor_ref & ref)
    {
        auto it = std::find_if(m_children.begin(), m_children.end(), [&ref](const std::unique_ptr<actor_cell> & cell) { return cell->ref() == ref; });
        assert(it != m_children.end());
        if(it != m_children.end()) {
            m_children.erase(it);
        }
    }
    //--------------------------------------------

} // namespace actors

} // namespace yato
