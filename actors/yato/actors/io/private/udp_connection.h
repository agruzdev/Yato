/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_UDP_CONNECTION_H_
#define _YATO_ACTORS_IO_UDP_CONNECTION_H_

#include <boost/asio.hpp>
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
        boost::asio::ip::udp::socket m_socket;

    public:
        udp_connection(const actor_ref & server, boost::asio::io_service & io, const inet_address & address)
            : m_server(server), m_socket(io, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), address.port))
        { }

        boost::asio::ip::udp::socket & socket() {
            return m_socket;
        }

        const boost::asio::ip::udp::socket & socket() const {
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

