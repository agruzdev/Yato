/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_CONFIG_H_
#define _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_CONFIG_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "yato/config/config.h"
#include "yato/variant_match.h"
#include "manual_value.h"

namespace yato {

namespace conf {


    /**
     * Implements object
     */
    using manual_object_t = std::map<std::string, std::unique_ptr<config_value>>;
    
    /**
     * Implements array
     */
    using manual_array_t  = std::vector<std::unique_ptr<config_value>>;


    class manual_config final
        : public config_backend
    {
    public:

        static
        std::unique_ptr<manual_config> copy_with_path(const yato::config& other, const conf::path& path, stored_variant value)
        {
            return copy_with_path_impl_(other, path.tokenize(), copy_value_(std::move(value)));
        }

        static
        std::unique_ptr<manual_config> copy_without_path(const yato::config& other, const conf::path& path)
        {
            return copy_without_path_impl_(other, path.tokenize());
        }

        static
        std::unique_ptr<manual_config> copy_only_path(const yato::config& other, const conf::path& path)
        {
            return copy_only_path_impl_(other, path.tokenize());
        }

        static
        std::unique_ptr<manual_config> copy_with_whitelist(const yato::config& other, std::vector<std::string> names)
        {
            return copy_with_filter_(other, std::move(names), true);
        }

        static
        std::unique_ptr<manual_config> copy_with_blacklist(const yato::config& other, std::vector<std::string> names)
        {
            return copy_with_filter_(other, std::move(names), false);
        }

        static
        std::unique_ptr<manual_config> merge(const config& lhs, const config& rhs, conf::priority p = conf::priority::left)
        {
            switch (p) {
                default:
                    YATO_ASSERT(false,  "Invalid priority value");
                    YATO_ATTR_FALLTHROUGH;
                case conf::priority::left:
                    return merge_impl_(lhs, rhs);
                case conf::priority::right:
                    return merge_impl_(rhs, lhs);
            }
        }

        explicit
        manual_config(details::object_tag_t)
            : m_data(in_place_type_t<manual_object_t>{})
        { }

        explicit
        manual_config(details::array_tag_t)
            : m_data(in_place_type_t<manual_array_t>{})
        { }

        explicit
        manual_config(manual_object_t&& obj) noexcept
            : m_data(std::move(obj))
        { }

        explicit
        manual_config(manual_array_t&& arr) noexcept
            : m_data(std::move(arr))
        { }

        ~manual_config() override = default;


        manual_config(const manual_config&) = delete;

        manual_config(manual_config&& other) noexcept = default;

        manual_config& operator=(const manual_config&) = delete;

        manual_config& operator=(manual_config&& other) noexcept = default;

        void put(std::string name, std::unique_ptr<config_value>&& value)
        {
            yato::variant_match(
                [&](manual_object_t & obj) {
                    obj.insert_or_assign(std::move(name), std::move(value));
                },
                [&](match_default_t) {
                    throw config_error("manual_config[put]: Config must be an object.");
                }
            )(m_data);
        }

        void add(std::unique_ptr<config_value>&& value)
        {
            yato::variant_match(
                [&](manual_array_t & arr) {
                    arr.push_back(std::move(value));
                },
                [&](match_default_t) {
                    throw config_error("manual_config[add]: Config must be an array.");
                }
             )(m_data);
        }

    private:
        size_t do_size() const noexcept override
        {
            return yato::variant_match(
                [](const manual_object_t & obj) {
                    return obj.size();
                },
                [](const manual_array_t & arr) {
                    return arr.size();
                }
            )(m_data);
        }

        bool do_is_object() const noexcept override
        {
            return m_data.is_type<manual_object_t>();
        }

        key_value_t do_find(size_t index) const noexcept override
        {
            key_value_t kv{};
            yato::variant_match(
                [&](const manual_object_t & obj) {
                    if(index < obj.size()) {
                        const auto it = std::next(obj.cbegin(), index);
                        kv.first  = (*it).first;
                        kv.second = (*it).second.get();
                    }
                },
                [&](const manual_array_t & arr) {
                    if(index < arr.size()) {
                        kv.second = arr[index].get();
                    }
                }
            )(m_data);
            return kv;
        }

        key_value_t do_find(const std::string & name) const noexcept override
        {
            key_value_t kv{};
            yato::variant_match(
                [&](const manual_object_t & obj) {
                    const auto it = obj.find(name);
                    if(it != obj.cend()) {
                        kv.first  = (*it).first;
                        kv.second = (*it).second.get();
                    }
                },
                [&](match_default_t) {
                }
            )(m_data);
            return kv;
        }

        void do_release(const config_value* /*val*/) const noexcept override
        {
            return;
        }

        static
        std::unique_ptr<config_value> copy_value_(stored_variant val)
        {
            return yato::variant_match(
                    [](typename stored_type_trait<stored_type::boolean>::return_type&& v) {
                        return static_cast<std::unique_ptr<config_value>>(std::make_unique<manual_value<stored_type::boolean>>(v));
                    },
                    [](typename stored_type_trait<stored_type::integer>::return_type&& v) {
                        return static_cast<std::unique_ptr<config_value>>(std::make_unique<manual_value<stored_type::integer>>(v));
                    },
                    [](typename stored_type_trait<stored_type::real>::return_type&& v) {
                        return static_cast<std::unique_ptr<config_value>>(std::make_unique<manual_value<stored_type::real>>(v));
                    },
                    [](typename stored_type_trait<stored_type::string>::return_type&& v) {
                        return static_cast<std::unique_ptr<config_value>>(std::make_unique<manual_value<stored_type::string>>(std::move(v)));
                    },
                    [](typename stored_type_trait<stored_type::config>::return_type&& v) {
                        return static_cast<std::unique_ptr<config_value>>(std::make_unique<manual_value<stored_type::config>>(std::move(v)));
                    },
                    [](match_default_t) {
                        return nullptr;
                    }
            )(std::move(val));
        }

        static
        std::unique_ptr<config_value> wrap_config_(std::unique_ptr<manual_config>&& conf)
        {
            return std::make_unique<manual_value<stored_type::config>>(std::shared_ptr<config_backend>(std::move(conf)));
        }

        static
        std::unique_ptr<manual_config> merge_impl_(const yato::config& lhs_conf, const yato::config& rhs_conf)
        {
            if ((lhs_conf.is_null() || lhs_conf.is_object()) && (rhs_conf.is_null() || rhs_conf.is_object())) {
                auto joint_config = std::make_unique<manual_config>(details::object_tag_t{});
                std::vector<std::string> lhs_keys = lhs_conf.keys();
                std::vector<std::string> rhs_keys = rhs_conf.keys();
                std::sort(lhs_keys.begin(), lhs_keys.end());
                std::sort(rhs_keys.begin(), rhs_keys.end());
                std::vector<std::string> keys_intersection;
                keys_intersection.reserve(std::max(lhs_keys.size(), rhs_keys.size()));
                std::set_intersection(lhs_keys.cbegin(), lhs_keys.cend(), rhs_keys.cbegin(), rhs_keys.cend(), std::back_inserter(keys_intersection));

                const auto is_common_key = [&keys_intersection](const std::string& key) {
                    const auto it = std::lower_bound(keys_intersection.cbegin(), keys_intersection.cend(), key);
                    return (it != keys_intersection.cend()) ? (key == *it) : false;
                };

                const auto process_common_key = [&lhs_conf, &rhs_conf, &joint_config](const std::string& key) {
                    const auto lhs_entry = lhs_conf.find(key);
                    if (!lhs_entry) {
                        throw yato::config_error("merge: Failed to fetch a lhs value: " + key);
                    }
                    const auto rhs_entry = rhs_conf.find(key);
                    if (!rhs_entry) {
                        throw yato::config_error("merge: Failed to fetch a rhs value: " + key);
                    }

                    bool subconf_merged = false;
                    if (lhs_entry.type() == stored_type::config && rhs_entry.type() == stored_type::config) {
                        const yato::config lhs_subconf = lhs_entry.value<config>().get();
                        const yato::config rhs_subconf = rhs_entry.value<config>().get();
                        if (lhs_subconf.is_object() && rhs_subconf.is_object()) {
                            joint_config->put(key, wrap_config_(merge_impl_(lhs_subconf, rhs_subconf)));
                            subconf_merged = true;
                        }
                    }
                    if (!subconf_merged) {
                        auto value_copy = copy_value_(lhs_entry.value_handle_()->get());
                        if (value_copy) {
                            joint_config->put(key, std::move(value_copy));
                        } else {
                            throw yato::config_error("merge: Failed to copy a common value: " + key);
                        }
                    }
                };

                for (const std::string& name : lhs_keys) {
                    if (!is_common_key(name)) {
                        const auto entry = lhs_conf.find(name);
                        if (entry) {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                joint_config->put(name, std::move(value_copy));
                            } else {
                                throw yato::config_error("merge: Failed to copy a lhs value: " + entry.key());
                            }
                        }
                    }
                    else {
                        process_common_key(name);
                    }
                }

                for (const std::string& name : rhs_keys) {
                    if (!is_common_key(name)) {
                        const auto entry = rhs_conf.find(name);
                        if (entry) {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                joint_config->put(name, std::move(value_copy));
                            } else {
                                throw yato::config_error("merge: Failed to copy a rhs value: " + entry.key());
                            }
                        }
                    }
                    else {
                        process_common_key(name);
                    }
                }

                return joint_config;
            }
            else {
                throw yato::config_error("merge: Only objects can be merged.");
            }
        }

        template <typename Tokenizer_>
        static
        std::unique_ptr<manual_config> copy_with_path_impl_(const yato::config& other, Tokenizer_ path_tokenizer, std::unique_ptr<config_value>&& value)
        {
            const auto t = path_tokenizer.next();
            const std::string name{ t.begin(), t.end() };
            if (other.is_null() || other.is_object()) {
                auto config_copy = std::make_unique<manual_config>(details::object_tag_t{});
                bool key_replaced = false;
                for (const auto& entry : other) {
                    if (entry) {
                        if (entry.key() == name) {
                            if (path_tokenizer.has_next()) {
                                config_copy->put(name, wrap_config_(copy_with_path_impl_(entry.object(), path_tokenizer, std::move(value))));
                            }
                            else {
                                config_copy->put(name, std::move(value));
                            }
                            key_replaced = true;
                        } else {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                config_copy->put(entry.key(), std::move(value_copy));
                            } else {
                                throw yato::config_error("copy_with_path: Failed to copy a value: " + entry.key());
                            }
                        }
                    }
                }
                if (!key_replaced) {
                    if (path_tokenizer.has_next()) {
                        config_copy->put(name, wrap_config_(copy_with_path_impl_({}, path_tokenizer, std::move(value))));
                    }
                    else {
                        config_copy->put(name, std::move(value));
                    }
                }
                return config_copy;
            }
            else {
                throw yato::config_error("copy_with_path: Path can't be added to array.");
            }
        }

        template <typename Tokenizer_>
        static
        std::unique_ptr<manual_config> copy_only_path_impl_(const yato::config& other, Tokenizer_ path_tokenizer)
        {
            const auto t = path_tokenizer.next();
            const std::string name{ t.begin(), t.end() };
            if (other.is_null() || other.is_object()) {
                std::unique_ptr<manual_config> config_copy = nullptr;
                for (const auto& entry : other) {
                    if (entry) {
                        if (entry.key() == name) {
                            if (path_tokenizer.has_next()) {
                                auto child_config = copy_only_path_impl_(entry.object(), path_tokenizer);
                                if (child_config != nullptr) {
                                    config_copy = std::make_unique<manual_config>(details::object_tag_t{});
                                    config_copy->put(name, wrap_config_(std::move(child_config)));
                                }
                            }
                            else {
                                auto value_copy = copy_value_(entry.value_handle_()->get());
                                if (value_copy) {
                                    config_copy = std::make_unique<manual_config>(details::object_tag_t{});
                                    config_copy->put(entry.key(), std::move(value_copy));
                                } else {
                                    throw yato::config_error("copy_only_path: Failed to copy a value: " + entry.key());
                                }
                            }
                            break;
                        }
                    }
                }
                return config_copy;
            }
            else {
                throw yato::config_error("copy_only_path: Path can't be copied from array.");
            }
        }

        template <typename Tokenizer_>
        static
        std::unique_ptr<manual_config> copy_without_path_impl_(const yato::config& other, Tokenizer_ path_tokenizer)
        {
            const auto t = path_tokenizer.next();
            const std::string name{ t.begin(), t.end() };
            if (other.is_null()) {
                return nullptr;
            }
            if (other.is_object()) {
                auto config_copy = std::make_unique<manual_config>(details::object_tag_t{});
                for (const auto& entry : other) {
                    if (entry) {
                        if (entry.key() == name) {
                            if (path_tokenizer.has_next()) {
                                config_copy->put(name, wrap_config_(copy_without_path_impl_(entry.object(), path_tokenizer)));
                            }
                        } else {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                config_copy->put(entry.key(), std::move(value_copy));
                            } else {
                                throw yato::config_error("copy_without_path: Failed to copy a value: " + entry.key());
                            }
                        }
                    }
                }
                return config_copy;
            }
            else {
                throw yato::config_error("copy_without_path: Path can't be removed from array.");
            }
        }

        static
        std::unique_ptr<manual_config> copy_with_filter_(const yato::config& conf, std::vector<std::string> names, bool is_whitelist)
        {
            if (conf.is_null()) {
                return nullptr;
            }
            else if (conf.is_object()) {
                std::set<std::string> names_set(std::make_move_iterator(names.begin()), std::make_move_iterator(names.end()));
                auto config_copy = std::make_unique<manual_config>(details::object_tag_t{});
                for (const auto& entry : conf) {
                    if (entry) {
                        const bool name_matched = (names_set.find(entry.key()) != names_set.cend());
                        if (is_whitelist == name_matched) {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                config_copy->put(entry.key(), std::move(value_copy));
                            } else {
                                throw yato::config_error("copy_with_filter: Failed to copy a value: " + entry.key());
                            }
                        }
                    }
                }
                return config_copy;
            }
            else {
                throw yato::config_error("copy_with_filter: Only object-like config is supported.");
            }
        }

    private:
        yato::variant<manual_object_t, manual_array_t> m_data;
    };



} // namespace yato

} // namespace conf

#endif // _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_CONFIG_H_
