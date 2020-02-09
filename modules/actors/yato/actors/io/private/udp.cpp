/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#include <yato/any_match.h>

#include "../../actor_system.h"
#include "../../private/actor_system_ex.h"
#include "../udp.h"
#include "context.h"

#include "udp_remote.h"

namespace yato
{
namespace actors
{
namespace io
{

    const char* udp_manager::actor_name()
    {
        return "udp";
    }
    //-----------------------------------------------------

    udp_manager::udp_manager(const std::shared_ptr<io_context> & ctx)
        : m_context(ctx)
    { }
    //-----------------------------------------------------

    udp_manager::~udp_manager() = default;
    //-----------------------------------------------------

    void udp_manager::pre_start() {
    }
    //------------------------------------------------------

    void udp_manager::receive(yato::any && message) {
        yato::any_match(
            [this](const udp::bind & bind) {
                log().debug("Bind");
                auto connection = std::make_unique<udp_connection>(bind.handler, m_context->service(), bind.address);

                const std::string remote_name = bind.address.to_string();
                actor_system_ex::create_actor<udp_remote>(system(), self(), remote_name, std::move(connection));

                bind.handler.tell(udp::bound(bind.address), self());
            }
        )(message);
    }
    //------------------------------------------------------

    void udp_manager::post_stop() {
    }
    //-----------------------------------------------------

    actor_ref udp::get_for(const actor_system & sys)
    {
        // ToDo (a.gruzdev): Add additional name->ref registration in the actor system for fast lookup?
        YATO_CONSTEXPR_VAR auto timeout = std::chrono::seconds(5);
        auto manager = sys.find(actor_path(sys, actor_scope::system, udp_manager::actor_name()), timeout).get();
        if(manager.empty()) {
            throw yato::runtime_error("yato::actors::io::udp[get_for]: Tcp manager doesn't exist for actor_system \"" + sys.name() + "\"");
        }
        return manager;
    }


}// namespace io

}// namespace actors

}// namespace yato

