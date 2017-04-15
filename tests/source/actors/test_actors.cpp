#include "gtest/gtest.h"


#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>

namespace
{
    class EchoActor
        : public yato::actors::actor<>
    {
        yato::actors::logger_ptr m_log = yato::actors::logger_factory::create("Echo");

        void receive(const yato::any& message) override
        {
            try {
                m_log->info(yato::any_cast<std::string>(message));
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
