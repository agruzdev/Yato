/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_PRIVATE_INI_LIB_H_
#define _YATO_CONFIG_INI_PRIVATE_INI_LIB_H_

#define SI_SUPPORT_IOSTREAMS 1
#define SI_CONVERT_GENERIC 1
#include "SimpleIni.h"

namespace yato {

namespace conf {

    using ini_section_ptr = decltype(std::declval<CSimpleIniA>().GetSection(std::declval<const char*>()));

} // namespace conf

} // namespace yato

#endif //#define _YATO_CONFIG_INI_PRIVATE_INI_LIB_H_
