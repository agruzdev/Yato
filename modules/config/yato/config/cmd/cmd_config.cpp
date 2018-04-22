/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#include <map>

#include <tclap/CmdLine.h>

#include "cmd_config.h"

namespace yato {

namespace conf {

    /**
     * Wrapper class as workaround for TCLAP issue with long long trait redefinition.
     */
    struct integer_wrapper
    {
        int64_t val;

        YATO_CONSTEXPR_FUNC
        integer_wrapper(int64_t val) 
            : val(val)
        { }
    };

    inline
    std::istream & operator >> (std::istream & is, integer_wrapper & wrapper)
    {
        is >> wrapper.val;
        return is;
    }

    //--------------------------------------------------------------------------------------


    struct argument_info  // NOLINT
    {
        std::unique_ptr<TCLAP::Arg> value;
        config_type type;
    };

    //--------------------------------------------------------------------------------------


    class cmd_config_state
    {
    private:
        TCLAP::CmdLine mCmd;
        std::map<std::string, argument_info> mArgs;

    public:
        cmd_config_state(const std::string & description, const std::string & version)
            : mCmd(description, ' ', version)
        { }
        
        ~cmd_config_state() = default;

        void add(argument_info && arg)
        {
            YATO_REQUIRES(arg.value != nullptr);

            const std::string name = arg.value->getName();
            mCmd.add(arg.value.get());
            mArgs[name] = std::move(arg);
        }

        // Return not const pointer since ValueArg::getValue() is not const
        TCLAP::Arg* find(const std::string & name, config_type type) const
        {
            const auto it = mArgs.find(name);
            if(it != mArgs.cend() && (*it).second.type == type) {
                return (*it).second.value.get();
            }
            return nullptr;
        }

        void parse(int argc, const char* const* argv) {
            mCmd.parse(argc, argv);
        }
    };

    //-------------------------------------------------------------------------


    cmd_config::cmd_config(std::unique_ptr<cmd_config_state> && impl)
        : m_impl(std::move(impl))
    { }

    cmd_config::~cmd_config() = default;

    cmd_config::cmd_config(cmd_config&&) noexcept = default;

    cmd_config& cmd_config::operator=(cmd_config&&) noexcept = default;

    bool cmd_config::do_is_object() const noexcept
    {
        return true;
    }

    stored_variant cmd_config::do_get_by_name(const std::string & name, config_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        stored_variant res{};

        TCLAP::Arg* arg = m_impl->find(name, type);
        switch (type)
        {
        case yato::conf::config_type::integer: {
                auto value = dynamic_cast<TCLAP::ValueArg<integer_wrapper>*>(arg);
                if(value != nullptr) {
                    using return_type = stored_type_trait<config_type::integer>::return_type;
                    res.emplace<return_type>(yato::narrow_cast<return_type>(value->getValue().val));
                }
            }
            break;
        case yato::conf::config_type::boolean: {
                auto value = dynamic_cast<TCLAP::SwitchArg*>(arg);
                if(value != nullptr) {
                    using return_type = stored_type_trait<config_type::boolean>::return_type;
                    res.emplace<return_type>(value->getValue());
                }
            }
            break;
        case yato::conf::config_type::floating: {
                auto value = dynamic_cast<TCLAP::ValueArg<double>*>(arg);
                if(value != nullptr) {
                    using return_type = stored_type_trait<config_type::floating>::return_type;
                    res.emplace<return_type>(yato::narrow_cast<return_type>(value->getValue()));
                }
            }
            break;
        case yato::conf::config_type::string: {
                auto value = dynamic_cast<TCLAP::ValueArg<std::string>*>(arg);
                if(value != nullptr) {
                    using return_type = stored_type_trait<config_type::string>::return_type;
                    res.emplace<return_type>(value->getValue());
                }
            }
            break;
        default:
            break;
        }

        return res;
    }

    stored_variant cmd_config::do_get_by_index(size_t index, config_type type) const noexcept
    {
        YATO_MAYBE_UNUSED(index);
        YATO_MAYBE_UNUSED(type);
        return yato::nullvar_t{};
    }

    bool cmd_config::do_is_array() const noexcept
    {
        return false;
    }

    size_t cmd_config::do_get_size() const noexcept
    {
        return 0;
    }

    //-------------------------------------------------------------------------------------
    // Command line config builder

    cmd_builder::cmd_builder(const std::string & description)
    {
        m_impl = std::make_unique<cmd_config_state>(description, "");
    }

    cmd_builder::cmd_builder(const std::string & description, const std::string & version)
    {
        m_impl = std::make_unique<cmd_config_state>(description, version);
    }

    cmd_builder::~cmd_builder() = default;

    cmd_builder::cmd_builder(cmd_builder&&) noexcept = default;

    cmd_builder& cmd_builder::operator=(cmd_builder&&) noexcept = default;

    cmd_builder& cmd_builder::integer(const std::string & flag, const std::string & name, const std::string & description, const yato::optional<int64_t> & default_value)
    {
        if(m_impl == nullptr) {
            throw config_error("cmd_builder is empty after creating config.");
        }

        argument_info arg;
        arg.type  = config_type::integer;
        arg.value = std::make_unique<TCLAP::ValueArg<integer_wrapper>>(flag, name, description, default_value.empty(), default_value.get_or(0), "Integer type", nullptr);

        m_impl->add(std::move(arg));

        return *this;
    }

    cmd_builder& cmd_builder::floating(const std::string & flag, const std::string & name, const std::string & description, const yato::optional<double> & default_value)
    {
        if(m_impl == nullptr) {
            throw config_error("cmd_builder is empty after creating config.");
        }

        argument_info arg;
        arg.type  = config_type::floating;
        arg.value = std::make_unique<TCLAP::ValueArg<double>>(flag, name, description, default_value.empty(), default_value.get_or(0.0), "Floating-point type", nullptr);

        m_impl->add(std::move(arg));

        return *this;
    }

    cmd_builder& cmd_builder::string(const std::string & flag, const std::string & name, const std::string & description, const yato::optional<std::string> & default_value)
    {
        if(m_impl == nullptr) {
            throw config_error("cmd_builder is empty after creating config.");
        }

        argument_info arg;
        arg.type  = config_type::string;
        arg.value = std::make_unique<TCLAP::ValueArg<std::string>>(flag, name, description, default_value.empty(), default_value.get_or(std::string()), "String", nullptr);

        m_impl->add(std::move(arg));

        return *this;
    }

    cmd_builder& cmd_builder::boolean(const std::string & flag, const std::string & name, const std::string & description)
    {
        if(m_impl == nullptr) {
            throw config_error("cmd_builder is empty after creating config.");
        }

        argument_info arg;
        arg.type  = config_type::boolean;
        arg.value = std::make_unique<TCLAP::SwitchArg>(flag, name, description, false);

        m_impl->add(std::move(arg));

        return *this;
    }

    config cmd_builder::parse(int argc, const char* const* argv)
    {
        if(m_impl == nullptr) {
            throw config_error("cmd_builder is empty after creating config.");
        }
        m_impl->parse(argc, argv);
        return config(std::make_shared<cmd_config>(std::move(m_impl)));
    }

    config cmd_builder::parse(const yato::array_view_1d<std::string> & args)
    {
        if(m_impl == nullptr) {
            throw config_error("cmd_builder is empty after creating config.");
        }

        const int argc = yato::narrow_cast<int>(args.size());
        std::vector<const char*> argv;
        argv.reserve(argc);
        for(const std::string & str : args.crange()) {
            argv.push_back(str.c_str());
        }
        m_impl->parse(argc, argv.data());

        return config(std::make_shared<cmd_config>(std::move(m_impl)));
    }

} // namespace conf

} // namespace yato

/**
 * Enable int64_t
 */
namespace TCLAP
{
    template<>
    struct ArgTraits<yato::conf::integer_wrapper> {
        typedef ValueLike ValueCategory;
    };
}
