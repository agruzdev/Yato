/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_JSON_CONFIG_H_
#define _YATO_CONFIG_JSON_CONFIG_H_

#include "../config.h"

namespace yato {

namespace conf {


    class json_config_state;

    class json_config
        : public basic_config
    {
    private:
        std::unique_ptr<json_config_state> m_impl;

        bool do_is_object() const noexcept override;
        details::value_variant do_get_by_name(const std::string & name, config_type type) const noexcept override;

        bool do_is_array() const noexcept override;
        details::value_variant do_get_by_index(size_t index, config_type type) const noexcept override;

        size_t do_get_size() const noexcept override;

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
        config_ptr parse(const char* json) const;

        /**
         * Parse Json from string
         */
        config_ptr parse(const std::string & json) const;
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_JSON_CONFIG_H_
