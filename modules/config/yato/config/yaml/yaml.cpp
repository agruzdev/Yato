/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "yaml.h"
#include "private/yaml_config.h"

namespace yato {

namespace conf {


    namespace yaml {

        template <typename InputTy_>
        config read_yaml_(InputTy_&& input)
        {
            backend_ptr backend = nullptr;
            auto root = YAML::Load(std::forward<InputTy_>(input));
            if (root.IsDefined() && (root.Type() == YAML::NodeType::Map || root.Type() == YAML::NodeType::Sequence)) {
                backend = std::make_shared<yaml_config>(std::move(root));
            }
            return config{ backend };
        }

        config read(const char* str, size_t len)
        {
            if (len != yato::nolength) {
                return read_yaml_(std::string(str, len));
            }
            else {
                return read_yaml_(str);
            }
        }

        config read(const std::string& str)
        {
            return read_yaml_(str);
        }

        config read(std::istream& is)
        {
            return read_yaml_(is);
        }

        template <typename Ty_>
        void put_(YAML::Node& node, const std::string& key, Ty_&& val) {
            if (node.Type() == YAML::NodeType::Map) {
                node[key] = val;
            }
            else {
                node.push_back(val);
            }
        }

        YAML::Node to_yaml_(const yato::config& c)
        {
            if (c.is_null()) {
                return YAML::Node(YAML::NodeType::Null);
            }
            auto yaml_node = c.is_associative() ? YAML::Node(YAML::NodeType::Map) : YAML::Node(YAML::NodeType::Sequence);
            for (auto entry : c) {
                if (entry) {
                    switch (entry.type()) {
                    case stored_type::integer:
                        put_(yaml_node, entry.key(), entry.value<int64_t>().get());
                        break;
                    case stored_type::real:
                        put_(yaml_node, entry.key(), entry.value<double>().get());
                        break;
                    case stored_type::boolean:
                        put_(yaml_node, entry.key(), entry.value<bool>().get());
                        break;
                    case stored_type::string:
                        put_(yaml_node, entry.key(), entry.value<std::string>().get());
                        break;
                    case stored_type::config:
                        put_(yaml_node, entry.key(), to_yaml_(entry.object()));
                        break;
                    }
                }
                else {
                    put_(yaml_node, entry.key(), YAML::Node(YAML::NodeType::Null));
                }
            }
            return yaml_node;
        }

        std::string write(const yato::config& c, uint32_t indent)
        {
            YAML::Emitter emitter;
            emitter.SetIndent(indent);
            emitter << to_yaml_(c);
            return std::string(emitter.c_str(), emitter.size());
        }

        void write(const yato::config& c, std::ostream& os, uint32_t indent)
        {
            YAML::Emitter emitter(os);
            emitter.SetIndent(indent);
            emitter << to_yaml_(c);
        }


    } // namespace yaml



} // namespace conf

} // namespace yato


