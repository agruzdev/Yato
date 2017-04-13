#include "gtest/gtest.h"


#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>

namespace
{
    class EchoActor
        : public yato::actors::actor<>
    {
        yato::actors::logger& log = yato::actors::logger::instance();

        void receive(const yato::any& message) override
        {
            try {
                log.info(yato::any_cast<std::string>(message));
            }
            catch(...)
            { }
        }
    };
}

TEST(Yato_Actors, common)
{

    yato::actors::actor_system system("default");

    auto actor = system.create_actor<EchoActor>("echo1");
    actor.tell(std::string("Hello, Actor!"));

}
