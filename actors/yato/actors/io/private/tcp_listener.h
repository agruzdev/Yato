/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_TCP_LISTENER_H_
#define _YATO_ACTORS_IO_TCP_LISTENER_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "../../actor.h"
#include "tcp_remote.h"

namespace yato
{
namespace actors
{
namespace io
{

    class tcp_listener
        : public actor<>
    {
        std::shared_ptr<boost::asio::io_service> m_io;
        boost::asio::ip::tcp::acceptor m_acceptor;
        boost::asio::ip::tcp::endpoint m_endpoint;
        actor_ref m_server;

        void pre_start() override
        {
            log().info("Listening %s", m_endpoint.address().to_string().c_str());
            start_accept_();
        }

        void start_accept_()
        {
            auto connection = std::make_shared<tcp_connection>(m_server, *m_io);
            m_acceptor.async_accept(connection->socket, boost::bind(&tcp_listener::handle_accept_, this, connection, boost::asio::placeholders::error));
        }

        void handle_accept_(const std::shared_ptr<tcp_connection> & connection, const boost::system::error_code & error)
        {
            if (!error) {
                auto remote = connection->socket.remote_endpoint();
                log().debug("New connection from %s", remote.address().to_string().c_str());

                auto remote_handler = actor_system_ex::create_actor<tcp_remote>(system(), actor_scope::remote, "wow!", connection);
            } else {
                log().error("tcp_listener[handle_accept_]: Error! %s", error.message().c_str());
            }
            start_accept_();
        }

        void receive(const yato::any & message) override
        {
        }

    public:
        tcp_listener(const actor_ref & handler, const std::shared_ptr<boost::asio::io_service>& io_service, const boost::asio::ip::tcp::endpoint & endpoint)
            : m_io(io_service), m_acceptor(*io_service, endpoint), m_endpoint(endpoint), m_server(handler)
        { }
    };

} // namespace io

} // namespace actors

} // namespace yato

#endif // _YATO_ACTORS_IO_TCP_LISTENER_H_

