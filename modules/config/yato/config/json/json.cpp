/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "json.h"
#include "private/json_config.h"

namespace yato {

namespace conf {

namespace json {

    template <typename... Args_>
    config read_impl_(Args_&&... args)
    {
        backend_ptr_t backend = nullptr;
        auto json = std::make_shared<nlohmann::json>(nlohmann::json::parse(std::forward<Args_>(args)..., nullptr, false));
        if (!json->is_discarded()) {
            backend = std::make_shared<json_config>(std::move(json));
        }
        return config(backend);
    }

    config read(const char* str, size_t len)
    {
        return (len != yato::nolength) ? read_impl_(str, str + len) : read_impl_(str);
    }

    config read(const std::string& str)
    {
        return read_impl_(str);
    }

    config read(std::istream& is)
    {
        return read_impl_(is);
    }

    template <typename Ty_>
    void put(nlohmann::json& js, const std::string& key, Ty_&& val) {
        if (js.is_object()) {
            js.emplace(std::make_pair(key, std::forward<Ty_>(val)));
        }
        else {
            js.emplace_back(std::forward<Ty_>(val));
        }
    }

    static
    nlohmann::json to_json_(const yato::config& c)
    {
        if (c.is_null()) {
            return nlohmann::json(nullptr);
        }
        nlohmann::json js = c.is_associative() ? nlohmann::json::object() : nlohmann::json::array();
        for (auto entry : c) {
            if (entry) {
                switch (entry.type()) {
                case stored_type::integer:
                    put(js, entry.key(), entry.value<nlohmann::json::number_integer_t>().get());
                    break;
                case stored_type::real:
                    put(js, entry.key(), entry.value<nlohmann::json::number_float_t>().get());
                    break;
                case stored_type::boolean:
                    put(js, entry.key(), entry.value<nlohmann::json::boolean_t>().get());
                    break;
                case stored_type::string:
                    put(js, entry.key(), entry.value<nlohmann::json::string_t>().get());
                    break;
                case stored_type::config:
                    put(js, entry.key(), to_json_(entry.object()));
                    break;
                }
            }
            else {
                put(js, entry.key(), nullptr);
            }
        }
        return js;
    }

    std::string write(const yato::config& c, uint32_t indent)
    {
        return to_json_(c).dump(indent);
    }

    void write(const yato::config& c, std::ostream& os, uint32_t indent)
    {
        const auto backup_width = os.width();
        os.width(indent);
        os << to_json_(c);
        os.width(backup_width);
    }

} //namespace json

} //namespace conf

} //namespace yato


