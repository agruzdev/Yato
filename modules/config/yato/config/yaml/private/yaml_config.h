/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_YAML_PRIVATE_YAML_CONFIG_H_
#define _YATO_CONFIG_YAML_PRIVATE_YAML_CONFIG_H_

#include "yaml_lib.h"
#include "../../config_backend.h"

namespace yato {

namespace conf {

    class yaml_config final
        : public config_backend
    {
    public:
        yaml_config(YAML::Node node);

        yaml_config(const yaml_config&)= delete;
        yaml_config(yaml_config&&) = delete;

        yaml_config& operator=(const yaml_config&)= delete;
        yaml_config& operator=(yaml_config&&) = delete;

        ~yaml_config();

        size_t do_size() const noexcept override;

        bool do_is_object() const noexcept override;

        key_value_t do_find(size_t index) const noexcept override;

        key_value_t do_find(const std::string & name) const noexcept override;

        void do_release(const config_value* val) const noexcept override;

        std::vector<std::string> do_keys() const noexcept override;

    private:
        YAML::Node m_node;
    };

} // namespace conf

} // namespace yato


#endif //_YATO_CONFIG_YAML_PRIVATE_YAML_CONFIG_H_
