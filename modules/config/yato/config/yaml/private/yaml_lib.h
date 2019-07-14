/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_YAML_PRIVATE_YAML_LIB_H_
#define _YATO_CONFIG_YAML_PRIVATE_YAML_LIB_H_

#include "yato/prerequisites.h"

#ifndef _NOEXCEPT
# define _NOEXCEPT YATO_NOEXCEPT_KEYWORD
#endif
YATO_PRAGMA_WARNING_PUSH
YATO_MSCV_WARNING_IGNORE(4127)
#include "yaml-cpp/yaml.h"
YATO_PRAGMA_WARNING_POP

#endif //_YATO_CONFIG_YAML_PRIVATE_YAML_LIB_H_
