/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/

#ifndef _YATO_EXECUTION_CONTEXT_H_
#define _YATO_EXECUTION_CONTEXT_H_

#include "../config.h"

#include "abstract_executor.h"
#include "pinned_executor.h"
#include "dynamic_executor.h"

namespace yato
{
namespace actors
{

    struct execution_context
    {
        std::string name;
        std::unique_ptr<abstract_executor> executor;

        //----------------------------------------------------------------
        static
        execution_context get_default(actor_system* system, const std::string & name) {
            execution_context ctx;
            ctx.name = name;
            ctx.executor =  std::make_unique<dynamic_executor>(system, 4, 5);
            return ctx;
        }
    };


    class execution_context_converter
    {
    private:
        actor_system* m_system;

    public:
        execution_context_converter(actor_system* system)
            : m_system(system)
        {
            YATO_ENSURES(m_system != nullptr);
        }

        execution_context operator()(const yato::config & conf) const
        {
            try {
                execution_context ctx;
                ctx.name = conf.value<std::string>("name").get();
                const std::string type = conf.value<std::string>("type").get();
                if(type == "thread_pool") {
                    const auto threads_num = conf.value<uint32_t>("threads_num").get_or(4);
                    const auto throughput  = conf.value<uint32_t>("throughput").get_or(5);
                    ctx.executor = std::make_unique<dynamic_executor>(m_system, threads_num, throughput);
                }
                else if(type == "pinned") {
                    const auto threads_limit = conf.value<uint32_t>("threads_limit").get_or(16);
                    ctx.executor = std::make_unique<pinned_executor>(m_system, threads_limit);
                }
                else {
                    throw yato::config_error("Failed to deserialize excution_context: Unknown executor type!");
                }
                return ctx;
            }
            catch (yato::bad_optional_access & err) {
                throw yato::config_error("Failed to deserialize excution_context: " + std::string(err.what()));
            }
            catch (...) {
                // rethrow other exceptions
                throw;
            }
        }
    };


}// namespace actors

namespace conf
{

    template <>
    struct config_value_trait<actors::execution_context>
    {
        using converter_type = actors::execution_context_converter;
        static constexpr stored_type fetch_type = stored_type::config;
    };

} // namespace conf

}// namespace yato

#endif //_YATO_EXECUTION_CONTEXT_H_

