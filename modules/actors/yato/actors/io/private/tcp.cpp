/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#include <string>
#include <map>

#include <asio.hpp>

#include <yato/any_match.h>

#include "../../actor_system.h"
#include "../../private/actor_system_ex.h"
#include "../../private/actors/group.h"

#include "../tcp.h"
#include "context.h"
#include "tcp_listener.h"

namespace yato
{
namespace actors
{
namespace io
{
    const char* tcp_manager::actor_name()
    {
        return "tcp";
    }
    //-----------------------------------------------------

    tcp_manager::tcp_manager(const std::shared_ptr<io_context> & io_context)
        : m_context(io_context)
    { }
    //-----------------------------------------------------

    tcp_manager::~tcp_manager() = default;
    //-----------------------------------------------------

    void tcp_manager::pre_start()
    { }
    //-----------------------------------------------------

    void tcp_manager::receive(yato::any && message)
    {
        yato::any_match(
            [this](const tcp::bind & bind) {
                log().debug("Bind");

                auto address = bind.address;

                asio::error_code err;
                asio::ip::tcp::resolver resolver(m_context->service());
                asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), address.host, std::to_string(address.port));
                auto endpoint = *resolver.resolve(query, err);
                if (err) {
                    bind.handler.tell(tcp::command_fail("Failed to resolve endpoint!"), self());
                    return;
                }

                const auto name = address.host + ":" + std::to_string(address.port);
                const auto listener = actor_system_ex::create_actor<tcp_listener>(system(), self(), name, bind.handler, m_context, endpoint);
                system().watch(bind.handler, listener);

                bind.handler.tell(tcp::bound(address), self());
            },
            [this](const tcp::connect & connect) {
                if(connect.handler.empty() || connect.handler == system().dead_letters()) {
                    log().error("Connection handler is empty.");
                    return;
                }
                const auto & remote = connect.address;
                log().debug("Connect " + remote.to_string());

                asio::error_code err;
                asio::ip::tcp::resolver resolver(m_context->service());
                asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), remote.host, std::to_string(remote.port));
                auto endpoint = *resolver.resolve(query, err);
                if (err) {
                    connect.handler.tell(tcp::command_fail("Failed to resolve endpoint! Address = " + remote.to_string()), self());
                    return;
                }

                auto connection = std::make_shared<tcp_connection>(connect.handler, m_context->service());

                // ToDo (a.gruzdev): Is async connect necessary?
                connection->socket().connect(endpoint, err);
                if(err) {
                    connect.handler.tell(tcp::command_fail("Failed to connect endpoint! Address = " + remote.to_string()), self());
                    return;
                }

                const std::string remote_name = remote.to_string();
                actor_system_ex::create_actor<tcp_remote>(system(), self(), remote_name, connection);
            }
        )(message);
    }
    //-----------------------------------------------------

    void tcp_manager::post_stop()
    { }
    //-----------------------------------------------------

    actor_ref tcp::get_for(const actor_system & sys)
    {
        // ToDo (a.gruzdev): Add additional name->ref registration in the actor system for fast lookup?
        YATO_CONSTEXPR_VAR auto timeout = std::chrono::seconds(5);
        auto manager = sys.find(actor_path(sys, actor_scope::system, tcp_manager::actor_name()), timeout).get();
        if(manager.empty()) {
            throw yato::runtime_error("yato::actors::io::tcp[get_for]: Tcp manager doesn't exist for actor_system \"" + sys.name() + "\"");
        }
        return manager;
    }

}// namespace io

}// namespace actors

}// namespace yato

