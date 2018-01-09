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
#include <boost/enable_shared_from_this.hpp>

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
        struct accept
        {
            std::shared_ptr<tcp_connection> connection;
            boost::system::error_code error;

            accept(const std::shared_ptr<tcp_connection> & connection, const boost::system::error_code & error)
                : connection(connection), error(error)
            { }
        };
        //-----------------------------------------------------------

        /**
         * Listens to socket and sends `accept` message to listener actor
         */
        class acceptor
            : public boost::enable_shared_from_this<acceptor>
        {
            std::shared_ptr<boost::asio::io_service> m_io;
            boost::asio::ip::tcp::acceptor m_acceptor;
            boost::asio::ip::tcp::endpoint m_endpoint;
            actor_ref m_server;
            actor_ref m_listener;

            static
            void handle_accept_(const boost::weak_ptr<acceptor> weak_self, const std::shared_ptr<tcp_connection> & connection, const boost::system::error_code & error)
            {
                auto self = weak_self.lock();
                if (self != nullptr) {
                    self->m_listener.tell(accept(connection, error));
                    self->start_accept_();
                }
            }

            void start_accept_()
            {
                auto connection = std::make_shared<tcp_connection>(m_server, *m_io);
                m_acceptor.async_accept(connection->socket,
                    boost::bind(&handle_accept_, weak_from_this(), connection, boost::asio::placeholders::error));
            }

            acceptor(const std::shared_ptr<boost::asio::io_service> & io, const boost::asio::ip::tcp::endpoint & endpoint, const actor_ref & server, const actor_ref & listener)
                : m_io(io), m_acceptor(*io, endpoint), m_endpoint(endpoint), m_server(server), m_listener(listener) 
            { }

        public:
            ~acceptor() = default;

            /**
             * Has to be stored in shared_ptr object
             */
            static
            boost::shared_ptr<acceptor> create(const std::shared_ptr<boost::asio::io_service> & io, const boost::asio::ip::tcp::endpoint & endpoint, const actor_ref & server, const actor_ref & listener) 
            {
                boost::shared_ptr<acceptor> p;
                p.reset(new acceptor(io, endpoint, server, listener));
                p->start_accept_();
                return p;
            }
        };
        //-----------------------------------------------------------

        std::shared_ptr<boost::asio::io_service> m_io;
        boost::asio::ip::tcp::endpoint m_endpoint;
        actor_ref m_server;

        boost::shared_ptr<acceptor> m_acceptor;
        //-----------------------------------------------------------

        void pre_start() override
        {
            log().info("Listening %s", m_endpoint.address().to_string().c_str());
            m_acceptor = acceptor::create(m_io, m_endpoint, m_server, self());
        }

        void receive(yato::any & message) override
        {
            yato::any_match(
                [this](const accept & acc) {
                    if (!acc.error) {
                        auto remote = acc.connection->socket.remote_endpoint();
                        log().debug("New connection from %s", remote.address().to_string().c_str());

                        //const std::string remote_name = m_server.name() + "/" + remote.address().to_string() + ":" + std::to_string(remote.port());
                        //auto remote_handler = actor_system_ex::create_actor<tcp_remote>(system(), actor_scope::remote, remote_name, acc.connection);
                        const std::string remote_name = remote.address().to_string() + ":" + std::to_string(remote.port());
                        const auto remote_handler = actor_system_ex::create_actor<tcp_remote>(system(), self(), remote_name, acc.connection);
                        //system().watch(self(), remote_handler); // ToDo (a.gruzdev): Temporal solution. Proper family tree will solve it
                    } else {
                        log().error("Error! %s", acc.error.message().c_str());
                    }
                },
                [this](const terminated & t) {
                    // Server actor is terminated
                    if(m_server == t.ref) {
                        self().tell(poison_pill);
                    }
                }
            )(message);
        }

        void post_stop() override
        {
            log().info("Stop listening %s", m_endpoint.address().to_string().c_str());
            m_acceptor.reset();
        }

    public:
        tcp_listener(const actor_ref & handler, const std::shared_ptr<boost::asio::io_service>& io_service, const boost::asio::ip::tcp::endpoint & endpoint)
            : m_io(io_service), m_endpoint(endpoint), m_server(handler)
        { }
    };

} // namespace io

} // namespace actors

} // namespace yato

#endif // _YATO_ACTORS_IO_TCP_LISTENER_H_

