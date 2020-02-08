/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_TCP_H_
#define _YATO_ACTORS_IO_TCP_H_

#include <memory>
#include <vector>

#include "../actor.h"
#include "facade.h"
#include "inet_address.h"

namespace yato
{
namespace actors
{
    class actor_system;

namespace io
{
    class io_context;


    namespace tcp
    {
        /**
         * Returns tcp manager for an actor system.
         */
        actor_ref get_for(const actor_system & sys);

        /**
         * Create a TCP server and listen for inbound connections.
         */
        struct bind
        {
            actor_ref handler;
            inet_address address;

            bind(const actor_ref & handler, const inet_address & address)
                : handler(handler), address(address)
            { }
        };

        /**
         * Create a TCP client and connect to an endpoint.
         */
        struct connect
        {
            actor_ref handler;
            inet_address address;

            connect(const actor_ref & handler, const inet_address & address)
                : handler(handler), address(address)
            { }
        };

        /**
         * Assign handler for a new connection.
         * (Similar to Akka's Register class)
         */
        struct assign
        {
            actor_ref handler;

            assign(const actor_ref & handler)
                : handler(handler)
            { }
        };

        /**
         * Indicates successful binding of server to an address.
         */
        struct bound
        {
            inet_address local;

            explicit
            bound(const inet_address & local)
                : local(local)
            { }
        };

        /**
         * Informs about new connection.
         */
        struct connected
        {
            inet_address remote;

            explicit
            connected(const inet_address & remote)
                : remote(remote)
            { }
        };

        /**
         * Contains received data.
         */
        struct received
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

        /**
         * Outgoing data wrapper
         */
        struct write
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

        /**
         * Reports error in command execution.
         */
        struct command_fail
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

        /**
         * Connection was closed by remote
         */
        struct peer_closed
        { };
    }


        // ToDo (a.gruzdev): Move declaration into private sources
    class tcp_manager
        : public actor
    {
    private:
        std::shared_ptr<io_context> m_context;
        //------------------------------------------------

        void pre_start() override;

        void receive(yato::any && message) override;

        void post_stop() override;

        static const char* actor_name();
        //------------------------------------------------

    public:
        // ToDo (a.gruzdev): Make an interface for protecting actors constructors
        tcp_manager(const std::shared_ptr<io_context> & io_context);

        ~tcp_manager();

        friend struct facade;
        friend actor_ref tcp::get_for(const actor_system & sys);
    };


}// namespace io

}// namespace actors

}// namespace yato

#endif // _YATO_ACTORS_IO_TCP_H_

