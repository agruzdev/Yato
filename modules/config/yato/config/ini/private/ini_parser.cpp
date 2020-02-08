/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "ini_parser.h"
#include "yato/config/config_backend.h"

namespace yato {

namespace conf {


    static
    int on_value_parsed_(void* context, const char* section, const char* name, const char* value)
    {
        ini_parser* thiz = static_cast<ini_parser*>(context);
        if (name && value) {
            std::string section_name(section);
            if (!section_name.empty()) {
                thiz->sections[section_name].emplace(name, value);
            }
            else {
                thiz->global_values.emplace(name, value);
            }
        }
        return 1;
    }


    std::shared_ptr<ini_parser> ini_parser::parse_file(const char* filename)
    {
        auto result = std::make_shared<ini_parser>();
        if (0 != ini_parse(filename, &on_value_parsed_, result.get())) {
            throw yato::conf::config_error("Failed to parse ini file " + std::string(filename));
        }
        return result;
    }

    std::shared_ptr<ini_parser> ini_parser::parse_c_string(const char* str)
    {
        auto result = std::make_shared<ini_parser>();
        if (0 != ini_parse_string(str, &on_value_parsed_, result.get())) {
            throw yato::conf::config_error("Failed to parse ini string.");
        }
        return result;
    }




} // namespace conf

} // namespace yato

