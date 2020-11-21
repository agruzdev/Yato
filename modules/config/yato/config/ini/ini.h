/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_INI_H_
#define _YATO_CONFIG_INI_INI_H_

#include <istream>
#include <string>
#include "yato/config/config.h"

namespace yato {

namespace conf {

    namespace ini {

        /**
         * Parse Ini from string
         */
        config read(const char* str, size_t len = yato::nolength);

        /**
         * Parse Ini from string
         */
        config read(const std::string& str);

        /**
         * Parse Ini from stream
         */
        config read(std::istream& is);

    } // namespace ini


} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_INI_INI_H_
