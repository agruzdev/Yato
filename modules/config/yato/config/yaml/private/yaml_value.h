/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_YAML_PRIVATE_YAML_VALUE_H_
#define _YATO_CONFIG_YAML_PRIVATE_YAML_VALUE_H_

#include "yaml_lib.h"
#include "../../config_backend.h"

namespace yato {

namespace conf {

    class yaml_value final
        : public config_value
    {
    public:
        yaml_value(YAML::Node node);

        yaml_value(const yaml_value&)= delete;
        yaml_value(yaml_value&&) = delete;

        yaml_value& operator=(const yaml_value&)= delete;
        yaml_value& operator=(yaml_value&&) = delete;

        ~yaml_value();

        stored_type type() const noexcept override;

        stored_variant get() const noexcept override;

    private:
        YAML::Node m_node;
    };

} // namespace conf

} // namespace yato


#endif //_YATO_CONFIG_YAML_PRIVATE_YAML_VALUE_H_
