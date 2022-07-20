/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
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
        m_conf = std::make_shared<cmd_config>(description, "");
    }

    cmd_builder::cmd_builder(const std::string & description, const std::string & version)
    {
        m_conf = std::make_shared<cmd_config>(description, version);
    }

    cmd_builder::cmd_builder(cmd_builder&&) noexcept = default;

    cmd_builder& cmd_builder::operator=(cmd_builder&&) noexcept = default;

    cmd_builder::~cmd_builder() = default;

    cmd_builder& cmd_builder::integer(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description, const yato::optional<int64_t> & default_value) &
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[integer]: builder is empty after creating config.");
        }
        m_conf->add(name, std::make_unique<cmd_value>(arg_type, stored_type::integer, flag, name, description, to_variant<int64_t>(default_value)));
        return *this;
    }

    cmd_builder&& cmd_builder::integer(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description, const yato::optional<int64_t> & default_value) &&
    {
        return std::move(this->integer(arg_type, flag, name, description, default_value));
    }

    cmd_builder& cmd_builder::floating(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description, const yato::optional<double> & default_value) &
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[floating]: builder is empty after creating config.");
        }
        m_conf->add(name, std::make_unique<cmd_value>(arg_type, stored_type::real, flag, name, description, to_variant<double>(default_value)));
        return *this;
    }

    cmd_builder&& cmd_builder::floating(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description, const yato::optional<double> & default_value) &&
    {
        return std::move(this->floating(arg_type, flag, name, description, default_value));
    }

    cmd_builder& cmd_builder::string(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description, const yato::optional<std::string> & default_value) &
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[string]: builder is empty after creating config.");
        }
        m_conf->add(name, std::make_unique<cmd_value>(arg_type, stored_type::string, flag, name, description, to_variant<std::string>(default_value)));
        return *this;
    }

    cmd_builder&& cmd_builder::string(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description, const yato::optional<std::string> & default_value) &&
    {
        return std::move(this->string(arg_type, flag, name, description, default_value));
    }

    cmd_builder& cmd_builder::boolean(const std::string & flag, const std::string & name, const std::string & description) &
    {
        if (m_conf == nullptr) {
            throw config_error("cmd_builder[boolean]: builder is empty after creating config.");
        }
        m_conf->add(name, std::make_unique<cmd_value>(argument_type::optional, stored_type::boolean, flag, name, description));
        return *this;
    }

    cmd_builder&& cmd_builder::boolean(const std::string & flag, const std::string & name, const std::string & description) &&
    {
        return std::move(this->boolean(flag, name, description));
    }

    cmd_builder& cmd_builder::set_ostream(std::ostream* os) &
    {
        m_conf->set_ostream(os);
        return *this;
    }

    cmd_builder&& cmd_builder::set_ostream(std::ostream* os) &&
    {
        return std::move(this->set_ostream(os));
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



    struct cmd_dispatcher::cmd_dispatcher_impl
    {
        std::string description;
        std::map<std::string, std::pair<alternative_handler, cmd_builder>> alternatives;
        std::ostream* dst_stream = &std::cout;
        return_type error_code = 1;

        static
        yato::optional<std::string> get_first_positional(int argc, const char* const* argv)
        {
            if (argc <= 1 || !argv[1]) {
                return yato::nullopt;
            }
            return std::string(argv[1]);
        }

        template <typename StrVec_>
        static
        yato::optional<std::string> get_first_positional(const StrVec_ & args)
        {
            if (args.size() <= 1) {
                return yato::nullopt;
            }
            return args[1];
        }

        static
        std::vector<const char*> select_the_rest(int argc, const char* const* argv)
        {
            std::vector<const char*> new_argv;
            if (argc > 0) {
                new_argv.push_back(argv[0]);
                for (int i = 2; i < argc; ++i) {
                    new_argv.push_back(argv[i]);
                }
            }
            return new_argv;
        }

        template <typename StrVec_>
        static
        std::vector<const char*> select_the_rest(const StrVec_ & args)
        {
            std::vector<const char*> new_argv;
            if (args.size() > 0) {
                new_argv.push_back(args[0].c_str());
                for (size_t i = 2; i < args.size(); ++i) {
                    new_argv.push_back(args[i].c_str());
                }
            }
            return new_argv;
        }

        cmd_dispatcher::return_type handle_error(const std::string& message)
        {
            if (dst_stream) {
                (*dst_stream) << message << std::endl << description << std::endl;
                return error_code;
            }
            else {
                throw yato::config_error(message);
            }
        }

        template <typename... Args_>
        cmd_dispatcher::return_type parse_impl(Args_&&... args)
        {
            const yato::optional<std::string> key = get_first_positional(std::forward<Args_>(args)...);
            if (!key) {
                return handle_error("Command line is empty");
            }
            auto it = alternatives.find(key.get());
            if (it == alternatives.cend()) {
                return handle_error("Unknown command line alternative: '" + key.get() + "'");
            }
            const std::vector<const char*> new_argv = select_the_rest(std::forward<Args_>(args)...);
            try {
                return std::get<0>(it->second)(std::get<1>(it->second)
                    .set_ostream(dst_stream)
                    .parse(yato::narrow_cast<int>(new_argv.size()), new_argv.data()));
            }
            catch (TCLAP::ExitException& exit) {
                return exit.getExitStatus();
            }
        }

    };


    cmd_dispatcher::cmd_dispatcher(const std::string& usage_description)
        : m_pimpl(std::make_unique<cmd_dispatcher_impl>())
    {
        m_pimpl->description = usage_description;
    }

    cmd_dispatcher::cmd_dispatcher(cmd_dispatcher&&) noexcept = default;

    cmd_dispatcher::~cmd_dispatcher() = default;

    cmd_dispatcher& cmd_dispatcher::operator=(cmd_dispatcher&&) noexcept = default;

    cmd_dispatcher& cmd_dispatcher::add_alternative(std::string key, alternative_handler handler, cmd_builder cmd) &
    {
        assert(m_pimpl);
        m_pimpl->alternatives.insert_or_assign(std::move(key), std::make_pair(std::move(handler), std::move(cmd)));
        return *this;
    }

    cmd_dispatcher&& cmd_dispatcher::add_alternative(std::string key, alternative_handler handler, cmd_builder cmd) &&
    {
        return std::move(this->add_alternative(std::move(key), std::move(handler), std::move(cmd)));
    }

    cmd_dispatcher& cmd_dispatcher::set_ostream(std::ostream* os) &
    {
        assert(m_pimpl);
        m_pimpl->dst_stream = os;
        return *this;
    }

    cmd_dispatcher&& cmd_dispatcher::set_ostream(std::ostream* os) &&
    {
        return std::move(this->set_ostream(os));
    }

    cmd_dispatcher& cmd_dispatcher::set_error_code(return_type value) &
    {
        assert(m_pimpl);
        m_pimpl->error_code = value;
        return *this;
    }

    cmd_dispatcher&& cmd_dispatcher::set_error_code(return_type value) &&
    {
        return std::move(this->set_error_code(value));
    }

    cmd_dispatcher::return_type cmd_dispatcher::parse(int argc, const char* const* argv)
    {
        assert(m_pimpl);
        return m_pimpl->parse_impl(argc, argv);
    }

    cmd_dispatcher::return_type cmd_dispatcher::parse(const yato::array_view_1d<std::string> & args)
    {
        assert(m_pimpl);
        return m_pimpl->parse_impl(args);
    }

    cmd_dispatcher::return_type cmd_dispatcher::parse(const std::vector<std::string> & args)
    {
        assert(m_pimpl);
        return m_pimpl->parse_impl(args);
    }

} // namesapce conf

} // namespace yato

