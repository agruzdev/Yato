/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_ARRAY_H_
#define _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_ARRAY_H_

#include <memory>
#include <string>
#include <vector>

#include "manual_config_base.h"

namespace yato {

namespace conf {


    class manual_array final
        : public manual_config_base
    {
    public:
        manual_array() = default;

        manual_array(const manual_array&) = delete;

        manual_array(manual_array&& other) noexcept = default;

        ~manual_array() override = default;

        manual_array& operator=(const manual_array&) = delete;

        manual_array& operator=(manual_array&& other) noexcept = default;

    private:
        void put(std::string /*name*/, std::unique_ptr<config_value>&& /*value*/) override
        {
            throw config_error("manual_array[put]: Config must be associative.");
        }

        void remove(const std::string& /*name*/) override
        {
            throw config_error("manual_array[remove]: Config must be associative.");
        }

        void add(std::unique_ptr<config_value>&& value) override
        {
            m_data.push_back(std::move(value));
        }

        void pop() override
        {
            m_data.pop_back();
        }

        size_t do_size() const noexcept override
        {
            return m_data.size();
        }

        bool do_has_property(config_property p) const noexcept override
        {
            switch (p) {
            case config_property::keeps_order:
                return true;
            default:
                return false;
            }
        }

        find_index_result_t do_find(size_t index) const override
        {
            find_index_result_t result = config_backend::no_index_result;
            if (index < m_data.size()) {
                std::get<1>(result) = m_data[index].get();
            }
            return result;
        }

        find_key_result_t do_find(const std::string & /*name*/) const override
        {
            return config_backend::no_key_result;
        }

        void do_release(const config_value* /*val*/) const noexcept override
        {
            // do nothing
        }


        std::vector<std::unique_ptr<config_value>> m_data;
    };



} // namespace yato

} // namespace conf

#endif // _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_ARRAY_H_
