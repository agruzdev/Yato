#include "gtest/gtest.h"

#include <thread>

#include <yato/actors/actor_system.h>
#include <yato/actors/logger.h>
#include <yato/any_match.h>

namespace
{
    class TestActor
        : public yato::actors::actor<>
    {
        void receive(yato::any &) override
        { }
    };

    class ObserverActor
        : public yato::actors::actor<>
    {
        void receive(yato::any & message) override
        {
            yato::any_match(
                [this](const yato::actors::terminated & t) {
                    log().info("Actor %s is terminated.", t.ref.get_path().c_str());
                    self().tell(yato::actors::poison_pill);
                }
            )(message);
        }
    };
}

TEST(Yato_Actors, watch)
{
    yato::actors::actor_system system("default");
    system.logger()->set_filter(yato::actors::log_level::verbose);

    auto actor = system.create_actor<TestActor>("Test");

    auto watcher = system.create_actor<ObserverActor>("Watcher");
    system.watch(actor, watcher);

    actor.tell(yato::actors::poison_pill);
}

TEST(Yato_Actors, unwatch)
{
    yato::actors::actor_system system("default");
    system.logger()->set_filter(yato::actors::log_level::verbose);

    auto actor = system.create_actor<TestActor>("Test");

    auto watcher = system.create_actor<ObserverActor>("Watcher");
    system.watch(actor, watcher);
    system.unwatch(actor, watcher);

    actor.tell(yato::actors::poison_pill);
    watcher.tell(yato::actors::poison_pill);
}


