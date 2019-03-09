/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_TCP_REMOTE_H_
#define _YATO_ACTORS_IO_TCP_REMOTE_H_

#include <memory>

#include <asio.hpp>

#include "../../actor.h"
#include "../tcp.h"
#include "tcp_receiver.h"

namespace yato
{
namespace actors
{
namespace io
{

    class tcp_remote
        : public actor
    {
        std::shared_ptr<tcp_connection> m_connection;
        std::shared_ptr<tcp_receiver> m_receiver;
        //------------------------------------------------------

        void pre_start() override
        {
            auto remote = m_connection->socket().remote_endpoint();
            const auto remote_address = inet_address(remote.address().to_string(), remote.port());

            m_connection->server().tell(tcp::connected(remote_address), self());
        }

        void receive(yato::any && message) override
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
                        m_receiver = tcp_receiver::create(m_connection, self(), assign.handler);
                    }
                },
                [this](const asio::error_code & error) {
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

