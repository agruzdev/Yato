/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_VALUE_H_
#define _YATO_CONFIG_MANUAL_VALUE_H_

#include <yato/config/config.h>

namespace yato {

    class manual_value
        : public config_value
    {
    private:
        using manual_type = yato::variant<int64_t, double, std::string>;
        manual_type m_value;

        yato::optional<int64_t> do_get_int() const noexcept override {
            return m_value.type() == typeid(int64_t) ? yato::make_optional(m_value.get_as<int64_t>()) : yato::nullopt_t{};
        }

        yato::optional<double> do_get_real() const noexcept override {
            return m_value.type() == typeid(double) ? yato::make_optional(m_value.get_as<double>()) : yato::nullopt_t{};
        }
    
        yato::optional<std::string> do_get_string() const noexcept override {
            return m_value.type() == typeid(std::string) ? yato::make_optional(m_value.get_as<std::string>()) : yato::nullopt_t{};
        }

        void* do_get_underlying_type() noexcept override {
            return static_cast<void*>(&m_value);
        }
    
    public:
        manual_value(int8_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        manual_value(int16_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        manual_value(int32_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        manual_value(int64_t val)
            : m_value(val)
        { }
    
        manual_value(uint8_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        manual_value(uint16_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        manual_value(uint32_t val)
            : m_value(static_cast<int64_t>(val))
        { }
    
        manual_value(uint64_t val)
            : m_value(yato::narrow_cast<int64_t>(val))
        { }
    
        manual_value(float val)
            : m_value(static_cast<double>(val))
        { }
    
        manual_value(double val)
            : m_value(val)
        { }
    
        manual_value(const std::string & val)
            : m_value(val)
        { }
    
        manual_value(std::string && val)
            : m_value(std::move(val))
        { }
    
        ~manual_value() = default;
    
        manual_value(const manual_value&) = default;
        manual_value(manual_value&&) = default;
    
        manual_value& operator = (const manual_value&) = default;
        manual_value& operator = (manual_value&&) = default;
    };

} // namespace yato

#endif // _YATO_CONFIG_MANUAL_VALUE_H_
