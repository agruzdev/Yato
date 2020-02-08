/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <thread>
#include <iostream>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>
#include <yato/any_match.h>

#include "test_actors_common.h"

namespace
{
    class TestActor
        : public yato::actors::actor
    {
       
        void receive(yato::any && message) override
        {
            yato::any_match(
                [this](int num) {
                    sender().tell(num + 1);
                },
                [](yato::match_default_t) {
                }
            )(message);
        }

    };
}

TEST(Yato_Actors, ask)
{
    yato::actors::actor_system system("default");
    system.logger()->set_filter(yato::actors::log_level::verbose);

    auto actor = system.create_actor<TestActor>("increment");

    auto res1 = actor.ask(41, std::chrono::seconds(1)).get();
    auto res2 = actor.ask(42, std::chrono::seconds(1)).get();
    auto res3 = actor.ask(43, std::chrono::seconds(1)).get();

    ASSERT_EQ(42, res1.get_as<int>(0));
    ASSERT_EQ(43, res2.get_as<int>(0));
    ASSERT_EQ(44, res3.get_as<int>(0));

    // shoould not wait sop long, will be killed by the system
    actor.ask(1.0f, std::chrono::seconds(1000));

    actor.tell(yato::actors::poison_pill);
}


