/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#include <nlohmann/json.hpp>

#include "json_object.h"

namespace yato {

namespace conf {

    using element_type = yato::variant<void, config_value, /*manual_array, */ json_object>;

    struct json_object_impl {
        nlohmann::json object;
    };

    /**
     * Store read-only iterator
     * Root is shared between all configs
     */
    class json_object_state
    {
        std::shared_ptr<nlohmann::json> m_root;
        yato::optional<nlohmann::json::const_iterator> m_iter;

    public:
        json_object_state(nlohmann::json && json)
            : m_root(std::make_shared<nlohmann::json>(std::move(json)))
            , m_iter(yato::nullopt_t{})
        { }

        json_object_state(const json_object_state & parent, const nlohmann::json::const_iterator & iter)
            : m_root(parent.m_root)
            , m_iter(iter)
        { }

        const nlohmann::json & get() const
        {
            return static_cast<bool>(m_iter) ? *m_iter.get_unsafe() : *m_root;
        }
    };

    //template <typename Ty_>
    //const Ty_* get_field_impl_(const manual_object_impl & self, const std::string & key)
    //{
    //    const auto it = self.data.find(key);
    //    if (it == self.data.cend()) {
    //        return nullptr;
    //    }
    //    if(it->second.type() != typeid(Ty_)) {
    //        return nullptr;
    //    }
    //    return &it->second.get_as_unsafe<Ty_>();
    //}
    //
    //template <typename Ty_>
    //void put_impl_(manual_object_impl & self, const std::string & key, Ty_ && obj) 
    //{
    //    self.data[key] = static_cast<element_type>(std::forward<Ty_>(obj));
    //}

    std::vector<std::string> json_object::do_get_keys() const noexcept
    {
        //YATO_ASSERT(m_pimpl != nullptr, "null impl");
        //std::vector<std::string> keys;
        //keys.reserve(m_pimpl->data.size());
        //for(auto it : m_pimpl->data) {
        //    keys.push_back(it.first);
        //}
        //return keys;
        return {};
    }

    
    yato::optional<config_value> json_object::do_get_value(const std::string & key) const noexcept
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        const auto it = m_pimpl->object.find(key);
        //if(it != m_pimpl->object.end()) {
        //    if(it->is_primitive()) {
        //        return yato::make_optional(config_value(json.get()));
        //    }
        //}
        //return *get_field_impl_<config_value>(*m_pimpl, key);
        return yato::nullopt_t{};
    }

    const config_object* json_object::do_get_object(const std::string & key) const noexcept
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        //return get_field_impl_<manual_object>(*m_pimpl, key);
        return nullptr;
    }

    const config_array* json_object::do_get_array(const std::string & key) const noexcept
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        //return get_field_impl_<manual_array>(*m_pimpl, key);
        return nullptr;
    }

    void* json_object::do_get_underlying_type() noexcept
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        return &m_pimpl->object;
    }

    json_object::json_object(std::unique_ptr<json_object_impl> && impl)
        : m_pimpl(std::move(impl))
    { }

    json_object::~json_object() = default;

    json_object::json_object(json_object&&) noexcept = default;

    json_object& json_object::operator =(json_object&&) noexcept = default;

#if 0
    void json_object::put(const std::string & key, const config_value & val) 
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        //put_impl_(*m_pimpl, key, val);
    }

    void json_object::put(const std::string & key, config_value && val) 
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        //put_impl_(*m_pimpl, key, std::move(val));
    }

    void json_object::put(const std::string & key, const manual_array & val)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        //put_impl_(*m_pimpl, key, val);
    }

    void json_object::put(const std::string & key, manual_array && val)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        //put_impl_(*m_pimpl, key, std::move(val));
    }

    void json_object::put(const std::string & key, const manual_object & val)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        //put_impl_(*m_pimpl, key, val);
    }

    void manual_object::put(const std::string & key, manual_object && val)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, key, std::move(val));
    }
#endif

    void json_object::swap(json_object & other) noexcept
    {
        m_pimpl.swap(other.m_pimpl);
    }






    json_config::json_config(std::unique_ptr<json_object_state> && impl)
        : m_impl(std::move(impl))
    { }

    yato::any json_config::do_get_by_name(config_type type, const std::string & name) const noexcept
    {
        assert(m_impl != nullptr);
        yato::any res{};

        try {
            const auto & json = m_impl->get();
            const auto it = json.find(name);
            if(it != json.end()) {
                switch (type)
                {
                case yato::conf::config_type::integer:
                    if(it->is_number_integer()) {
                        using return_type = typename details::config_type_trait<config_type::integer>::return_type;
                        res = yato::narrow_cast<return_type>(it->get<nlohmann::json::number_integer_t>());
                    }
                    break;
                case yato::conf::config_type::boolean:
                    if(it->is_boolean()) {
                        using return_type = typename details::config_type_trait<config_type::boolean>::return_type;
                        res = static_cast<return_type>(it->get<nlohmann::json::boolean_t>());
                    }
                    break;
                case yato::conf::config_type::floating:
                    if(it->is_number_float()) {
                        using return_type = typename details::config_type_trait<config_type::floating>::return_type;
                        res = yato::narrow_cast<return_type>(it->get<nlohmann::json::number_float_t>());
                    }
                    break;
                case yato::conf::config_type::string:
                    if(it->is_string()) {
                        using return_type = typename details::config_type_trait<config_type::string>::return_type;
                        res = static_cast<return_type>(it->get<nlohmann::json::string_t>());
                    }
                    break;
                case yato::conf::config_type::config:
                    if(it->is_object()) {
                        using return_type = typename details::config_type_trait<config_type::config>::return_type;
                        auto impl = std::make_unique<json_object_state>(*m_impl, it);
                        return_type subconfig = std::make_unique<json_config>(std::move(impl));
                        res.emplace<return_type>(std::move(subconfig));
                    }
                    break;
                default:
                    break;
                }
            }
        }
        catch(...) {
            // ToDo (a.gruzdev): Report error somehow
            /* In case of error return empty result */
        }
        return res;
    }

    std::unique_ptr<json_config> json_factory::create(const std::string & json) const
    {
        auto impl = std::make_unique<json_object_state>(nlohmann::json::parse(json, nullptr, false));
        if(impl->get().is_discarded()) {
            return nullptr;
        }
        return std::make_unique<json_config>(std::move(impl));
    }

} // namespace conf

} // namespace yato
