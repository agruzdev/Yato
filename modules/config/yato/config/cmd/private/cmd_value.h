/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_CMD_PRIVATE_CMD_VALUE_H_
#define _YATO_CONFIG_CMD_PRIVATE_CMD_VALUE_H_

#include <memory>

#include <tclap/CmdLine.h>

#include "../cmd.h"
#include "../../config_backend.h"
#include "../../utility.h"

#include "yato/prerequisites.h"

namespace yato {

namespace conf {


    class cmd_value final
        : public config_value
    {
    public:
        cmd_value(argument_type arg_type, stored_type value_type, const std::string & flag, const std::string & name, const std::string & description, stored_variant default_value = stored_variant{})
            : m_type(value_type)
        {
            switch(value_type) {
                case stored_type::integer:
                case stored_type::real:
                case stored_type::string:
                    switch(arg_type) {
                        case argument_type::positional:
                            m_arg = std::make_unique<TCLAP::UnlabeledValueArg<std::string>>(name, description, true, std::string{}, to_string(value_type));
                            break;
                        case argument_type::required:
                            m_arg = std::make_unique<TCLAP::ValueArg<std::string>>(flag, name, description, true, std::string{}, to_string(value_type), nullptr);
                            break;
                        case argument_type::optional:
                            m_arg = std::make_unique<TCLAP::ValueArg<std::string>>(flag, name, description, false, std::string{}, to_string(value_type), nullptr);
                            m_default = std::move(default_value);
                            break;
                    }
                    break;
                case stored_type::boolean:
                    // alaways an opional switch
                    m_arg = std::make_unique<TCLAP::SwitchArg>(flag, name, description, false);
                    m_default = stored_variant(yato::in_place_type_t<bool>{}, false);
                    break;
                default:
                    throw config_error("config_value[ctor]: Invalid value type.");
            }
        }

        ~cmd_value() override = default;

        stored_type type() const noexcept override
        {
            return m_type;
        }

        stored_variant get() const noexcept override
        {
            stored_variant res{};
            if (m_arg && m_arg->isSet()) {
                switch(m_type) {
                    case stored_type::integer:
                    case stored_type::real:
                    case stored_type::string: {
                            auto string_arg = static_cast<TCLAP::ValueArg<std::string>*>(m_arg.get());
                            if (!cvt_from(string_arg->getValue(), res, m_type)) {
                                // ToDo (a.gruzdev): Report error here
                            }
                        }
                        break;
                    case stored_type::boolean: {
                            auto switch_arg = static_cast<TCLAP::SwitchArg*>(m_arg.get());
                            res.emplace<bool>(switch_arg->getValue());
                        }
                        break;
                    default:
                        break;
                }
            }
            else if(!m_default.is_type<void>()) {
                res = m_default;
            }
            return res;
        }

        /**
         * Is set or has default value.
         */
        bool valid() const
        {
            return (m_arg && m_arg->isSet()) || !m_default.is_type<void>();
        }

        TCLAP::Arg* arg_handle()
        {
            return m_arg.get();
        }

    private:
        std::unique_ptr<TCLAP::Arg> m_arg = nullptr;
        stored_type m_type;
        stored_variant m_default;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_CMD_PRIVATE_CMD_VALUE_H_
