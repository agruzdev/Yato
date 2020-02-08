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

    yaml_builder::yaml_builder() = default;

    yaml_builder::~yaml_builder() = default;

    yaml_builder::yaml_builder(yaml_builder&&) noexcept = default;

    yaml_builder& yaml_builder::operator=(yaml_builder&&) noexcept = default;

    template <typename InputTy_>
    config parse_yaml_(InputTy_ && input)
    {
        backend_ptr backend = nullptr;
        auto root = YAML::Load(std::forward<InputTy_>(input));
        if (root.IsDefined() && (root.Type() == YAML::NodeType::Map || root.Type() == YAML::NodeType::Sequence)) {
            backend = std::make_shared<yaml_config>(std::move(root));
        }
        return config{ backend };
    }

    config yaml_builder::parse(const char* input) const
    {
        return parse_yaml_(input);
    }

    config yaml_builder::parse(const std::string & input) const
    {
        return parse_yaml_(input);
    }

    config yaml_builder::parse(std::istream & input) const
    {
        return parse_yaml_(input);
    }

} // namespace conf

} // namespace yato


