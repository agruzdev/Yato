/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "cmd.h"

#include "private/cmd_config.h"

namespace yato {

namespace conf {

    namespace
    {
        template <typename Ty_>
        stored_variant to_variant(const yato::optional<Ty_> & opt)
        {
            return !opt.empty()
                ? stored_variant(yato::in_place_type_t<Ty_>{}, opt.get())
                : stored_variant();
        }
    }

    cmd_builder::cmd_builder(const std::string & description)
    {
        m_conf = std::make_unique<cmd_config>(description, "");
    }

    cmd_builder::cmd_builder(const std::string & description, const std::string & version)
    {
        m_conf = std::make_unique<cmd_config>(description, version);
    }

    cmd_builder::cmd_builder(cmd_builder&&) noexcept = default;

    cmd_builder& cmd_builder::operator=(cmd_builder&&) noexcept = default;

    cmd_builder::~cmd_builder() = default;

    cmd_builder& cmd_builder::integer(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description, const yato::optional<int64_t> & default_value)
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[integer]: builder is empty after creating config.");
        }
        m_conf->add(name, std::make_unique<cmd_value>(arg_type, stored_type::integer, flag, name, description, to_variant<int64_t>(default_value)));
        return *this;
    }

    cmd_builder& cmd_builder::floating(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description, const yato::optional<double> & default_value)
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[floating]: builder is empty after creating config.");
        }
        m_conf->add(name, std::make_unique<cmd_value>(arg_type, stored_type::real, flag, name, description, to_variant<double>(default_value)));
        return *this;
    }

    cmd_builder& cmd_builder::string(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description, const yato::optional<std::string> & default_value)
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[string]: builder is empty after creating config.");
        }
        m_conf->add(name, std::make_unique<cmd_value>(arg_type, stored_type::string, flag, name, description, to_variant<std::string>(default_value)));
        return *this;
    }

    cmd_builder& cmd_builder::boolean(const std::string & flag, const std::string & name, const std::string & description)
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[boolean]: builder is empty after creating config.");
        }
        m_conf->add(name, std::make_unique<cmd_value>(argument_type::optional, stored_type::boolean, flag, name, description));
        return *this;
    }

    config cmd_builder::parse(int argc, const char* const* argv)
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[parse]: builder is empty after creating config.");
        }
        m_conf->parse(argc, argv);
        return config(std::move(m_conf));
    }

    config cmd_builder::parse(const yato::array_view_1d<std::string> & args)
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[parse]: builder is empty after creating config.");
        }

        const int argc = yato::narrow_cast<int>(args.size());
        std::vector<const char*> argv;
        argv.reserve(argc);
        for(const std::string & str : args.crange()) {
            argv.push_back(str.c_str());
        }
        m_conf->parse(argc, argv.data());

        return config(std::move(m_conf));
    }

    config cmd_builder::parse(const std::vector<std::string> & args)
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[parse]: builder is empty after creating config.");
        }
        m_conf->parse(args);
        return config(std::move(m_conf));
    }

} // namesapce conf

} // namespace yato

