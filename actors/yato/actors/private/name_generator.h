/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_NAME_GENERATOR_H_
#define _YATO_ACTORS_NAME_GENERATOR_H_

#include <map>
#include <string>

namespace yato
{
namespace actors
{

    class name_generator
    {
        std::map<std::string, size_t> m_prefixes;

    public:
        name_generator()
        { }

        ~name_generator()
        { }

        name_generator(const name_generator&) = default;
        name_generator(name_generator&&) = default;

        name_generator& operator= (const name_generator&) = default;
        name_generator& operator= (name_generator&&) = default;

        std::string next_indexed(const std::string & prefix) 
        {
            auto it = m_prefixes.find(prefix);
            if(it == m_prefixes.end()) {
                it = m_prefixes.emplace_hint(it, prefix, 0);
            }
            return prefix + std::to_string((*it).second++);
        }
    };

}// namespace actors

}// namespace yato

#endif // _YATO_ACTORS_NAME_GENERATOR_H_

