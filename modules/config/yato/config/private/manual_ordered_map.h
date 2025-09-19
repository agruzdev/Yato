/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2025 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_ORDERED_MAP_H_
#define _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_ORDERED_MAP_H_

#include <deque>
#include <tuple>
#include <memory>
#include <string>
#include <unordered_map>

#include "manual_config_base.h"

namespace yato {

namespace conf {

    class manual_ordered_map final
        : public manual_config_base
    {
    public:
        manual_ordered_map() = default;

        manual_ordered_map(const manual_ordered_map&) = delete;

        manual_ordered_map(manual_ordered_map&& other) noexcept = default;

        ~manual_ordered_map() override = default;

        manual_ordered_map& operator=(const manual_ordered_map&) = delete;

        manual_ordered_map& operator=(manual_ordered_map&& other) noexcept = default;

    private:
        void put(std::string name, std::unique_ptr<config_value>&& value) override
        {
            auto it = std::find_if(m_values.begin(), m_values.end(), [&name](const record_type& r) { return std::get<0>(r) == name; });
            if (it != m_values.end()) {
                std::get<1>(*it) = std::move(value);
            }
            else {
                m_values.emplace_back(std::move(name), std::move(value));
            }
        }

        void remove(const std::string& name) override
        {
            auto it = std::find_if(m_values.cbegin(), m_values.cend(), [&name](const record_type& r) { return std::get<0>(r) == name; });
            if (it != m_values.cend()) {
                m_values.erase(it);
            }
        }

        void add(std::unique_ptr<config_value>&& /*value*/) override
        {
            throw config_error("manual_ordered_map[add]: Config must be not associative.");
        }

        void pop() override
        {
            throw config_error("manual_ordered_map[pop]: Config must be not associative.");
        }

        size_t do_size() const noexcept override
        {
            return m_values.size();
        }

        bool do_has_property(config_property p) const noexcept override
        {
            switch (p) {
            case config_property::associative:
            case config_property::keeps_order:
                return true;
            default:
                return false;
            }
        }

        find_index_result_t do_find(size_t index) const override
        {
            find_index_result_t result = config_backend::no_index_result;
            if (index < m_values.size()) {
                const auto& t = m_values[index];
                result = std::make_tuple(std::get<0>(t), std::get<1>(t).get());
            }
            return result;
        }

        find_key_result_t do_find(const std::string& name) const override
        {
            find_key_result_t result = config_backend::no_key_result;
            auto it = std::find_if(m_values.cbegin(), m_values.cend(), [&name](const record_type& r) { return std::get<0>(r) == name; });
            if (it != m_values.cend()) {
                result = std::make_tuple(yato::narrow_cast<size_t>(std::distance(m_values.cbegin(), it)), std::get<1>(*it).get());
            }
            return result;
        }

        void do_release(const config_value* /*val*/) const noexcept override
        {
            // do nothing
        }


        using record_type = std::tuple<std::string, std::unique_ptr<config_value>>;
        std::vector<record_type> m_values;
    };


} // namespace yato

} // namespace conf

#endif // _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_MAP_H_
