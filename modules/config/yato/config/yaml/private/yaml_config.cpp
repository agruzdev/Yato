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
        case config_property::keeps_order:
            return (m_node.Type() == YAML::NodeType::Sequence);
        default:
            return false;
        }
    }

    config_backend::find_index_result_t yaml_config::do_find(size_t index) const
    {
        YATO_REQUIRES(m_node.Type() == YAML::NodeType::Map || m_node.Type() == YAML::NodeType::Sequence);
        find_index_result_t result = config_backend::no_index_result;
        if(index < m_node.size()) {
            const auto it = std::next(m_node.begin(), index);
            if (m_node.Type() == YAML::NodeType::Map) {
                if ((*it).second.IsDefined()) {
                    result = std::make_tuple((*it).first.Scalar(), new yaml_value((*it).second));
                }
            }
            else {
                if ((*it).IsDefined()) {
                    std::get<1>(result) = new yaml_value(*it);
                }
            }
        }
        return result;
    }

    config_backend::find_key_result_t yaml_config::do_find(const std::string & name) const
    {
        find_key_result_t result = config_backend::no_key_result;
        if (m_node.Type() == YAML::NodeType::Map) {
            // YAML::Node::operator[] uses linear search
            const auto it = std::find_if(m_node.begin(), m_node.end(), [&name](const std::pair<YAML::Node, YAML::Node>& kv) { return kv.first.Scalar() == name; });
            if (it != m_node.end() && (*it).first.IsDefined()) {
                result = std::make_tuple(yato::narrow_cast<size_t>(std::distance(m_node.begin(), it)), new yaml_value((*it).second));
            }
        }
        return result;
    }

    void yaml_config::do_release(const config_value* val) const noexcept
    {
        delete val;
    }

    std::vector<std::string> yaml_config::do_enumerate_keys() const
    {
        std::vector<std::string> keys;
        if (m_node.Type() == YAML::NodeType::Map) {
            keys.reserve(m_node.size());
            std::for_each(m_node.begin(), m_node.end(), [&keys](const auto & kv) {
                keys.push_back(kv.first.Scalar());
            });
        }
        return keys;
    }


} // namespace conf

} // namespace yato

