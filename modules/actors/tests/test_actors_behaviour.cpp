#include "gtest/gtest.h"

#include <thread>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>
#include <yato/actors/inbox.h>
#include <yato/any_match.h>

namespace
{
    class TestActor
        : public yato::actors::actor
    {
        std::unique_ptr<yato::actors::message_consumer> m_ping;
        std::unique_ptr<yato::actors::message_consumer> m_pong;

        void pre_start() override {
            m_ping = yato::actors::make_behaviour(
                yato::any_match(
                    [this](yato::match_default_t) {
                        sender().tell(std::string("ping"));
                        become(m_pong.get());
                    }
                )
            );
            m_pong = yato::actors::make_behaviour(
                [this](yato::any &&) {
                    sender().tell(std::string("pong"));
                    become(m_ping.get());
                }
            );
            become(m_ping.get(), false);
        }

        void receive(yato::any &&) override
        { }

    };
}

TEST(Yato_Actors, behaviour)
{

    yato::actors::actor_system system("default");
    system.logger()->set_filter(yato::actors::log_level::verbose);

    auto actor = system.create_actor<TestActor>("test");

    auto inbox = yato::actors::inbox(system, "in");

    inbox.send(actor, 1);
    inbox.send(actor, 2);
    inbox.send(actor, 3);
    inbox.send(actor, 4);

    auto response = inbox.receive(std::chrono::seconds(1));
    ASSERT_NO_THROW(
        ASSERT_EQ("ping", response.get_as<std::string>())
    );

    response = inbox.receive(std::chrono::seconds(1));
    ASSERT_NO_THROW(
        ASSERT_EQ("pong", response.get_as<std::string>())
    );

    response = inbox.receive(std::chrono::seconds(1));
    ASSERT_NO_THROW(
        ASSERT_EQ("ping", response.get_as<std::string>())
    );

    response = inbox.receive(std::chrono::seconds(1));
    ASSERT_NO_THROW(
        ASSERT_EQ("pong", response.get_as<std::string>())
    );

    actor.tell(yato::actors::poison_pill);
}



namespace
{
    class TestActor2
        : public yato::actors::actor
    {
        std::unique_ptr<yato::actors::message_consumer> m_enabled;

        void enabled(yato::any && msg) {
            yato::any_match(
                [this](int x) {
                    log().info(std::to_string(x));
                },
                [this](std::string && txt) {
                    if(txt == "disable") {
                        log().info("disable");
                        unbecome();
                    }
                }
            )(msg);
        }

        void pre_start() override {
            m_enabled = yato::actors::make_behaviour(std::bind(&TestActor2::enabled, this, std::placeholders::_1));
        }

        void receive(yato::any && msg) override {
            yato::any_match(
                [this](std::string && txt) {
                    if(txt == "enable") {
                        log().info("enable");
                        become(m_enabled.get(), false);
                    }
                },
                [this](yato::match_default_t) {
                    log().info("n/a");
                }
            )(msg);
        }

    };
}


TEST(Yato_Actors, behaviour2)
{

    yato::actors::actor_system system("default");
    system.logger()->set_filter(yato::actors::log_level::verbose);

    auto actor = system.create_actor<TestActor2>("test");

    actor.tell(1);
    actor.tell(2);
    actor.tell(std::string("enable"));
    actor.tell(1);
    actor.tell(4);
    actor.tell(8);
    actor.tell(std::string("disable"));
    actor.tell(2);

    actor.tell(yato::actors::poison_pill);
}
