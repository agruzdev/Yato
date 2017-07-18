/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_TCP_REMOTE_H_
#define _YATO_ACTORS_IO_TCP_REMOTE_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "../../actor.h"

namespace yato
{
namespace actors
{
namespace io
{

    struct tcp_connection
    {
        actor_ref server;
        boost::asio::ip::tcp::socket socket;

        explicit
        tcp_connection(const actor_ref & server, boost::asio::io_service & io)
            : server(server), socket(io)
        { }
    };

    class tcp_remote
        : public actor<>
    {
        std::shared_ptr<tcp_connection> m_connection;
        std::unique_ptr<actor_ref> m_handler;

        // http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/overview/core/reactor.html
        void start_receive_()
        {
            m_connection->socket.async_read_some(boost::asio::null_buffers(), 
                boost::bind(&tcp_remote::handle_receive_, this, boost::asio::placeholders::error));
        }

        void handle_receive_(const boost::system::error_code & error)
        {
            if (!error) {
                std::vector<char> buf(m_connection->socket.available());
                m_connection->socket.read_some(boost::asio::buffer(buf));

                log().debug("%s", std::string(&buf[0], buf.size()).c_str());
                if(m_handler) {
                    m_handler->tell(tcp::received(std::move(buf)), self());
                } else {
                    log().warning("No handler is assigned. A packet was dropped.");
                }
            }
            else {
                log().error("Receive error! %s", error.message().c_str());
            }
            start_receive_();
        }

        void pre_start() override
        {
            auto remote = m_connection->socket.remote_endpoint();
            auto remote_address = inet_address(remote.address().to_string(), remote.port());
            m_connection->server.tell(tcp::connected(remote_address), self());
            m_connection->socket.non_blocking(true);
            start_receive_();
        }

        void receive(const yato::any & message) override
        {
            any_match(
                [this](const tcp::assign & assign) {
                    m_handler = std::make_unique<actor_ref>(assign.handler);
                }
            )(message);
        }

    public:
        explicit
        tcp_remote(const std::shared_ptr<tcp_connection> & connection)
            : m_connection(connection)
        { }
    };

} // namespace io

} // namespace actors

} // namespace yato

#endif // _YATO_ACTORS_IO_TCP_REMOTE_H_

