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



