/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "ini_section.h"
#include "ini_config.h"

namespace yato {

namespace conf {

    ini_section::ini_section(std::shared_ptr<ini_config> subsection)
        : m_subsection(std::move(subsection))
    { }

    ini_section::~ini_section() = default;

    stored_variant ini_section::get_as(stored_type dst_type) const noexcept
    {
        stored_variant res;
        if (dst_type == stored_type::config) {
            res.emplace<backend_ptr>(m_subsection);
        }
        return res;
    }

} // namespace conf

} // namespace yato

