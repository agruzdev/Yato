/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_JSON_OBJECT_H_
#define _YATO_CONFIG_JSON_OBJECT_H_

#include "../config.h"

namespace yato {

namespace conf {

    class json_array;
    struct json_object_impl;

    /**
     * Wrapper around json object
     */
    class json_object
        : public config_object
    {
    private:
        std::unique_ptr<json_object_impl> m_pimpl;

        std::vector<std::string> do_get_keys() const noexcept override;

        yato::optional<config_value> do_get_value(const std::string & key) const noexcept override;
        const config_object* do_get_object(const std::string & key) const noexcept override;
        const config_array*  do_get_array(const std::string & key) const noexcept override;
    
        void* do_get_underlying_type() noexcept override;

    public:
        json_object(std::unique_ptr<json_object_impl> && impl);
        ~json_object();
    
        json_object(const json_object&) = delete;
        json_object(json_object&&) noexcept;
    
        json_object& operator = (const json_object&) = delete;
        json_object& operator = (json_object&&) noexcept;

        void swap(json_object & other) noexcept;
    };





    class json_object_state;

    class json_config
        : public basic_config
    {
    private:
        std::unique_ptr<json_object_state> m_impl;

        bool do_is_object() const noexcept override;
        yato::any do_get_by_name(config_type type, const std::string & name) const noexcept override;

        bool do_is_array() const noexcept override;
        yato::any do_get_by_index(config_type type, size_t index) const noexcept override;

        size_t do_get_size() const noexcept override;

    public:
        json_config(std::unique_ptr<json_object_state> && impl);
        ~json_config() = default;
    };



    class json_factory
        : public config_factory
    {
    public:
        std::unique_ptr<basic_config> create(const std::string & json) const override;
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_JSON_OBJECT_H_
