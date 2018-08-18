/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_TCP_RECEIVER_H_
#define _YATO_ACTORS_IO_TCP_RECEIVER_H_

#include <functional>
#include <memory>

#include <asio.hpp>

#include "../tcp.h"
#include "tcp_connection.h"

namespace yato
{
namespace actors
{
namespace io
{

    class tcp_receiver
        : public std::enable_shared_from_this<tcp_receiver>
    {
    private:
        std::shared_ptr<tcp_connection> m_connection;
        actor_ref m_remote;
        actor_ref m_handler;

        static
        void handle_receive_(const std::weak_ptr<tcp_receiver> & weak_self, const asio::error_code & error, std::size_t /*bytes_transferred*/)
        {
            auto self = weak_self.lock();
            if (self != nullptr) {
                if ((asio::error::eof == error) || (asio::error::connection_reset == error)) {
                    // Disconnected
                    self->m_remote.tell(tcp::peer_closed());
                    return;
                }
                if (!error) {
                    std::vector<char> buf(self->m_connection->socket().available());

                    asio::error_code read_err;
                    const size_t len = self->m_connection->socket().read_some(asio::buffer(buf), read_err);
                    // ToDo (a.gruzdev): Returns 0 but error is not set. Why?
                    if ((asio::error::eof == read_err) || (asio::error::connection_reset == read_err) || (0 == len)) {
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

        std::weak_ptr<tcp_receiver> weak_from_this_impl_()
        {
#if defined(YATO_CXX17) || (defined(_MSC_VER) && (_MSC_VER >= 1910))
            return weak_from_this();
#else
            return std::weak_ptr<tcp_receiver>(shared_from_this());
#endif
        }

        // http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/overview/core/reactor.html
        void start_receive_()
        {
            m_connection->socket().async_read_some(asio::null_buffers(), 
                std::bind(&handle_receive_, weak_from_this_impl_(), std::placeholders::_1, std::placeholders::_2));
        }

        tcp_receiver(const std::shared_ptr<tcp_connection> & connection, const actor_ref & remote, const actor_ref & handler)
            : m_connection(connection), m_remote(remote), m_handler(handler) 
        {
            m_connection->socket().non_blocking(true);
        }

    public:
        ~tcp_receiver() = default;

        tcp_receiver(const tcp_receiver&) = delete;
        tcp_receiver(tcp_receiver&&) = delete;

        tcp_receiver& operator=(const tcp_receiver&) = delete;
        tcp_receiver& operator=(tcp_receiver&&) = delete;

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
            asio::error_code err;
            const size_t len = m_connection->socket().send(asio::buffer(data), 0, err);
            return !err && (len == data.size());
        }

        static
        std::shared_ptr<tcp_receiver> create(const std::shared_ptr<tcp_connection> & connection, const actor_ref & remote, const actor_ref & handler)
        {
            std::shared_ptr<tcp_receiver> p;
            p.reset(new tcp_receiver(connection, remote, handler));
            p->start_receive_();
            return p;
        }
    };

} // namespace io

} // namespace actors

} // namespace yato

#endif // _YATO_ACTORS_IO_RECEIVER_H_

