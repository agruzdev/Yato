#include "gtest/gtest.h"

#include <thread>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>
#include <yato/any_match.h>

namespace
{
    class EchoActor
        : public yato::actors::actor<>
    {
        void pre_start() override 
        {
            log().info("pre_start() is called!");
        }

        void receive(const yato::any& message) override
        {
            yato::any_match(
                [this](const std::string & str) {
                    log().info(str);
                },
                [this](yato::match_default_t) {
                    log().error("Unknown message!");
                }
            )(message);
            sender().tell("Nobody will hear me");
        }

        void post_stop() override
        {
            log().info("post_stop() is called!");
        }
    };
}

TEST(Yato_Actors, common)
{

    yato::actors::actor_system system("default");

    auto actor = system.create_actor<EchoActor>("echo1");
    actor.tell(std::string("Hello, Actor!"));

    actor.tell(yato::actors::poison_pill);
}



namespace
{
    const int PING_LIMIT = 100000;
    const int PING_TICK  = PING_LIMIT / 10;

    class PingActor
        : public yato::actors::actor<>
    {
        yato::actors::actor_ref m_other;
        void pre_start() override
        {
            log().info("Ping 0");
            m_other.tell(1, self());
        }

        void receive(const yato::any& message) override
        {
            yato::any_match(
                [this](int count) {
                    sender().tell(count + 1, self());
                    if(count >= PING_LIMIT) {
                        self().stop();
                    }
                    else {
                        if (count % PING_TICK == 0) {
                            log().info("Ping " + std::to_string(count));
                        }
                    }
                }
            )(message);
        }

    public:
        explicit
        PingActor(const yato::actors::actor_ref & other)
            : m_other(other)
        { }

    };


    class PongActor
        : public yato::actors::actor<>
    {

        void receive(const yato::any& message) override
        {
            yato::any_match(
                [this](int count) {
                    sender().tell(count + 1, self());
                    if (count >= PING_LIMIT) {
                        self().stop();
                    }
                    else {
                        if (count % PING_TICK == 1) {
                            log().info("Pong " + std::to_string(count));
                        }
                    }
                }
            )(message);
        }
    };
}

TEST(Yato_Actors, ping_pong) 
{
    yato::actors::actor_system system("default");

    auto actor1 = system.create_actor<PongActor>("Actor1");
    auto actor2 = system.create_actor<PingActor>("Actor2", actor1);
}

