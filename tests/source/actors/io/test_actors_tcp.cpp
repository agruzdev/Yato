#include "gtest/gtest.h"

#include <thread>
#include <iostream>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>
#include <yato/any_match.h>

#include <yato/actors/io/tcp.h>

namespace
{
    class TcpEchoServer
        : public yato::actors::actor<>
    {
        void receive(const yato::any & message) override {
            using namespace yato::actors::io;
            log().info(message.type().name());
            yato::any_match(
                [this](const tcp::bound & bound) {
                    log().info("Bound. Actor %s Address %s:%d", bound.listener.get_name().c_str(), bound.address.host.c_str(), bound.address.port);
                },
                [this](const tcp::connected & connected) {
                    log().info("New connection. Address %s:%d", connected.remote.host.c_str(), connected.remote.port);
                    sender().tell(tcp::assign(self())); // Register self as handler for messages
                },
                [this](const tcp::received & received) {
                    log().info("Received: %s", std::string(received.data.cbegin(), received.data.cend()).c_str());
                    sender().tell(tcp::write(received.data));
                },
                [this](const tcp::peer_closed &) {
                    log().info("Disconnected");
                },
                [this](const tcp::command_fail & fail) {
                    log().error("Fail. Reason: %s", fail.reason.c_str());
                }
            )(message);
        }
    };
}

TEST(Yato_Actors, io_tcp)
{
    using namespace yato::actors;

    auto conf = config::defaults();
    conf.log_filter = log_level::verbose;
    conf.enable_io = true;

    actor_system system("default", conf);

    auto manager = io::tcp::get_for(system);
    ASSERT_NE(manager, system.dead_letters());

    auto server = system.create_actor<TcpEchoServer>("server");

    manager.tell(io::tcp::bind(server, io::inet_address("localhost", 9001)));

    std::this_thread::sleep_for(std::chrono::seconds(2));
    server.tell(poison_pill);
}


