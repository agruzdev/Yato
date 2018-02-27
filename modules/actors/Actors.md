## Yato::Actors


Hello world:

```c++

    class MyActor
        : public yato::actors::actor
    {
        void receive(yato::any && message) {
            log().info("Hello from Actor!");
        }
    };

    ...

    yato::actors::actor_system system("default");

    auto actor = system.create_actor<MyActor>("hello");
    actor.tell(std::string("Hello, Actor!"));

```

## Yato::Actors::IO

Actors IO module implements basic interface for creating TCP connection and sending/receiving UDP datagrams similar to Akka.IO

#### Simple TCP client:

```c++

    class TcpClient
        : public yato::actors::actor
    {
        void receive(yato::any && message) override {
            using namespace yato::actors::io;
            yato::any_match(
                [this](const tcp::connected & connected) {
                    log().info("Connected to %s:%d", connected.remote.host.c_str(), connected.remote.port);
                    sender().tell(tcp::assign(self()));
                },
                [this](const tcp::received & received) {
                    const std::string msg = std::string(received.data.cbegin(), received.data.cend());
                    log().info("Received: %s", msg.c_str());
                },
                [this](const tcp::peer_closed &) {
                    log().info("Disconnected");
                    self().stop();
                },
                [this](const tcp::command_fail & fail) {
                    log().error("Fail. Reason: %s", fail.reason.c_str());
                    self().stop();
                }
            )(message);
        }
    };

    ...

    auto manager = io::tcp::get_for(system);
    auto client  = system.create_actor<TcpClient>("client");
    manager.tell(io::tcp::connect(client, io::inet_address("localhost", 9001)));

```

#### Simple TCP server:

```c++

    class TcpEchoServer
        : public yato::actors::actor
    {
        void receive(yato::any && message) override {
            using namespace yato::actors::io;
            log().info(message.type().name());
            yato::any_match(
                [this](const tcp::bound & bound) {
                    log().info("Bound. Address %s:%d", bound.local.host.c_str(), bound.local.port);
                },
                [this](const tcp::connected & connected) {
                    log().info("New connection. Address %s:%d", connected.remote.host.c_str(), connected.remote.port);
                    sender().tell(tcp::assign(self()));
                },
                [this](const tcp::received & received) {
                    const std::string msg = std::string(received.data.cbegin(), received.data.cend());
                    log().info("Received: %s", msg.c_str());
                    sender().tell(tcp::write(received.data));
                },
                [this](const tcp::peer_closed &) {
                    log().info("Disconnected");
                    self().stop();
                }
                [this](const tcp::command_fail & fail) {
                    log().error("Fail. Reason: %s", fail.reason.c_str());
                    self().stop();
                }
            )(message);
        }
    };

    ...

    auto manager = io::tcp::get_for(system);
    auto server  = system.create_actor<TcpEchoServer>("TcpServer");
    manager.tell(io::tcp::bind(server, io::inet_address("localhost", 9001)));

```

