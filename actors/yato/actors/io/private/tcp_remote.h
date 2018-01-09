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
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

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
        class receiver
            : public boost::enable_shared_from_this<receiver>
        {
            std::shared_ptr<tcp_connection> m_connection;
            actor_ref m_remote;
            actor_ref m_handler;

            static
            void handle_receive_(const boost::weak_ptr<receiver> & weak_self, const boost::system::error_code & error)
            {
                auto self = weak_self.lock();
                if (self != nullptr) {
                    if ((boost::asio::error::eof == error) || (boost::asio::error::connection_reset == error)) {
                        // Disconnected
                        self->m_remote.tell(tcp::peer_closed());
                        return;
                    }
                    if (!error) {
                        std::vector<char> buf(self->m_connection->socket.available());

                        boost::system::error_code read_err;
                        auto len = self->m_connection->socket.read_some(boost::asio::buffer(buf), read_err);
                        // ToDo (a.gruzdev): Returns 0 but error is not set. Why?
                        if ((boost::asio::error::eof == read_err) || (boost::asio::error::connection_reset == read_err) || (0 == len)) {
                            // Disconnected
                            self->m_remote.tell(tcp::peer_closed());
                            return;
                        }

                        self->m_handler.tell(tcp::received(std::move(buf)), self->m_remote);
                    }
                    else {
                        self->m_remote.tell(error);
                    }
                    self->start_receive_();
                }
            }

            // http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/overview/core/reactor.html
            void start_receive_()
            {
                m_connection->socket.async_read_some(boost::asio::null_buffers(), 
                    boost::bind(&handle_receive_, weak_from_this(), boost::asio::placeholders::error));
            }

            receiver(const std::shared_ptr<tcp_connection> & connection, const actor_ref & remote, const actor_ref & handler)
                : m_connection(connection), m_remote(remote), m_handler(handler) 
            {
                m_connection->socket.non_blocking(true);
            }

        public:
            ~receiver() = default;

            /**
             * Get handler ref
             */
            const actor_ref & handler() const 
            {
                return m_handler;
            }

            /**
             * Write data to socket
             */
            bool write(const std::vector<char> & data) {
                boost::system::error_code err;
                const size_t len = m_connection->socket.send(boost::asio::buffer(data), 0, err);
                return !err && (len == data.size());
            }

            /**
             * Has to be stored in boost::shared_ptr
             */
            static
            boost::shared_ptr<receiver> create(const std::shared_ptr<tcp_connection> & connection, const actor_ref & remote, const actor_ref & handler)
            {
                boost::shared_ptr<receiver> p;
                p.reset(new receiver(connection, remote, handler));
                p->start_receive_();
                return p;
            }
        };
        //------------------------------------------------------

        std::shared_ptr<tcp_connection> m_connection;
        boost::shared_ptr<receiver> m_receiver;
        //------------------------------------------------------

        void pre_start() override
        {
            auto remote = m_connection->socket.remote_endpoint();
            auto remote_address = inet_address(remote.address().to_string(), remote.port());

            m_connection->server.tell(tcp::connected(remote_address), self());
        }

        void receive(yato::any & message) override
        {
            any_match(
                [this](const tcp::write & msg) {
                    if(m_receiver != nullptr) {
                        if(!m_receiver->write(msg.data)) {
                            log().error("Unknown error on tcp::write");
                        }
                    } else {
                        log().warning("Handler is not assigned yet. Message was dropped!");
                    }
                },
                [this](const tcp::assign & assign) {
                    if (m_receiver == nullptr) {
                        watch(assign.handler);
                        m_receiver = receiver::create(m_connection, self(), assign.handler);
                    }
                },
                [this](const boost::system::error_code & error) {
                    log().error("Receive error! %s", error.message().c_str());
                },
                [this](const tcp::peer_closed & closed) {
                    log().info("Disconnected.");
                    if(m_receiver != nullptr) {
                        m_receiver->handler().tell(closed, self());
                    }
                    self().tell(poison_pill);
                },
                [this](const terminated &) {
                    self().tell(tcp::peer_closed());
                },
                [this](match_default_t) {
                    log().error("Unknown message!");
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

