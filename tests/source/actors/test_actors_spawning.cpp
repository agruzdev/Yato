#include "gtest/gtest.h"

#include <atomic>
#include <thread>

#include <yato/any_match.h>
#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>

namespace
{
    const size_t ECHO_NUM   = 10;
    const size_t ACTORS_NUM = 10000;


    class EchoActor
        : public yato::actors::actor
    {
        uint32_t m_counter = 0;
    public:
        void receive(yato::any & message) override
        {
            for(uint32_t i = 0; i < ECHO_NUM; ++i) {
                sender().tell(message, self());
            }
            ++m_counter;
            if(m_counter % (ACTORS_NUM / 10) == 0) {
                log().info("Echo tick %u", m_counter / 10);
            }
            if(m_counter == ACTORS_NUM) {
                self().stop();
            }
        }
    };


    struct run_actor {};

    class TempActor
        : public yato::actors::actor
    {
        yato::actors::actor_ref m_echo;
    public:
        TempActor(const yato::actors::actor_ref & dst)
            : m_echo(dst)
        { }

        void receive(yato::any & message) override
        {
            yato::any_match(
                [this](const run_actor &) {
                    m_echo.tell("ping", self());
                    self().tell(yato::actors::poison_pill);
                },
                [this](yato::match_default_t){
                }
            )(message);
        }

    };

}

TEST(Yato_Actors, highload_spawn)
{
    auto conf = yato::actors::config::defaults();
    conf.log_filter = yato::actors::log_level::info;

    yato::actors::actor_system system("default", conf);

    auto echoActor = system.create_actor<EchoActor>("Echo");

    std::vector<yato::actors::actor_ref> actors;
    for(size_t i = 0; i < ACTORS_NUM; ++i) {
        actors.push_back(system.create_actor<TempActor>("temp_" + std::to_string(i), echoActor));
    }

    for (size_t i = 0; i < ACTORS_NUM; ++i) {
        actors[i].tell(run_actor{});
    }
}


