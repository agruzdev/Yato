/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_UDP_REMOTE_H_
#define _YATO_ACTORS_IO_UDP_REMOTE_H_

#include <yato/any_match.h>

#include "../../actor.h"
#include "../udp.h"
#include "udp_receiver.h"

namespace yato
{
namespace actors
{
namespace io
{
    

    class udp_remote
        : public actor
    {
        std::shared_ptr<udp_connection> m_connection;
        boost::shared_ptr<udp_receiver> m_receiver;
        //------------------------------------------------------

        void pre_start() override
        {
            //const auto endpoint = m_connection->socket.remote_endpoint();
            //const auto address = inet_address(endpoint.address().to_string(), endpoint.port());

            //m_connection->server.tell(udp::bound(address), self());

            m_receiver = udp_receiver::create(m_connection, self(), m_connection->server());
        }

        void receive(yato::any && message) override
        {
            yato::any_match(
                [this](const boost::system::error_code & error) {
                    log().error("Receive error! %s", error.message().c_str());
                },
                [this](const udp::peer_closed & closed) {
                    log().info("Disconnected.");
                    if(m_receiver != nullptr) {
                        m_receiver->handler().tell(closed, self());
                    }
                    self().tell(poison_pill);
                },
                [this](const udp::send & send) {
                    if(m_receiver != nullptr) {
                        m_receiver->write(send.data, send.target);
                    }
                },
                [this](const terminated &) {
                    self().tell(udp::peer_closed());
                },
                [this](yato::match_default_t) {
                    log().error("Unknown message!");
                }
            )(message);
        }

    public:
        explicit
        udp_remote(const std::shared_ptr<udp_connection> & connection)
            : m_connection(connection)
        { }

        explicit
        udp_remote(std::shared_ptr<udp_connection> && connection)
            : m_connection(std::move(connection))
        { }
    };

} // namespace io

} // namespace actors

} // namespace yato

#endif // _YATO_ACTORS_IO_UDP_RECEIVER_H_

