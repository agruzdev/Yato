#include "gtest/gtest.h"

#include <thread>
#include <iostream>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>
#include <yato/any_match.h>

namespace
{
    class TestActor
        : public yato::actors::actor<>
    {
       
        void receive(const yato::any& message) override
        {
            yato::any_match(
                [this](int num) {
                    sender().tell(num + 1);
                }
            )(message);
        }

    };
}

TEST(Yato_Actors, ask)
{

    yato::actors::actor_system system("default");

    auto actor = system.create_actor<TestActor>("increment");

    auto res1 = actor.ask(41, std::chrono::seconds(1)).get();
    auto res2 = actor.ask(42, std::chrono::seconds(1)).get();
    auto res3 = actor.ask(43, std::chrono::seconds(1)).get();

    ASSERT_EQ(42, res1.get_as<int>(0));
    ASSERT_EQ(43, res2.get_as<int>(0));
    ASSERT_EQ(44, res3.get_as<int>(0));

    actor.tell(yato::actors::poison_pill);
}

