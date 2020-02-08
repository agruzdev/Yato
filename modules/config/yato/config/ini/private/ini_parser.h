/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_PRIVATE_INI_PARSER_H_
#define _YATO_CONFIG_INI_PRIVATE_INI_PARSER_H_

#include <map>
#include <memory>
#include <string>
#include <ini.h>

namespace yato {

namespace conf {

    struct ini_parser
    {
        using kv_multimap = std::multimap<std::string, std::string>;


        kv_multimap global_values;
        std::map<std::string, ini_parser::kv_multimap> sections;


        static
        std::shared_ptr<ini_parser> parse_file(const char* filename);

        static
        std::shared_ptr<ini_parser> parse_c_string(const char* str);
    };


} // namespace conf

} // namespace yato

#endif //#define _YATO_CONFIG_INI_PRIVATE_INI_LIB_H_
