/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "ini_section.h"
#include "ini_config.h"

namespace yato {

namespace conf {

    ini_section::ini_section(std::shared_ptr<ini_config> subsection)
        : m_subsection(std::move(subsection))
    { }

    ini_section::~ini_section() = default;

    stored_type ini_section::type() const noexcept
    {
        return stored_type::config;
    }

    stored_variant ini_section::get() const noexcept
    {
        return stored_variant(yato::in_place_type_t<backend_ptr_t>{}, m_subsection);
    }

} // namespace conf

} // namespace yato

