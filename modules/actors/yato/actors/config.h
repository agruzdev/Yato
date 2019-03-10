/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
* 
* @details
* YatoActors config has the following structure:
* 
* JSON: {
* 
*   "execution_contexts" : [
*       {
*           "name" : "dynamic",
*           "type" : "thread_pool",
*           "threads_num": 4
*           "throughput": 5
*       },
*       {
*           "name" : "fixed",
*           "type" : "pinned",
*           "threads_limit": 8
*       }
*   ],
*   "default_executor" : "dynamic",
* 
*   "log_level": "info",
*   "enable_io": false
* }
* 
*/

#ifndef _YATO_ACTOR_CONFIG_H_
#define _YATO_ACTOR_CONFIG_H_

#include <map>
#include <yato/config/config.h>
#include "logger.h"

namespace yato
{
namespace actors
{

    struct config_converter_log
    {
        log_level operator()(const std::string & str) const
        {
            static const std::map<std::string, log_level> levels = {
                {"silent",  log_level::silent},
                {"error",   log_level::error},
                {"warning", log_level::warning},
                {"info",    log_level::info},
                {"debug",   log_level::debug},
                {"verbose", log_level::verbose}
            };
            const auto it = levels.find(str);
            if(it == levels.cend()) {
                throw yato::config_error("Unknown log parameter: " + str);
            }
            return (*it).second;
        }
    };

} // namespacea actors

namespace conf
{

    template <>
    struct config_value_trait<actors::log_level>
    {
        using converter_type = actors::config_converter_log;
        static constexpr stored_type fetch_type = stored_type::string;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_ACTOR_CONFIG_H_

