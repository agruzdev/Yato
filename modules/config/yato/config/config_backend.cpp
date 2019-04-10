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
    // In order to avoid linker warning, that file defines no symbols
    config_backend::~config_backend() //NOLINT
    { }

} // namespace conf

} // namespace yato
