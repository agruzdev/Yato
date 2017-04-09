#include "gtest/gtest.h"


#include <yato/actors/actor_system.h>

namespace
{
    class EchoActor
        : public yato::actors::actor<>
    {
        void receive(const yato::any& message) override
        {
            try {
                std::cout << yato::any_cast<std::string>(message) << std::endl;
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
    system.tell(actor, std::string("Hello, Actor!"));

}
