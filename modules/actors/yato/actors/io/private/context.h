/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_CONTEXT_H_
#define _YATO_ACTORS_IO_CONTEXT_H_

#include <memory>
#include <thread>

#include <asio.hpp>

#include "../../logger.h"

namespace yato
{
namespace actors
{
namespace io
{

    class io_context
    {
    private:
        logger_ptr m_log = logger_factory::create("io_context");

        std::unique_ptr<asio::io_service> m_service;
        std::unique_ptr<asio::io_service::work> m_work;
        std::thread m_thread;

    public:
        io_context() {
            m_service = std::make_unique<asio::io_service>();
            m_work    = std::make_unique<asio::io_service::work>(*m_service);
            m_thread = std::thread([this]{
                m_log->debug("IO start");
                m_service->run();
                m_log->debug("IO stop");
            });
        }

        ~io_context() {
            m_work.reset();
            m_service->stop();
            m_thread.join();
        }

        io_context(const io_context&) = delete;
        io_context(io_context&&) = delete;

        io_context& operator= (const io_context&) = delete;
        io_context& operator= (io_context&&) = delete;


        const asio::io_service & service() const {
            return *m_service;
        }

        asio::io_service & service() {
            return *m_service;
        }
    };

} // namespace io

} // namespace actors

} // namespace yato


#endif // _YATO_ACTORS_IO_CONTEXT_H_
