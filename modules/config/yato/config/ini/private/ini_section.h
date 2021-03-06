/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_PRIVATE_INI_SECTION_H_
#define _YATO_CONFIG_INI_PRIVATE_INI_SECTION_H_

#include <memory>
#include "../../config_backend.h"
#include "ini_parser.h"

namespace yato {

namespace conf {

    class ini_config;

    class ini_section final
        : public config_value
    {
    public:
        ini_section(std::shared_ptr<ini_config> subsection);

        ini_section(const ini_section&) = delete;
        ini_section(ini_section&&) = delete;

        ini_section& operator=(const ini_section&) = delete;
        ini_section& operator=(ini_section&&) = delete;

        ~ini_section() override;

        stored_type type() const noexcept override;

        stored_variant get() const noexcept override;

    private:
        std::shared_ptr<ini_config> m_subsection;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_INI_PRIVATE_INI_SECTION_H_
