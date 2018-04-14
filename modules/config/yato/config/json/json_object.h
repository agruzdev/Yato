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


    class json_object_state;

    class json_config
        : public basic_config
    {
    private:
        std::unique_ptr<json_object_state> m_impl;

        bool do_is_object() const noexcept override;
        details::value_variant do_get_by_name(const std::string & name, config_type type) const noexcept override;

        bool do_is_array() const noexcept override;
        details::value_variant do_get_by_index(size_t index, config_type type) const noexcept override;

        size_t do_get_size() const noexcept override;

    public:
        json_config(std::unique_ptr<json_object_state> && impl);
        ~json_config() = default;
    };



    class json_factory
        : public config_factory
    {
    public:
        config_ptr create(const std::string & json) const override;
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_JSON_OBJECT_H_
