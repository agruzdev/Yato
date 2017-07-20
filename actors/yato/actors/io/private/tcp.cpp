/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#include <string>
#include <map>

#include <boost/asio.hpp>

#include <yato/any_match.h>

#include "../../actor_system.h"
#include "../../private/actor_system_ex.h"
#include "../tcp.h"

#include "tcp_listener.h"

namespace yato
{
namespace actors
{
namespace io
{
    struct tcp_context
    {
        std::shared_ptr<boost::asio::io_service> io_service;
        std::unique_ptr<boost::asio::io_service::work> io_work;
        std::thread io_thread;
    };


    const char* tcp_manager::actor_name()
    {
        return "tcpManager";
    }
    //-----------------------------------------------------

    tcp_manager::tcp_manager()
    { }
    //-----------------------------------------------------

    tcp_manager::~tcp_manager()
    { }
    //-----------------------------------------------------

    void tcp_manager::pre_start()
    {
        m_context = std::make_unique<tcp_context>();
        m_context->io_service = std::make_unique<boost::asio::io_service>();
        m_context->io_work = std::make_unique<boost::asio::io_service::work>(*m_context->io_service);
        m_context->io_thread = std::thread([this]{
            log().debug("IO start");
            m_context->io_service->run();
            log().debug("IO stop");
        });
    }
    //-----------------------------------------------------

    void tcp_manager::receive(const yato::any & message)
    {
        yato::any_match(
            [this](const tcp::bind & bind) {
                log().debug("Bind");

                auto address = bind.address;

                boost::system::error_code err;
                boost::asio::ip::tcp::resolver resolver(*m_context->io_service);
                boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), address.host, std::to_string(address.port));
                auto endpoint = *resolver.resolve(query, err);
                if (err) {
                    bind.handler.tell(tcp::command_fail("Failed to resolve endpoint!"), self());
                    return;
                }

                auto name = "tcp/" + address.host + ":" + std::to_string(address.port);
                auto listener = actor_system_ex::create_actor<tcp_listener>(system(), actor_scope::system, name, bind.handler, m_context->io_service, endpoint);
                system().watch(bind.handler, listener);

                bind.handler.tell(tcp::bound(listener, address), self());
            }
        )(message);
    }
    //-----------------------------------------------------

    void tcp_manager::post_stop()
    {
        m_context->io_work.reset();
        m_context->io_service->stop();
        m_context->io_thread.join();
        m_context.reset();
    }
    //-----------------------------------------------------

    actor_ref tcp::get_for(const actor_system & sys)
    {
        auto manager = sys.select(actor_path(sys, actor_scope::system, tcp_manager::actor_name()));
        if(manager == sys.dead_letters()) {
            throw yato::runtime_error("yato::actors::io::tcp[get_for]: Tcp manager doesn't exist for actor_system \"" + sys.name() + "\"");
        }
        return manager;
    }

}// namespace io

}// namespace actors

}// namespace yato

