/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "yaml_config.h"
#include "yaml_value.h"

namespace yato {

namespace conf {

    yaml_config::yaml_config(YAML::Node node)
        : m_node(std::move(node))
    {
        if (!m_node.IsDefined() || (m_node.Type() != YAML::NodeType::Map && m_node.Type() != YAML::NodeType::Sequence)) {
            throw yato::conf::config_error("yaml_config[ctor]: Invalid node type.");
        }
    }

    yaml_config::~yaml_config() = default;

    size_t yaml_config::do_size() const noexcept
    {
        try {
            return m_node.size();
        }
        catch (std::runtime_error & /*err*/) {
            // ToDo (a.gruzdev): Report error here
        }
        return 0;
    }

    bool yaml_config::do_has_property(config_property p) const noexcept
    {
        switch (p) {
        case config_property::associative:
            return (m_node.Type() == YAML::NodeType::Map);
        case config_property::ordered:
            return (m_node.Type() == YAML::NodeType::Sequence);
        default:
            return false;
        }
    }

    config_backend::key_value_t yaml_config::do_find(size_t index) const noexcept
    {
        YATO_REQUIRES(m_node.Type() == YAML::NodeType::Map || m_node.Type() == YAML::NodeType::Sequence);
        std::string name;
        std::unique_ptr<yaml_value> res;
        try {
            if(index < m_node.size()) {
                const auto it = std::next(m_node.begin(), index);
                if (m_node.Type() == YAML::NodeType::Map) {
                    if ((*it).second.IsDefined()) {
                        name = (*it).first.Scalar();
                        res  = std::make_unique<yaml_value>((*it).second);
                    }
                }
                else {
                    if ((*it).IsDefined()) {
                        res  = std::make_unique<yaml_value>(*it);
                    }
                }
            }
        }
        catch(std::exception & /*err*/) {
            // ToDo (a.gruzdev): report error here
        }
        return std::make_pair(name, res.release());
    }

    config_backend::key_value_t yaml_config::do_find(const std::string & name) const noexcept
    {
        std::unique_ptr<yaml_value> res;
        if (m_node.Type() == YAML::NodeType::Map) {
            try {
                const YAML::Node it = m_node[name];
                if (it.IsDefined()) {
                    res = std::make_unique<yaml_value>(it);
                }
            }
            catch (std::exception & /*err*/) {
                // ToDo (a.gruzdev): report error here
            }
        }
        return std::make_pair(name, res.release());
    }

    void yaml_config::do_release(const config_value* val) const noexcept
    {
        delete val;
    }

    std::vector<std::string> yaml_config::do_enumerate_keys() const noexcept
    {
        std::vector<std::string> keys;
        if (m_node.Type() == YAML::NodeType::Map) {
            try {
                keys.reserve(m_node.size());
                std::for_each(m_node.begin(), m_node.end(), [&keys](const auto & kv) {
                    keys.push_back(kv.first.Scalar());
                });
            }
            catch(std::exception & /*err*/) {
                // ToDo (a.gruzdev): Report error here
            }
        }
        return keys;
    }


} // namespace conf

} // namespace yato

