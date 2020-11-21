/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_MAP_H_
#define _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_MAP_H_

#include <map>
#include <memory>
#include <string>

#include "manual_config_base.h"

namespace yato {

namespace conf {

    class manual_map final
        : public manual_config_base
    {
    public:
        manual_map() = default;

        manual_map(const manual_map&) = delete;

        manual_map(manual_map&& other) noexcept = default;

        ~manual_map() override = default;

        manual_map& operator=(const manual_map&) = delete;

        manual_map& operator=(manual_map&& other) noexcept = default;

    private:
        void put(std::string name, std::unique_ptr<config_value>&& value) override
        {
            m_data.insert_or_assign(std::move(name), std::move(value));
        }

        void remove(const std::string& name) override
        {
            m_data.erase(name);
        }

        void add(std::unique_ptr<config_value>&& /*value*/)
        {
            throw config_error("manual_map[add]: Config must be not associative.");
        }

        void pop()
        {
            throw config_error("manual_map[pop]: Config must be not associative.");
        }

        size_t do_size() const noexcept override
        {
            return m_data.size();
        }

        bool do_has_property(config_property p) const noexcept override
        {
            switch (p) {
            case config_property::associative:
                return true;
            default:
                return false;
            }
        }

        key_value_t do_find(size_t index) const noexcept override
        {
            key_value_t kv{};
            if (index < m_data.size()) {
                const auto it = std::next(m_data.cbegin(), index);
                kv.first  = (*it).first;
                kv.second = (*it).second.get();
            }
            return kv;
        }

        key_value_t do_find(const std::string& name) const noexcept override
        {
            key_value_t kv{};
            const auto it = m_data.find(name);
            if (it != m_data.cend()) {
                kv.first  = (*it).first;
                kv.second = (*it).second.get();
            }
            return kv;
        }

        void do_release(const config_value* /*val*/) const noexcept override
        {
            return;
        }


        std::map<std::string, std::unique_ptr<config_value>> m_data;
    };


} // namespace yato

} // namespace conf

#endif // _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_MAP_H_
