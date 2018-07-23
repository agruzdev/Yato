/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#include "config_backend.h"

namespace yato
{

namespace conf
{
    // In order to avoid linker warning, that file defones no symbols
    config_backend::~config_backend() //NOLINT
    { }

} // namespace conf

} // namespace yato
