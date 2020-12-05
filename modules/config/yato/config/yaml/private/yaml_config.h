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

        ~yaml_config();

        yaml_config& operator=(const yaml_config&)= delete;

        yaml_config& operator=(yaml_config&&) = delete;

        size_t do_size() const noexcept override;

        bool do_has_property(config_property p) const noexcept override;

        find_index_result_t do_find(size_t index) const override;

        find_key_result_t do_find(const std::string& name) const override;

        void do_release(const config_value* val) const noexcept override;

        std::vector<std::string> do_enumerate_keys() const override;

    private:
        YAML::Node m_node;
    };

} // namespace conf

} // namespace yato


#endif //_YATO_CONFIG_YAML_PRIVATE_YAML_CONFIG_H_
