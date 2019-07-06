/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_CONFIG_H_
#define _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_CONFIG_H_

#include <map>
#include <vector>

#include "yato/config/config.h"
#include "yato/variant_match.h"
#include "manual_value.h"

namespace yato {

namespace conf {


    /**
     * Implements object
     */
    using manual_object_t = std::map<std::string, manual_value>;
    
    /**
     * Implements array
     */
    using manual_array_t  = std::vector<manual_value>;


    class manual_config final
        : public config_backend
    {
    public:
        manual_config(details::object_tag_t)
            : m_data(in_place_type_t<manual_object_t>{})
        { }

        manual_config(details::array_tag_t)
            : m_data(in_place_type_t<manual_array_t>{})
        { }

        manual_config(manual_object_t && obj)
            : m_data(std::move(obj))
        { }

        manual_config(manual_array_t && arr)
            : m_data(std::move(arr))
        { }

        ~manual_config() = default;

        manual_config(const manual_config&) = delete;
        manual_config(manual_config&&) = delete;

        manual_config& operator=(const manual_config&) = delete;
        manual_config& operator=(manual_config&&) = delete;

        
        void put(std::string name, manual_value value)
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

        void add(manual_value value)
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
                        kv.second = &((*it).second);
                    }
                },
                [&](const manual_array_t & arr) {
                    if(index < arr.size()) {
                        kv.second = &arr[index];
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
                        kv.second = &((*it).second);
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

    private:
        yato::variant<manual_object_t, manual_array_t> m_data;
    };



} // namespace yato

} // namespace conf

#endif // _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_CONFIG_H_
