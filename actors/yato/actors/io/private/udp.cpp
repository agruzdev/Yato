/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <boost/asio.hpp>

#include <yato/any_match.h>

#include "../../actor_system.h"
#include "../../private/actor_system_ex.h"
#include "../udp.h"

#include "udp_remote.h"

namespace yato
{
namespace actors
{
namespace io
{

    struct udp_context
    {
        std::shared_ptr<boost::asio::io_service> io_service;
        std::unique_ptr<boost::asio::io_service::work> io_work;
        std::thread io_thread;
    };
    //------------------------------------------------------

    const char* udp_manager::actor_name()
    {
        return "udp";
    }
    //-----------------------------------------------------

    udp_manager::udp_manager()
    { }
    //-----------------------------------------------------

    udp_manager::~udp_manager()
    { }
    //-----------------------------------------------------

    void udp_manager::pre_start() {
        m_context = std::make_unique<udp_context>();
        m_context->io_service = std::make_unique<boost::asio::io_service>();
        m_context->io_work = std::make_unique<boost::asio::io_service::work>(*m_context->io_service);
        m_context->io_thread = std::thread([this]{
            log().debug("IO start");
            m_context->io_service->run();
            log().debug("IO stop");
        });
    }
    //------------------------------------------------------

    void udp_manager::receive(yato::any & message) {
        yato::any_match(
            [this](const udp::bind & bind) {
                log().debug("Bind");
                auto connection = std::make_unique<udp_connection>(bind.handler, *m_context->io_service, bind.address);

                const std::string remote_name = bind.address.to_string();
                actor_system_ex::create_actor<udp_remote>(system(), self(), remote_name, std::move(connection));

                bind.handler.tell(udp::bound(bind.address), self());
            }
        )(message);
    }
    //------------------------------------------------------

    void udp_manager::post_stop() {
        m_context->io_work.reset();
        m_context->io_service->stop();
        m_context->io_thread.join();
        m_context.reset();
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

