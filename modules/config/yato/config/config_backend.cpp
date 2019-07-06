/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "config_backend.h"

namespace yato
{

namespace conf
{

    const config_backend::key_value_t config_backend::novalue = std::make_pair(std::string{}, nullptr);

} // namespace conf

} // namespace yato
