/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#include <yato/any_match.h>

#include "../system_message.h"
#include "../actor_system_ex.h"
#include "selector.h"

namespace yato
{
namespace actors
{

    void selector::pre_start()
    {
        path_elements elems;
        if(!m_target.parce(elems)) {
            self().tell(selection_failure("Invalid target actor path."));
            return;
        }

        auto search_path = std::move(elems.names);
        std::reverse(search_path.begin(), search_path.end());
        search_path.push_back(actor_path::scope_to_str(elems.scope));

        actor_system_ex::send_system_message(system(), actor_system_ex::root(system()), system_message::selection(self(), elems.scope, std::move(search_path)));
    }
    //-----------------------------------------------------

    void selector::receive(yato::any && message)
    {
        yato::any_match(
            [this](const selection_success & succ) {
                if(!m_satisfied) {
                    m_result.set_value(succ.result);
                    m_satisfied = true;
                }
                self().stop();
            },
            [this](const selection_failure & suck) {
                if(!m_satisfied) {
                    m_result.set_value(actor_ref{});
                    m_satisfied = true;
                    log().verbose("Selection failed. Reason: %s", suck.reason.c_str());
                }
                self().stop();
            },
            [this](yato::match_default_t) {
                log().error("Unexpected message!");
            }
        )(message);
    }
    //------------------------------------------------------

    void selector::post_stop()
    {
        if(!m_satisfied) {
            m_result.set_value(actor_ref{});
            m_satisfied = true;
            log().warning("Search was interrupted by timeout.");
        }
    }

} // namespace actors

} // namespace yato
