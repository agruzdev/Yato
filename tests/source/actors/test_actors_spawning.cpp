#include "gtest/gtest.h"

#include <atomic>
#include <thread>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>

namespace
{
    const size_t ECHO_NUM   = 10;
    const size_t ACTORS_NUM = 100000;

    std::atomic_bool gStart{ false };

    class EchoActor
        : public yato::actors::actor<>
    {
        uint32_t m_counter = 0;
    public:
        void receive(const yato::any & message) override
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

    class TempActor
        : public yato::actors::actor<>
    {
        yato::actors::actor_ref m_echo;
    public:
        TempActor(const yato::actors::actor_ref & dst)
            : m_echo(dst)
        { }

        void pre_start() override
        {
            while (!gStart.load(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
            m_echo.tell("ping", self());
        }

        void receive(const yato::any & message) override
        {
            YATO_MAYBE_UNUSED(message);
        }

    };

}

TEST(Yato_Actors, highload_spawn)
{

    yato::actors::actor_system system("default");

    auto echoActor = system.create_actor<EchoActor>("Echo");

    system.logger()->set_filter(yato::actors::log_level::info);
    std::vector<yato::actors::actor_ref> actors;
    for(size_t i = 0; i < ACTORS_NUM; ++i) {
        actors.push_back(system.create_actor<TempActor>("temp_" + std::to_string(i), echoActor));
    }
    gStart = true;
    for (size_t i = 0; i < ACTORS_NUM; ++i) {
        actors[i].tell(yato::actors::poison_pill);
    }
}


