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

namespace yato {

namespace conf {

    struct ini_builder_state
    {
        std::shared_ptr<ini_parser> parser = nullptr;
    };

    ini_builder::ini_builder()
    {
        m_impl = std::make_unique<ini_builder_state>();
    }

    ini_builder::~ini_builder() = default;


    config ini_builder::parse(const char* ini)
    {
        m_impl->parser = ini_parser::parse_c_string(ini);
        return finalize_();
    }

    config ini_builder::parse(const std::string& ini)
    {
        m_impl->parser = ini_parser::parse_c_string(ini.c_str());
        return finalize_();
    }

    config ini_builder::parse_file(const char* filename)
    {
        m_impl->parser = ini_parser::parse_file(filename);
        return finalize_();
    }

    config ini_builder::parse_file(const std::string& filename)
    {
        m_impl->parser = ini_parser::parse_file(filename.c_str());
        return finalize_();
    }

    config ini_builder::finalize_()
    {
        return config(std::make_shared<ini_config>(m_impl->parser));
    }

} // namespace conf

} // namespace yato


