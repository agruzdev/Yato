/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "manual_config_base.h"
#include "manual_map.h"
#include "manual_ordered_map.h"
#include "manual_multimap.h"
#include "manual_array.h"

namespace yato {

namespace conf {

    namespace
    {

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

        std::unique_ptr<config_value> wrap_config_(std::shared_ptr<manual_config_base>&& conf)
        {
            YATO_REQUIRES(conf.use_count() == 1);
            return std::make_unique<manual_value<stored_type::config>>(std::shared_ptr<config_backend>(std::move(conf)));
        }

        std::shared_ptr<manual_config_base> create_empty_copy_(const yato::config& c)
        {
            if (c.is_null()) {
                return std::make_shared<manual_map>();
            }
            if (c.is_multi_associative()) {
                return std::make_shared<manual_multimap>();
            }
            if (c.is_associative()) {
                if (c.is_ordered()) {
                    return std::make_shared<manual_ordered_map>();
                }
                else {
                    return std::make_shared<manual_map>();
                }
            }
            return std::make_shared<manual_array>();
        }

        std::shared_ptr<manual_config_base> copy_impl_(const yato::config& c, bool deep_copy)
        {
            if (!c) {
                return nullptr;
            }

            std::shared_ptr<manual_config_base> config_copy = create_empty_copy_(c);

            for (auto entry : c) {
                std::unique_ptr<config_value> value_copy;
                if (entry) {
                    if (deep_copy && entry.type() == stored_type::config) {
                        value_copy = wrap_config_(copy_impl_(entry.object(), true));
                    }
                    else {
                        value_copy = copy_value_(entry.value_handle_()->get());
                    }
                    if (!value_copy) {
                        throw yato::config_error("deep_copy_impl_: Failed to copy a value");
                    }
                }

                if (c.is_associative()) {
                    config_copy->put(entry.key(), std::move(value_copy));
                }
                else {
                    config_copy->add(std::move(value_copy));
                }
            }

            return config_copy;
        }

        std::shared_ptr<manual_config_base> merge_impl_(const yato::config& lhs_conf, const yato::config& rhs_conf)
        {
            if ((lhs_conf.is_null() || lhs_conf.is_associative()) && (rhs_conf.is_null() || rhs_conf.is_associative())) {

                auto joint_config = (lhs_conf.is_multi_associative() || rhs_conf.is_multi_associative())
                    ? std::static_pointer_cast<manual_config_base>(std::make_shared<manual_multimap>())
                    : std::static_pointer_cast<manual_config_base>(std::make_shared<manual_map>());

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
                    const auto lhs_entry = lhs_conf.at(key);
                    if (!lhs_entry) {
                        throw yato::config_error("merge: Failed to fetch a lhs value: " + key);
                    }
                    const auto rhs_entry = rhs_conf.at(key);
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
                        }
                        else {
                            throw yato::config_error("merge: Failed to copy a common value: " + key);
                        }
                    }
                };

                for (const std::string& name : lhs_keys) {
                    if (!is_common_key(name)) {
                        const auto entry = lhs_conf.at(name);
                        if (entry) {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                joint_config->put(name, std::move(value_copy));
                            }
                            else {
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
                        const auto entry = rhs_conf.at(name);
                        if (entry) {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                joint_config->put(name, std::move(value_copy));
                            }
                            else {
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
                throw yato::config_error("merge: Only associative objects can be merged.");
            }
        }

        template <typename Tokenizer_>
        std::shared_ptr<manual_config_base> copy_with_path_impl_(const yato::config& other, Tokenizer_ path_tokenizer, std::unique_ptr<config_value>&& value)
        {
            const auto t = path_tokenizer.next();
            const std::string name{ t.begin(), t.end() };
            if (other.is_null() || other.is_associative()) {
                auto config_copy = create_empty_copy_(other);
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
                        }
                        else {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                config_copy->put(entry.key(), std::move(value_copy));
                            }
                            else {
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
            throw yato::config_error("copy_with_path: Path can't be added to array.");
        }

        template <typename Tokenizer_>
        std::shared_ptr<manual_config_base> copy_only_path_impl_(const yato::config& other, Tokenizer_ path_tokenizer)
        {
            const auto t = path_tokenizer.next();
            const std::string name{ t.begin(), t.end() };
            if (other.is_null()) {
                return nullptr;
            }
            if (other.is_associative()) {
                std::shared_ptr<manual_config_base> config_copy = nullptr;
                for (const auto& entry : other) {
                    if (entry) {
                        if (entry.key() == name) {
                            if (path_tokenizer.has_next()) {
                                auto child_config = copy_only_path_impl_(entry.object(), path_tokenizer);
                                if (child_config != nullptr) {
                                    config_copy = create_empty_copy_(other);
                                    config_copy->put(name, wrap_config_(std::move(child_config)));
                                }
                            }
                            else {
                                auto value_copy = copy_value_(entry.value_handle_()->get());
                                if (value_copy) {
                                    config_copy = create_empty_copy_(other);
                                    config_copy->put(entry.key(), std::move(value_copy));
                                }
                                else {
                                    throw yato::config_error("copy_only_path: Failed to copy a value: " + entry.key());
                                }
                            }
                            break;
                        }
                    }
                }
                return config_copy;
            }
            throw yato::config_error("copy_only_path: Path can't be copied from array.");
        }

        template <typename Tokenizer_>
        std::shared_ptr<manual_config_base> copy_without_path_impl_(const yato::config& other, Tokenizer_ path_tokenizer)
        {
            const auto t = path_tokenizer.next();
            const std::string name{ t.begin(), t.end() };
            if (other.is_null()) {
                return nullptr;
            }
            if (other.is_associative()) {
                auto config_copy = create_empty_copy_(other);
                for (const auto& entry : other) {
                    if (entry) {
                        if (entry.key() == name) {
                            if (path_tokenizer.has_next()) {
                                config_copy->put(name, wrap_config_(copy_without_path_impl_(entry.object(), path_tokenizer)));
                            }
                        }
                        else {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                config_copy->put(entry.key(), std::move(value_copy));
                            }
                            else {
                                throw yato::config_error("copy_without_path: Failed to copy a value: " + entry.key());
                            }
                        }
                    }
                }
                return config_copy;
            }
            throw yato::config_error("copy_without_path: Path can't be removed from array.");
        }

        std::shared_ptr<manual_config_base> copy_with_filter_(const yato::config& conf, std::vector<std::string> names, bool is_whitelist)
        {
            if (conf.is_null()) {
                return nullptr;
            }
            if (conf.is_associative()) {
                std::set<std::string> names_set(std::make_move_iterator(names.begin()), std::make_move_iterator(names.end()));
                auto config_copy = create_empty_copy_(conf);
                for (const auto& entry : conf) {
                    if (entry) {
                        const bool name_matched = (names_set.find(entry.key()) != names_set.cend());
                        if (is_whitelist == name_matched) {
                            auto value_copy = copy_value_(entry.value_handle_()->get());
                            if (value_copy) {
                                config_copy->put(entry.key(), std::move(value_copy));
                            }
                            else {
                                throw yato::config_error("copy_with_filter: Failed to copy a value: " + entry.key());
                            }
                        }
                    }
                }
                return config_copy;
            }
            throw yato::config_error("copy_with_filter: Only object-like config is supported.");
        }

    } // namespace


    std::shared_ptr<manual_config_base> manual_config_base::copy(const yato::config& c, bool deep_copy)
    {
        return copy_impl_(c, deep_copy);
    }

    std::shared_ptr<manual_config_base> manual_config_base::deep_copy(const yato::config& c)
    {
        return copy_impl_(c, true);
    }

    std::shared_ptr<manual_config_base> manual_config_base::shallow_copy(const yato::config& c)
    {
        return copy_impl_(c, false);
    }

    std::shared_ptr<manual_config_base> manual_config_base::copy_with_path(const yato::config& other, const conf::path& path, stored_variant value)
    {
        return copy_with_path_impl_(other, path.tokenize(), copy_value_(std::move(value)));
    }

    std::shared_ptr<manual_config_base> manual_config_base::copy_without_path(const yato::config& other, const conf::path& path)
    {
        return copy_without_path_impl_(other, path.tokenize());
    }

    std::shared_ptr<manual_config_base> manual_config_base::copy_only_path(const yato::config& other, const conf::path& path)
    {
        return copy_only_path_impl_(other, path.tokenize());
    }

    std::shared_ptr<manual_config_base> manual_config_base::copy_with_whitelist(const yato::config& other, std::vector<std::string> names)
    {
        return copy_with_filter_(other, std::move(names), true);
    }

    std::shared_ptr<manual_config_base> manual_config_base::copy_with_blacklist(const yato::config& other, std::vector<std::string> names)
    {
        return copy_with_filter_(other, std::move(names), false);
    }

    std::shared_ptr<manual_config_base> manual_config_base::merge(const config& lhs, const config& rhs, conf::priority p)
    {
        switch (p) {
        default:
            YATO_ASSERT(false, "Invalid priority value");
            YATO_ATTR_FALLTHROUGH;
        case conf::priority::left:
            return merge_impl_(lhs, rhs);
        case conf::priority::right:
            return merge_impl_(rhs, lhs);
        }
    }

} // namespace yato

} // namespace conf

