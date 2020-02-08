/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "yaml_value.h"
#include "yaml_config.h"

namespace yato {

namespace conf {

    yaml_value::yaml_value(YAML::Node node)
        : m_node(std::move(node))
    { }

    yaml_value::~yaml_value() = default;

    stored_type yaml_value::type() const noexcept
    {
        switch(m_node.Type()) {
            case YAML::NodeType::Scalar:
                return stored_type::string;
            default:
                return stored_type::config;
        }
    }

    stored_variant yaml_value::get() const noexcept
    {
        stored_variant res{};
        try {
            switch(m_node.Type()) {
                case YAML::NodeType::Scalar: {
                        using string_t = stored_type_trait<stored_type::string>::return_type;
                        res.emplace<string_t>(m_node.Scalar());
                    }
                    break;
                case YAML::NodeType::Map:
                case YAML::NodeType::Sequence: {
                        using config_t = stored_type_trait<stored_type::config>::return_type;
                        res.emplace<config_t>(std::make_shared<yaml_config>(m_node));
                    }
                    break;
                default:
                    break;
            }
        }
        catch(std::exception & /*err*/) {
            // ToDo (a.gruzdev): Report error
        }
        return res;
    }

} // namespace conf

} // namespace yato

