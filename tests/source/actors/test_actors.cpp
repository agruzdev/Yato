#include "gtest/gtest.h"


#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>

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
            try {
                log().info(yato::any_cast<std::string>(message));
                sender().tell("Nobody will hear me");
            }
            catch(...)
            { }
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

}



