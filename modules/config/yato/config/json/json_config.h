/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_JSON_CONFIG_H_
#define _YATO_CONFIG_JSON_CONFIG_H_

#include "../config.h"

namespace yato {

namespace conf {


    class json_config_state;

    class json_config
        : public config_backend
    {
    private:
        std::unique_ptr<json_config_state> m_impl;

        bool is_object() const noexcept override;
        stored_variant get_by_name(const std::string & name, stored_type type) const noexcept override;

        bool is_array() const noexcept override;
        stored_variant get_by_index(size_t index, stored_type type) const noexcept override;

        size_t size() const noexcept override;
        std::vector<std::string> keys() const noexcept override;

    public:
        json_config(std::unique_ptr<json_config_state> && impl);
        ~json_config() = default;

        json_config(const json_config&) = delete;
        json_config(json_config&&) noexcept;

        json_config& operator=(const json_config&) = delete;
        json_config& operator=(json_config&&) noexcept;
    };


    /**
     * Builder for Json config
     */
    class json_builder
    {
    public:
        json_builder();
        ~json_builder();

        json_builder(const json_builder&) = delete;
        json_builder(json_builder&&) noexcept;

        json_builder& operator = (const json_builder&) = delete;
        json_builder& operator = (json_builder&&) noexcept;

        /**
         * Parse Json from string
         */
        config parse(const char* json) const;

        /**
         * Parse Json from string
         */
        config parse(const std::string & json) const;
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_JSON_CONFIG_H_
