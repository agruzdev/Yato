/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_UDP_CONNECTION_H_
#define _YATO_ACTORS_IO_UDP_CONNECTION_H_

#include <asio.hpp>

#include "../../actor_ref.h"
#include "../inet_address.h"

namespace yato
{
namespace actors
{
namespace io
{

    class udp_connection
    {
    private:
        actor_ref m_server;
        asio::ip::udp::socket m_socket;

    public:
        udp_connection(const actor_ref & server, asio::io_service & io, const inet_address & address)
            : m_server(server), m_socket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), address.port))
        { }

        asio::ip::udp::socket & socket() {
            return m_socket;
        }

        const asio::ip::udp::socket & socket() const {
            return m_socket;
        }

        const actor_ref & server() const {
            return m_server;
        }
    };

} // namespace io

} // namespace actors

} // namespace yato

#endif // _YATO_ACTORS_IO_UDP_CONNECTION_H_

