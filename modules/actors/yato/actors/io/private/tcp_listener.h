/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_TCP_LISTENER_H_
#define _YATO_ACTORS_IO_TCP_LISTENER_H_

#include <functional>
#include <memory>

#include <asio.hpp>

#include "../../actor.h"
#include "tcp_remote.h"
#include "tcp_connection.h"

namespace yato
{
namespace actors
{
namespace io
{
    class tcp_listener
        : public actor
    {
        struct accept
        {
            std::shared_ptr<tcp_connection> connection;
            asio::error_code error;

            accept(const std::shared_ptr<tcp_connection> & connection, const asio::error_code & error)
                : connection(connection), error(error)
            { }
        };
        //-----------------------------------------------------------

        /**
         * Listens to socket and sends `accept` message to listener actor
         */
        class acceptor
            : public std::enable_shared_from_this<acceptor>
        {
            std::shared_ptr<io_context> m_context;
            asio::ip::tcp::acceptor m_acceptor;
            asio::ip::tcp::endpoint m_endpoint;
            actor_ref m_server;
            actor_ref m_listener;

            static
            void handle_accept_(const std::weak_ptr<acceptor> weak_self, const std::shared_ptr<tcp_connection> & connection, const asio::error_code & error)
            {
                auto self = weak_self.lock();
                if (self != nullptr) {
                    self->m_listener.tell(accept(connection, error));
                    self->start_accept_();
                }
            }

            std::weak_ptr<acceptor> weak_from_this_impl_()
            {
#if defined(YATO_CXX17) || (defined(_MSC_VER) && (_MSC_VER >= 1910))
                return weak_from_this();
#else
                return std::weak_ptr<acceptor>(shared_from_this());
#endif
            }

            void start_accept_()
            {
                auto connection = std::make_shared<tcp_connection>(m_server, m_context->service());
                m_acceptor.async_accept(connection->socket(),
                    std::bind(&handle_accept_, weak_from_this_impl_(), connection, std::placeholders::_1));
            }

            acceptor(const std::shared_ptr<io_context> & ctx, const asio::ip::tcp::endpoint & endpoint, const actor_ref & server, const actor_ref & listener)
                : m_context(ctx), m_acceptor(ctx->service(), endpoint), m_endpoint(endpoint), m_server(server), m_listener(listener) 
            { }

        public:
            ~acceptor() = default;

            /**
             * Has to be stored in shared_ptr object
             */
            static
            std::shared_ptr<acceptor> create(const std::shared_ptr<io_context> & ctx, const asio::ip::tcp::endpoint & endpoint, const actor_ref & server, const actor_ref & listener) 
            {
                std::shared_ptr<acceptor> p;
                p.reset(new acceptor(ctx, endpoint, server, listener));
                p->start_accept_();
                return p;
            }
        };
        //-----------------------------------------------------------

        std::shared_ptr<io_context> m_context;
        asio::ip::tcp::endpoint m_endpoint;
        actor_ref m_server;

        std::shared_ptr<acceptor> m_acceptor;
        //-----------------------------------------------------------

        void pre_start() override
        {
            log().info("Listening %s", m_endpoint.address().to_string().c_str());
            m_acceptor = acceptor::create(m_context, m_endpoint, m_server, self());
        }

        void receive(yato::any && message) override
        {
            yato::any_match(
                [this](const accept & acc) {
                    if (!acc.error) {
                        auto remote = acc.connection->socket().remote_endpoint();
                        log().debug("New connection from %s", remote.address().to_string().c_str());

                        const std::string remote_name = remote.address().to_string() + ":" + std::to_string(remote.port());
                        actor_system_ex::create_actor<tcp_remote>(system(), self(), remote_name, acc.connection);
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
        tcp_listener(const actor_ref & handler, const std::shared_ptr<io_context>& ctx, const asio::ip::tcp::endpoint & endpoint)
            : m_context(ctx), m_endpoint(endpoint), m_server(handler)
        { }
    };

} // namespace io

} // namespace actors

} // namespace yato

#endif // _YATO_ACTORS_IO_TCP_LISTENER_H_

