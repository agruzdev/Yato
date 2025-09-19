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

    namespace {

        template <typename NJson_, typename... Args_>
        config read_impl2_(Args_&&... args)
        {
            backend_ptr_t backend = nullptr;
            auto json = std::make_shared<NJson_>(NJson_::parse(std::forward<Args_>(args)..., nullptr, false));
            if (!json->is_discarded()) {
                backend = std::make_shared<json_config<NJson_>>(std::move(json));
            }
            return config(backend);
        }

        template <typename... Args_>
        config read_impl_(bool ordered, Args_&&... args)
        {
            if (ordered) {
                return read_impl2_<nlohmann::ordered_json>(std::forward<Args_>(args)...);
            }
            else {
                return read_impl2_<nlohmann::json>(std::forward<Args_>(args)...);
            }
        }

    } // namespace

    config read(const char* str, size_t len, bool ordered)
    {
        return (len != yato::nolength) ? read_impl_(ordered, str, str + len) : read_impl_(ordered, str);
    }

    config read(const std::string& str, bool ordered)
    {
        return read_impl_(ordered, str);
    }

    config read(std::istream& is, bool ordered)
    {
        return read_impl_(ordered, is);
    }

    namespace
    {

        template <typename NJson_, typename Ty_>
        void put(NJson_& js, const std::string& key, Ty_&& val) {
            if (js.is_object()) {
                js.emplace(key, std::forward<Ty_>(val));
            }
            else {
                js.emplace_back(std::forward<Ty_>(val));
            }
        }

        template <typename NJson_>
        NJson_ to_json_(const yato::config& c)
        {
            if (c.is_null()) {
                return static_cast<NJson_>(nullptr);
            }
            NJson_ js = c.is_associative() ? NJson_::object() : NJson_::array();
            for (auto entry : c) {
                if (entry) {
                    switch (entry.type()) {
                    case stored_type::integer:
                        put(js, entry.key(), entry.value<typename NJson_::number_integer_t>().get());
                        break;
                    case stored_type::real:
                        put(js, entry.key(), entry.value<typename NJson_::number_float_t>().get());
                        break;
                    case stored_type::boolean:
                        put(js, entry.key(), entry.value<typename NJson_::boolean_t>().get());
                        break;
                    case stored_type::string:
                        put(js, entry.key(), entry.value<typename NJson_::string_t>().get());
                        break;
                    case stored_type::config:
                        put(js, entry.key(), to_json_<NJson_>(entry.object()));
                        break;
                    }
                }
                else {
                    put(js, entry.key(), nullptr);
                }
            }
            return js;
        }

    } // namespace 

    std::string write(const yato::config& c, uint32_t indent, yato::optional<bool> ordered)
    {
        if (ordered.get_or(c.is_associative() && c.is_ordered())) {
            return to_json_<nlohmann::ordered_json>(c).dump(indent);
        }
        else {
            return to_json_<nlohmann::json>(c).dump(indent);
        }
    }

    void write(const yato::config& c, std::ostream& os, uint32_t indent, yato::optional<bool> ordered)
    {
        const auto backup_width = os.width();
        os.width(indent);
        if (ordered.get_or(c.is_associative() && c.is_ordered())) {
            os << to_json_<nlohmann::ordered_json>(c);
        }
        else {
            os << to_json_<nlohmann::json>(c);
        }
        os.width(backup_width);
    }

} //namespace json

} //namespace conf

} //namespace yato


