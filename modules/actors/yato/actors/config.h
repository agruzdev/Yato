/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_CONFIG_H_
#define _YATO_ACTOR_CONFIG_H_

#include "logger.h"

namespace yato
{
namespace actors
{

    class config
    {
    public:
        log_level log_filter = log_level::info;

        bool enable_io = false;

        //-------------------------------------

        static config defaults() {
            return config{};
        }
    };

} // namespacea actors

} // namespace yato

#endif //_YATO_ACTOR_CONFIG_H_

