/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "ini.h"

#include <fstream>

#include "private/ini_lib.h"
#include "private/ini_config.h"

namespace yato {

namespace conf {

    struct ini_builder_state
    {
        std::shared_ptr<CSimpleIniA> reader;
    };

    ini_builder::ini_builder()
    {
        m_impl = std::make_unique<ini_builder_state>();
        m_impl->reader = std::make_shared<CSimpleIniA>();
        m_impl->reader->SetMultiLine(true);
        m_impl->reader->SetMultiKey(false);
        m_impl->reader->SetUnicode(false);
    }

    ini_builder::~ini_builder() = default;

    ini_builder& ini_builder::multiline(bool enable)
    {
        m_impl->reader->SetMultiLine(enable);
        return *this;
    }

    config ini_builder::parse(std::istream & is)
    {
        m_impl->reader->LoadData(is);
        return finalize_();
    }

    config ini_builder::parse(const char* ini)
    {
        m_impl->reader->LoadData(ini, std::strlen(ini));
        return finalize_();
    }

    config ini_builder::parse(const std::string& ini)
    {
        m_impl->reader->LoadData(ini.c_str(), ini.size());
        return finalize_();
    }

    config ini_builder::parse_file(const char* filename)
    {
        std::ifstream file(filename, std::ios::binary);
        return parse(file);
    }

    config ini_builder::parse_file(const std::string & filename)
    {
        std::ifstream file(filename, std::ios::binary);
        return parse(file);
    }

    config ini_builder::finalize_()
    {
        return config(std::make_shared<ini_config>(m_impl->reader));
    }

} // namespace conf

} // namespace yato


