/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <thread>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>
#include <yato/actors/inbox.h>
#include <yato/any_match.h>

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

TEST(Yato_Actors, inbox)
{
    yato::actors::actor_system system("default");
    system.logger()->set_filter(yato::actors::log_level::verbose);

    const auto actor = system.create_actor<TestActor>("increment");

    auto inbox = yato::actors::inbox(system, "test");
    inbox.send(actor, 1);

    auto res = inbox.receive(std::chrono::seconds(2));
    int res_int = 0;
    EXPECT_NO_THROW(res_int = res.get<int>());
    EXPECT_EQ(2, res_int);

    inbox.watch(actor);
    actor.tell(yato::actors::poison_pill);

    auto t = inbox.receive(std::chrono::seconds(2));
    EXPECT_EQ(typeid(yato::actors::terminated), t.type());

    system.send_message(inbox.ref(), std::string("ping"));
    std::string res2;
    EXPECT_NO_THROW(res2 = inbox.receive(std::chrono::seconds(2)).get<std::string>());
    EXPECT_EQ(std::string("ping"), res2);

    auto empty_res = inbox.receive();
    EXPECT_TRUE(empty_res.empty());
}


