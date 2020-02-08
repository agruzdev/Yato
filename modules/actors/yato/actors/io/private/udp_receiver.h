/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_UDP_RECEIVER_H_
#define _YATO_ACTORS_IO_UDP_RECEIVER_H_

#include <functional>
#include <memory>

#include <asio.hpp>

#include "../../actor.h"
#include "../udp.h"
#include "udp_connection.h"

namespace yato
{
namespace actors
{
namespace io
{

    class udp_receiver
        : public std::enable_shared_from_this<udp_receiver>
    {
    private:
        static YATO_CONSTEXPR_VAR size_t MAX_BUFFER_SIZE = 65536;
        using buffer_type = std::array<char, MAX_BUFFER_SIZE>;

        std::shared_ptr<udp_connection> m_connection;
        asio::ip::udp::endpoint m_remote_endpoint;

        std::shared_ptr<buffer_type> m_message_buffer;

        actor_ref m_remote;
        actor_ref m_handler;

        static
        void handle_receive_(const std::weak_ptr<udp_receiver> & weak_self, const std::shared_ptr<buffer_type> & buffer, const asio::error_code & error, std::size_t bytes_transferred)
        {
            auto self = weak_self.lock();
            if (self != nullptr) {
                if ((asio::error::eof == error) || (asio::error::connection_reset == error)) {
                    // Disconnected
                    self->m_remote.tell(udp::peer_closed());
                    return;
                }
                if (!error) {
                    if(buffer != nullptr) {

                        std::vector<char> buf(bytes_transferred);
                        if(bytes_transferred != 0) {
                            std::memcpy(&buf[0], &(*buffer)[0], bytes_transferred);
                        }

                        inet_address sender(self->m_remote_endpoint.address().to_string(), self->m_remote_endpoint.port());
                        self->m_handler.tell(udp::received(std::move(buf), std::move(sender)), self->m_remote);
                    }
                }
                else {
                    self->m_remote.tell(error);
                }
                self->start_receive_();
            }
        }

        std::weak_ptr<udp_receiver> weak_from_this_impl_()
        {
#if defined(YATO_CXX17) || (defined(_MSC_VER) && (_MSC_VER >= 1910))
            return weak_from_this();
#else
            return std::weak_ptr<udp_receiver>(shared_from_this());
#endif
        }

        // http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/overview/core/reactor.html
        void start_receive_()
        {
            YATO_REQUIRES(m_message_buffer != nullptr);
            // Pass buffer ptr through bind to make sure it is not destroyed
            m_connection->socket().async_receive_from(asio::buffer(*m_message_buffer), m_remote_endpoint,
                std::bind(&handle_receive_, weak_from_this_impl_(), m_message_buffer, std::placeholders::_1, std::placeholders::_2));
        }

        udp_receiver(const std::shared_ptr<udp_connection> & connection, const actor_ref & remote, const actor_ref & handler)
            : m_connection(connection), m_remote(remote), m_handler(handler) 
        {
            m_message_buffer = std::make_shared<buffer_type>();
            m_connection->socket().non_blocking(true);
        }

    public:
        ~udp_receiver() = default;

        udp_receiver(const udp_receiver&) = delete;
        udp_receiver(udp_receiver&&) = delete;

        udp_receiver& operator=(const udp_receiver&) = delete;
        udp_receiver& operator=(udp_receiver&&) = delete;

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
        bool write(const std::vector<char> & data, const inet_address & address) {
            asio::error_code err;
            const asio::ip::udp::endpoint destination(asio::ip::address::from_string(address.host), address.port);
            const size_t len = m_connection->socket().send_to(asio::buffer(data), destination, 0, err);
            return !err && (len == data.size());
        }

        static
        std::shared_ptr<udp_receiver> create(const std::shared_ptr<udp_connection> & connection, const actor_ref & remote, const actor_ref & handler)
        {
            std::shared_ptr<udp_receiver> p;
            p.reset(new udp_receiver(connection, remote, handler));
            p->start_receive_();
            return p;
        }
    };

} // namespace io

} // namespace actors

} // namespace yato

#endif // _YATO_ACTORS_IO_RECEIVER_H_

