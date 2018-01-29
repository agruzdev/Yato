/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_UDP_H_
#define _YATO_ACTORS_IO_UDP_H_

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
    // UDP commands
    namespace udp
    {
        /**
         * Returns udp manager for an actor system.
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
         * Indicates successful binding of actor to an address.
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
         * Contains received data.
         */
        struct received
        {
            std::vector<char> data;
            inet_address sender;

            explicit
            received(const std::vector<char> & data, const inet_address & sender)
                : data(data), sender(sender)
            { }

            explicit
            received(std::vector<char> && data, const inet_address & sender)
                : data(std::move(data)), sender(sender)
            { }

            explicit
            received(std::vector<char> && data, inet_address && sender)
                : data(std::move(data)), sender(std::move(sender))
            { }
        };

        /**
         * Connection was closed by system
         */
        struct peer_closed
        { };

        /**
         * Send udp packet to specified address
         */
        struct send
        {
            std::vector<char> data;
            inet_address target;

            send(const std::vector<char> & data, const inet_address & target)
                : data(data), target(target)
            { }

            send(std::vector<char> && data, const inet_address & target)
                : data(std::move(data)), target(target)
            { }

            send(std::vector<char> && data, inet_address && target)
                : data(std::move(data)), target(std::move(target))
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

    } // namespace udp

    class io_context;

    class udp_manager
        : public actor
    {
        std::shared_ptr<io_context> m_context;
        //--------------------------------------------

        void pre_start() override;

        void receive(yato::any & message) override;

        void post_stop() override;

        static const char* actor_name();
        //------------------------------------------------

    public:
        // ToDo (a.gruzdev): Make an interface for protecting actors constructors
        udp_manager(const std::shared_ptr<io_context>& ctx);

        ~udp_manager();

        friend struct facade;
        friend actor_ref udp::get_for(const actor_system & sys);
    };

    

} // namespace yato

} // namespace actors

} // namespace io

#endif // _YATO_ACTORS_IO_UDP_H_
