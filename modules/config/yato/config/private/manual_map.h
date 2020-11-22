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

        find_index_result_t do_find(size_t index) const override
        {
            find_index_result_t result = config_backend::no_index_result;
            if (index < m_data.size()) {
                const auto it = std::next(m_data.cbegin(), index);
                result = std::make_tuple((*it).first, (*it).second.get());
            }
            return result;
        }

        find_key_result_t do_find(const std::string& name) const override
        {
            find_key_result_t result = config_backend::no_key_result;
            const auto it = m_data.find(name);
            if (it != m_data.cend()) {
                result = std::make_tuple(yato::narrow_cast<size_t>(std::distance(m_data.cbegin(), it)), (*it).second.get());
            }
            return result;
        }

        void do_release(const config_value* /*val*/) const noexcept override
        {
            // do nothing
        }


        std::map<std::string, std::unique_ptr<config_value>> m_data;
    };


} // namespace yato

} // namespace conf

#endif // _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_MAP_H_
