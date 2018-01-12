#include "gtest/gtest.h"

#include <thread>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>
#include <yato/any_match.h>

namespace
{
    class EchoActor
        : public yato::actors::actor
    {
        void pre_start() override 
        {
            log().info("pre_start() is called!");
        }

        void receive(yato::any& message) override
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
    auto conf = yato::actors::config::defaults();
    conf.log_filter = yato::actors::log_level::debug;

    yato::actors::actor_system system("default", conf);

    auto actor = system.create_actor<EchoActor>("echo1");
    actor.tell(std::string("Hello, Actor!"));

    system.shutdown();
}



namespace
{
    const int PING_LIMIT = 100000;
    const int PING_TICK  = PING_LIMIT / 10;

    class PingActor
        : public yato::actors::actor
    {
        void receive(yato::any & message) override
        {
            yato::any_match(
                [this](int count) {
                    sender().tell(count + 1, self());
                    if(count >= PING_LIMIT) {
                        self().stop();
                    }
                    else {
                        if (count % PING_TICK == 0) {
                            log().info("Ping " + std::to_string(count));
                        }
                    }
                }
            )(message);
        }
    };


    class PongActor
        : public yato::actors::actor
    {
        void receive(yato::any & message) override
        {
            yato::any_match(
                [this](int count) {
                    sender().tell(count + 1, self());
                    if (count >= PING_LIMIT) {
                        self().stop();
                    }
                    else {
                        if (count % PING_TICK == 1) {
                            log().info("Pong " + std::to_string(count));
                        }
                    }
                }
            )(message);
        }
    };
}

TEST(Yato_Actors, ping_pong) 
{
    auto conf = yato::actors::config::defaults();
    conf.log_filter = yato::actors::log_level::verbose;

    yato::actors::actor_system system("default", conf);

    auto actor1 = system.create_actor<PongActor>("PongActor");
    auto actor2 = system.create_actor<PingActor>("PingActor");

    system.send_message(actor1, 1, actor2);
}






namespace
{

    class actD
        : public yato::actors::actor
    {
        void pre_start() override {
            log().info("start %s", self().name().c_str());
            if(self().name() == "D") {
                create_child<actD>("D0");
                create_child<actD>("D1");
            }
        }

        void receive(yato::any &) override
        { }

        void post_stop() override {
            log().info("stop %s", self().name().c_str());
        }
    };

    class actB
        : public yato::actors::actor
    {
        void pre_start() override {
            log().info("start %s", self().name().c_str());
        }

        void receive(yato::any &) override
        { }

        void post_stop() override {
            log().info("stop %s", self().name().c_str());
        }
    };

    class actC
        : public yato::actors::actor
    {
        void pre_start() override {
            log().info("start %s", self().name().c_str());
            create_child<actD>("D");
        }

        void receive(yato::any &) override
        { }

        void post_stop() override {
            log().info("stop %s", self().name().c_str());
        }
    };

    class actA
        : public yato::actors::actor
    {
        void pre_start() override {
            log().info("start %s", self().name().c_str());
            create_child<actB>("B");
            create_child<actC>("C");
        }

        void receive(yato::any &) override
        { }

        void post_stop() override {
            log().info("stop %s", self().name().c_str());
        }
    };
}


TEST(Yato_Actors, search) 
{
    auto conf = yato::actors::config::defaults();
    conf.log_filter = yato::actors::log_level::verbose;

    yato::actors::actor_system system("default", conf);

    auto root = system.create_actor<actA>("A");

    auto a = system.find(yato::actors::actor_path("yato://default/user/A"), std::chrono::seconds(5)).get();
    EXPECT_FALSE(a.empty());

    auto b = system.find(yato::actors::actor_path("yato://default/user/A/B"), std::chrono::seconds(5)).get();
    EXPECT_FALSE(b.empty());

    auto c = system.find(yato::actors::actor_path("yato://default/user/C"), std::chrono::seconds(5)).get();
    EXPECT_TRUE(c.empty());

    root.tell(yato::actors::poison_pill);
}
