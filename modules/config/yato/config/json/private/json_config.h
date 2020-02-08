/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_JSON_PRIVATE_JSON_CONFIG_H_
#define _YATO_CONFIG_JSON_PRIVATE_JSON_CONFIG_H_


#include <nlohmann/json.hpp>

#include "../../config_backend.h"

namespace yato {

namespace conf {


    class json_config final
        : public config_backend
    {
    public:
        json_config(std::shared_ptr<nlohmann::json> json);
        json_config(std::shared_ptr<nlohmann::json> root, nlohmann::json::const_iterator iter);

        json_config(const json_config&) = delete;
        json_config(json_config&&) = delete;

        json_config& operator=(const json_config&) = delete;
        json_config& operator=(json_config&&) = delete;

        ~json_config();

        size_t do_size() const noexcept override;

        key_value_t do_find(size_t index) const noexcept override;

        void do_release(const config_value* val) const noexcept override;

        bool do_is_object() const noexcept override;

        key_value_t do_find(const std::string & name) const noexcept override;

    private:
        nlohmann::json::const_reference get_() const;

        std::shared_ptr<nlohmann::json> m_root;
        yato::optional<nlohmann::json::const_iterator> m_iter;
    };


} //namespace conf

} //namespace yato

#endif // _YATO_CONFIG_JSON_PRIVATE_JSON_CONFIG_H_
