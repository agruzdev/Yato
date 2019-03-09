/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include <map>
#include <vector>

#include "yato/variant_match.h"

#include "manual_config.h"

namespace yato {

namespace conf {

    /**
     * Each object/array stores either scalar value or object
     */
    using manual_value_type = yato::variant<void, details::manual_scalar, backend_ptr>;

    /**
     * Implements object
     */
    using manual_object_type = std::map<std::string, manual_value_type>;
    
    /**
     * Implements array
     */
    using manual_array_type  = std::vector<manual_value_type>;

    /**
     * Each config is either object or array
     */
    using manual_node  = yato::variant<manual_object_type, manual_array_type>;

    //--------------------------------------------------------------------
    // Manual config

    class manual_config_state
    {
    private:
        manual_node m_node;

        void put_impl_(const std::string & key, manual_value_type && val)
        {
            if(is_object()) {
                manual_object_type & obj = m_node.get_as_unsafe<manual_object_type>();
                obj[key] = std::move(val);
            }
            else {
                throw config_error("Is not key:value object");
            }
        }

        void append_impl_(manual_value_type && val)
        {
            if(is_array()) {
                manual_array_type & arr = m_node.get_as_unsafe<manual_array_type>();
                arr.push_back(std::move(val));
            }
            else {
                throw config_error("Is not key:value object");
            }
        }

        stored_variant get_impl_(const manual_value_type & value, config_type type)
        {
            stored_variant res{};
            switch (type) {
            case config_type::integer:
                if(value.is_type<details::manual_scalar>()) {
                    const auto & scalar = value.get_as_unsafe<details::manual_scalar>();
                    if(scalar.is_type<int64_t>()) {
                        using return_type = stored_type_trait<config_type::integer>::return_type;
                        res.emplace<return_type>(scalar.get_as_unsafe<int64_t>());
                    }
                }
                break;
            case config_type::floating:
                if(value.is_type<details::manual_scalar>()) {
                    const auto & scalar = value.get_as_unsafe<details::manual_scalar>();
                    if(scalar.is_type<double>()) {
                        using return_type = stored_type_trait<config_type::floating>::return_type;
                        res.emplace<return_type>(scalar.get_as_unsafe<double>());
                    }
                }
                break;
            case config_type::boolean:
                if(value.is_type<details::manual_scalar>()) {
                    const auto & scalar = value.get_as_unsafe<details::manual_scalar>();
                    if(scalar.is_type<bool>()) {
                        using return_type = stored_type_trait<config_type::boolean>::return_type;
                        res.emplace<return_type>(scalar.get_as_unsafe<bool>());
                    }
                }
                break;
            case config_type::string:
                if(value.is_type<details::manual_scalar>()) {
                    const auto & scalar = value.get_as_unsafe<details::manual_scalar>();
                    if(scalar.is_type<std::string>()) {
                        using return_type = stored_type_trait<config_type::string>::return_type;
                        res.emplace<return_type>(scalar.get_as_unsafe<std::string>());
                    }
                }
                break;
            case config_type::config:
                if(value.is_type<backend_ptr>()) {
                    using return_type = stored_type_trait<config_type::config>::return_type;
                    res.emplace<return_type>(value.get_as_unsafe<backend_ptr>());
                }
                break;
            }
            return res;
        }

    public:
        manual_config_state(manual_node && node)
            : m_node(std::move(node))
        { }

        manual_config_state(details::object_tag_t)
            : m_node(yato::in_place_type_t<manual_object_type>{})
        { }

        manual_config_state(details::array_tag_t)
            : m_node(yato::in_place_type_t<manual_array_type>{})
        { }

        bool is_object() const
        {
            return m_node.is_type<manual_object_type>();
        }

        bool is_array() const
        {
            return m_node.is_type<manual_array_type>();
        }

        size_t size() const
        {
            if(!is_array()) {
                return 0;
            }
            return m_node.get_as_unsafe<manual_array_type>().size();
        }

        std::vector<std::string> keys() const
        {
            std::vector<std::string> res;
            yato::variant_match(
                [&res](const manual_object_type & obj) {
                    res.reserve(obj.size());
                    for (const auto & entry : obj) {
                        res.push_back(entry.first);
                    }
                },
                [](yato::match_default_t) {}
            )(m_node);
            return res;
        }

        void put(const std::string & key, details::manual_scalar && val)
        {
            put_impl_(key, manual_value_type(std::move(val)));
        }

        void put(const std::string & key, backend_ptr && val)
        {
            put_impl_(key, manual_value_type(std::move(val)));
        }

        void append(details::manual_scalar && val)
        {
            append_impl_(manual_value_type(std::move(val)));
        }

        void append(backend_ptr && val)
        {
            append_impl_(manual_value_type(std::move(val)));
        }

        stored_variant get(const std::string & key, config_type type)
        {
            if(!is_object()) {
                return yato::nullvar_t{};
            }

            const manual_object_type & object = m_node.get_as_unsafe<manual_object_type>();
            const auto it = object.find(key);
            if(it == object.cend()) {
                // No such key
                return yato::nullvar_t{};
            }

            const manual_value_type & value = (*it).second;
            return get_impl_(value, type);
        }

        stored_variant get(size_t idx, config_type type)
        {
            if(!is_array()) {
                return yato::nullvar_t{};
            }

            const manual_array_type & arr = m_node.get_as_unsafe<manual_array_type>();
            if(idx >= arr.size()) {
                return yato::nullvar_t{};
            }

            return get_impl_(arr[idx], type);
        }
    };

    manual_config::manual_config(std::unique_ptr<manual_config_state> && impl)
        : m_impl(std::move(impl))
    {
        YATO_ENSURES(m_impl != nullptr);
    }

    manual_config::~manual_config() = default;

    manual_config::manual_config(manual_config&&) noexcept = default;

    manual_config& manual_config::operator=(manual_config&&) noexcept = default;

    bool manual_config::is_object() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        return m_impl->is_object();
    }

    std::vector<std::string> manual_config::keys() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        return m_impl->keys();
    }

    stored_variant manual_config::get_by_name(const std::string & name, config_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        return m_impl->get(name, type);
    }

    bool manual_config::is_array() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        return m_impl->is_array();
    }

    stored_variant manual_config::get_by_index(size_t index, config_type type) const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        return m_impl->get(index, type);
    }

    size_t manual_config::size() const noexcept
    {
        YATO_REQUIRES(m_impl != nullptr);
        return m_impl->size();
    }



    //---------------------------------------------------------------------------------
    // Manual config builder

    manual_builder::~manual_builder() = default;

    manual_builder::manual_builder(details::object_tag_t)
    {
        m_impl = std::make_unique<manual_config_state>(details::object_tag_t{});
    }

    manual_builder::manual_builder(details::array_tag_t)
    {
        m_impl = std::make_unique<manual_config_state>(details::array_tag_t{});
    }

    manual_builder::manual_builder(manual_builder &&) noexcept = default;

    manual_builder& manual_builder::operator=(manual_builder&&) noexcept = default;

    config manual_builder::create() noexcept
    {
        backend_ptr backend = nullptr;
        if(m_impl != nullptr) {
            backend = std::make_unique<manual_config>(std::move(m_impl));
            m_impl.reset();
        }
        return config(backend);
    }

    void manual_builder::put_scalar_(const std::string & key, details::manual_scalar && scalar)
    {
        if(m_impl == nullptr) {
            throw config_error("manual_builder is empty after creating config.");
        }
        m_impl->put(key, std::move(scalar));
    }

    void manual_builder::put_object_(const std::string & key, backend_ptr && conf)
    {
        if(m_impl == nullptr) {
            throw config_error("manual_builder is empty after creating config.");
        }
        m_impl->put(key, std::move(conf));
    }

    void manual_builder::append_scalar_(details::manual_scalar && scalar)
    {
        if(m_impl == nullptr) {
            throw config_error("manual_builder is empty after creating config.");
        }
        m_impl->append(std::move(scalar));
    }

    void manual_builder::append_object_(backend_ptr && conf)
    {
        if(m_impl == nullptr) {
            throw config_error("manual_builder is empty after creating config.");
        }
        m_impl->append(std::move(conf));
    }

} // namespace conf

} // namespace yato
