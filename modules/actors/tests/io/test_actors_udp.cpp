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

#include <yato/actors/io/udp.h>

#include "../test_actors_common.h"

namespace
{

    class UdpEchoServer
        : public yato::actors::actor
    {
        void receive(yato::any && message) override {
            using namespace yato::actors::io;
            log().info(message.type().name());
            yato::any_match(
                [this](const udp::bound & bound) {
                    log().info("Bound. Address %s:%d", bound.local.host.c_str(), bound.local.port);
                },
                [this](const udp::received & received) {
                    std::string msg = std::string(received.data.cbegin(), received.data.cend());
                    while(!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) {
                        msg.pop_back();
                    }
                    log().info("Received from <%s> : %s", received.sender.to_string().c_str(), msg.c_str());
                    if(msg == "exit") {
                        self().stop();
                    }
                    sender().tell(udp::send(received.data, received.sender));
                },
                [this](const udp::command_fail & fail) {
                    log().error("Fail. Reason: %s", fail.reason.c_str());
                    self().stop();
                }
            )(message);
        }
    };
}

TEST(Yato_Actors, io_udp_server)
{
    using namespace yato::actors;

    auto conf_builder = yato::config_builder::object();
    conf_builder.put("log_level", "verbose");
    conf_builder.put("enable_io", true);

    actor_system system("default", conf_builder.create());

    actor_ref manager;
    ASSERT_NO_THROW(manager = io::udp::get_for(system));

    auto server = system.create_actor<UdpEchoServer>("UdpServer");

    manager.tell(io::udp::bind(server, io::inet_address("localhost", 9001)));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    server.tell(poison_pill);
}

