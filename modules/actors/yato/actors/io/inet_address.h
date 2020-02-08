/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_INET_ADDRESS_H_
#define _YATO_ACTORS_IO_INET_ADDRESS_H_

#include <string>

#include <yato/types.h>

namespace yato
{
namespace actors
{
namespace io
{

    struct inet_address
    {
        std::string host;
        uint16_t port;

        inet_address(const char* host, uint16_t port)
            : host(host), port(port)
        { }

        inet_address(const std::string & host, uint16_t port)
            : host(host), port(port)
        { }

        inet_address(std::string && host, uint16_t port)
            : host(std::move(host)), port(port)
        { }

        std::string to_string() const {
            return host + ":" + std::to_string(port);
        }
    };

} // namespace yato

} // namespace actors

} // namespace io

#endif // _YATO_ACTORS_IO_INET_ADDRESS_H_
