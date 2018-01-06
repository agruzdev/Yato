/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <yato/any_match.h>

#include "../actor_system_ex.h"
#include "../system_message.h"

#include "guardian.h"
#include "root.h"

namespace yato
{
namespace actors
{

    actor_ref root::create_guard_(const actor_path & path)
    {
        auto cell = std::make_unique<actor_cell>(system(), path, std::make_unique<guardian>());
        auto ref  = cell->ref();

        log().debug("Guard %s is created.", path.c_str());
        actor_system_ex::send_system_message(system(), self(), system_message::attach_child(std::move(cell)));

        return ref;
    }
    //-----------------------------------------------------------

    void root::pre_start()
    {
        m_sys_guard = create_guard_(actor_path(self().get_path().to_string() + "/system"));
        watch(m_sys_guard);

        m_tmp_guard = create_guard_(actor_path(self().get_path().to_string() + "/temp"));
        watch(m_tmp_guard);

        m_usr_guard = create_guard_(actor_path(self().get_path().to_string() + "/user"));
        watch(m_usr_guard);
    }
    //-----------------------------------------------------------

    // ToDo (a.gruzdev): Add processing of unexpected termination of the guards!
    void root::receive(yato::any & message)
    {
        yato::any_match(
            [this] (root_add & add) {
                YATO_REQUIRES(add.cell != nullptr);
                path_elements elems;
                if(!add.cell->ref().get_path().parce(elems, true)) {
                    log().error("Invalid actor path: %s", add.cell->ref().get_path().c_str());
                    return;
                }

                log().verbose("Adding " + add.cell->ref().get_path().to_string());

                switch(elems.scope) {
                case actor_scope::user :
                    actor_system_ex::send_system_message(system(), m_usr_guard, system_message::attach_child(std::move(add.cell)));
                    break;
                case actor_scope::system :
                    actor_system_ex::send_system_message(system(), m_sys_guard, system_message::attach_child(std::move(add.cell)));
                    break;
                case actor_scope::temp :
                    actor_system_ex::send_system_message(system(), m_tmp_guard, system_message::attach_child(std::move(add.cell)));
                    break;
                default:
                    log().error("root_add: Invalid scope!");
                    break;
                }
            },
            [this] (const root_terminate &) {
                log().debug("Terminating root");
                actor_system_ex::send_system_message(system(), m_usr_guard, system_message::stop_after_children{});
            },
            [this](const terminated & t) {
                log().debug("Terminated " + t.ref.get_path().to_string());
                if(t.ref == m_sys_guard) {
                    actor_system_ex::send_system_message(system(), self(), system_message::stop_after_children{});
                }
                else if (t.ref == m_usr_guard) {
                    actor_system_ex::send_system_message(system(), m_sys_guard, system_message::stop{});
                    actor_system_ex::send_system_message(system(), m_tmp_guard, system_message::stop_after_children{});
                }
            },
            [this](yato::match_default_t) {
                YATO_ASSERT(false, "Unknown root message!");
                log().error("Unknown root message!");
            }
        )(message);
    }

} // namespace actors

} // namespace yato
