/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <thread>
#include <iostream>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>
#include <yato/any_match.h>

#include <yato/actors/io/tcp.h>

#include "../test_actors_common.h"

namespace
{

    class EchoSession
        : public yato::actors::actor
    {
        void receive(yato::any && message) override {
            using namespace yato::actors::io;
            log().info(message.type().name());
            yato::any_match(
                [this](const tcp::received & received) {
                    std::string msg = std::string(received.data.cbegin(), received.data.cend());
                    while(!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) {
                        msg.pop_back();
                    }
                    log().info("Received: %s", msg.c_str());
                    if(msg == "exit") {
                        self().stop();
                    }
                    sender().tell(tcp::write(received.data));
                },
                [this](const tcp::peer_closed &) {
                    log().info("Disconnected");
                    self().stop();
                }
            )(message);
        }
    };

    class TcpEchoServer
        : public yato::actors::actor
    {
        uint32_t m_counter = 0;

        void receive(yato::any && message) override {
            using namespace yato::actors::io;
            log().info(message.type().name());
            yato::any_match(
                [this](const tcp::bound & bound) {
                    log().info("Bound. Address %s:%d", bound.local.host.c_str(), bound.local.port);
                },
                [this](const tcp::connected & connected) {
                    log().info("New connection. Address %s:%d", connected.remote.host.c_str(), connected.remote.port);
                    const auto session = create_child<EchoSession>("Session_" + std::to_string(m_counter++));
                    sender().tell(tcp::assign(session)); // Register session actor as handler for messages
                },
                [this](const tcp::command_fail & fail) {
                    log().error("Fail. Reason: %s", fail.reason.c_str());
                    self().stop();
                }
            )(message);
        }
    };
}

TEST(Yato_Actors, io_tcp_server)
{
    using namespace yato::actors;

    auto conf_builder = yato::conf::manual_builder::object();
    conf_builder.put("log_level", "verbose");
    conf_builder.put("enable_io", true);

    actor_system system("default", conf_builder.create());

    actor_ref manager;
    ASSERT_NO_THROW(manager = io::tcp::get_for(system));

    auto server = system.create_actor<TcpEchoServer>("TcpServer");

    manager.tell(io::tcp::bind(server, io::inet_address("localhost", 9001)));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    server.tell(poison_pill);
}

namespace {
    class TcpClient
        : public yato::actors::actor
    {
        void receive(yato::any && message) override {
            using namespace yato::actors::io;
            log().info(message.type().name());
            yato::any_match(
                [this](const tcp::connected & connected) {
                    log().info("Connected to %s:%d", connected.remote.host.c_str(), connected.remote.port);
                    sender().tell(tcp::assign(self()));
                },
                 [this](const tcp::received & received) {
                    std::string msg = std::string(received.data.cbegin(), received.data.cend());
                    while(!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) {
                        msg.pop_back();
                    }
                    log().info("Received: %s", msg.c_str());
                    if(msg == "exit") {
                        self().stop();
                    }
                    sender().tell(tcp::write(received.data));
                },
                [this](const tcp::peer_closed &) {
                    log().info("Disconnected");
                    self().stop();
                },
                [this](const tcp::command_fail & fail) {
                    log().error("Fail. Reason: %s", fail.reason.c_str());
                    self().stop();
                }
            )(message);
        }
    };
}


TEST(Yato_Actors, io_tcp_client)
{
    using namespace yato::actors;

    auto conf_builder = yato::conf::manual_builder::object();
    conf_builder.put("log_level", "verbose");
    conf_builder.put("enable_io", true);

    actor_system system("default", conf_builder.create());

    actor_ref manager;
    ASSERT_NO_THROW(manager = io::tcp::get_for(system));

    auto client = system.create_actor<TcpClient>("client");

    manager.tell(io::tcp::connect(client, io::inet_address("localhost", 9001)));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.tell(poison_pill);
}

