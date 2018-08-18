/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_TCP_CONNECTION_H_
#define _YATO_ACTORS_IO_TCP_CONNECTION_H_

#include <asio.hpp>
#include "../../actor_ref.h"

namespace yato
{
namespace actors
{
namespace io
{

    class tcp_connection
    {
    private:
        actor_ref m_server;
        asio::ip::tcp::socket m_socket;

    public:
        tcp_connection(const actor_ref & server, asio::io_service & io)
            : m_server(server), m_socket(io)
        { }

        asio::ip::tcp::socket & socket() {
            return m_socket;
        }

        const asio::ip::tcp::socket & socket() const {
            return m_socket;
        }

        const actor_ref & server() const {
            return m_server;
        }
    };

} // namespace io

} // namespace actors

} // namespace yato

#endif // _YATO_ACTORS_IO_TCP_CONNECTION_H_

