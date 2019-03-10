/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_YAML_CONFIG_H_
#define _YATO_CONFIG_YAML_CONFIG_H_

#include "../config.h"

namespace yato {

namespace conf {


    class yaml_config_state;

    class yaml_config
        : public config_backend
    {
    private:
        std::unique_ptr<yaml_config_state> m_impl;

        bool is_object() const noexcept override;
        stored_variant get_by_name(const std::string & name, stored_type type) const noexcept override;

        bool is_array() const noexcept override;
        stored_variant get_by_index(size_t index, stored_type type) const noexcept override;

        size_t size() const noexcept override;
        std::vector<std::string> keys() const noexcept override;

    public:
        yaml_config(std::unique_ptr<yaml_config_state> && impl);
        ~yaml_config() = default;

        yaml_config(const yaml_config&) = delete;
        yaml_config(yaml_config&&) noexcept;

        yaml_config& operator=(const yaml_config&) = delete;
        yaml_config& operator=(yaml_config&&) noexcept;
    };


    /**
     * Builder for Json config
     */
    class yaml_builder
    {
    public:
        yaml_builder();
        ~yaml_builder();

        yaml_builder(const yaml_builder&) = delete;
        yaml_builder(yaml_builder&&) noexcept;

        yaml_builder& operator = (const yaml_builder&) = delete;
        yaml_builder& operator = (yaml_builder&&) noexcept;

        /**
         * Parse YAML from string
         */
        config parse(const char* yaml) const;

        /**
         * Parse YAML from string
         */
        config parse(const std::string & yaml) const;
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_YAML_CONFIG_H_
