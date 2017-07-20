/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_TCP_H_
#define _YATO_ACTORS_IO_TCP_H_

namespace yato
{
namespace actors
{
    class actor_system;

namespace io
{
    struct tcp_context;

    // ToDo (a.gruzdev): Move declaration into private sources
    class tcp_manager
        : public actor<>
    {
    private:
        std::unique_ptr<tcp_context> m_context;
        //------------------------------------------------

        tcp_manager();

        void pre_start() override;

        void receive(const yato::any & message);

        void post_stop() override;

        static const char* actor_name();
        //------------------------------------------------

    public:
        ~tcp_manager();

        // ToDo (a.gruzdev): Consider interface in actor_system for using actor factory
        friend class actor_system; // can be created only by actor system
        friend class tcp;
    };

    struct inet_address
    {
        std::string host;
        uint16_t port;

        inet_address(const char* host, uint16_t port)
            : host(host), port(port)
        { }

        inet_address(const std::string & host, uint16_t port)
            : host(host), port(port)
        { }
    };

    class tcp
    {
    public:
        /**
         * Create a TCP server and listen for inbound connections.
         */
        struct bind;

        /**
         * Assign handler for a new connection.
         * (Similar to Akka's Register class)
         */
        struct assign;

        /**
         * Indicates successful binding of server to an address.
         */
        struct bound;

        /**
         * Informs about new connection.
         */
        struct connected;

        /**
         * Contains received data.
         */
        struct received;

        /**
         * Outgoing data wrapper
         */
        struct write;

        /**
         * Reports error in command execution.
         */
        struct command_fail;

        /**
         * Connection was closed by remote
         */
        struct peer_closed;

        /**
         * Returns tcp manager for an actor system.
         */
        static actor_ref get_for(const actor_system & sys);
    };

    struct tcp::bind
    {
        actor_ref handler;
        inet_address address;

        bind(const actor_ref & handler, const inet_address & address)
            : handler(handler), address(address)
        { }
    };

    struct tcp::assign
    {
        actor_ref handler;

        assign(const actor_ref & handler)
            : handler(handler)
        { }
    };

    struct tcp::bound
    {
        actor_ref listener;
        inet_address address;

        explicit
        bound(const actor_ref & listener, const inet_address & address)
            : listener(listener), address(address)
        { }
    };

    struct tcp::connected
    {
        inet_address remote;

        explicit
        connected(const inet_address & remote)
            : remote(remote)
        { }
    };

    struct tcp::received
    {
        std::vector<char> data;

        explicit
        received(const std::vector<char> & data)
            : data(data)
        { }

        explicit
        received(std::vector<char> && data)
            : data(std::move(data))
        { }
    };

    struct tcp::write
    {
        std::vector<char> data;

        explicit
        write(const std::vector<char> & data)
            : data(data)
        { }

        explicit
        write(std::vector<char> && data)
            : data(std::move(data))
        { }
    };

    struct tcp::peer_closed
    { };

    struct tcp::command_fail
    {
        std::string reason;

        explicit
        command_fail(const char* reason)
            : reason(reason)
        { }

        explicit
        command_fail(const std::string & reason)
            : reason(reason)
        { }
    };


}// namespace io

}// namespace actors

}// namespace yato

#endif // _YATO_ACTORS_IO_TCP_H_

