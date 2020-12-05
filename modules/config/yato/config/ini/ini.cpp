/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "ini.h"

#include <fstream>

#include "private/ini_parser.h"
#include "private/ini_config.h"
#include "yato/config/utility.h"

namespace yato {

namespace conf {

    namespace ini {

        config read(const char* str, size_t len)
        {
            std::shared_ptr<ini_parser> parser;
            if (len != yato::nolength) {
                std::string str_copy(str, len);
                parser = ini_parser::parse_c_string(str_copy.c_str());
            }
            else {
                parser = ini_parser::parse_c_string(str);
            }
            return config(std::make_shared<ini_config>(std::move(parser)));
        }

        config read(const std::string& str)
        {
            return read(str.c_str(), yato::nolength);
        }

        config read(std::istream& is)
        {
            return read(get_text_stream_content(is));
        }

    } // namespace ini

} // namespace conf

} // namespace yato


