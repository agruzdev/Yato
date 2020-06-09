/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "yato/config/manual/manual.h"
#include "yato/config/manual/private/manual_config.h"
#include "yato/assertion.h"

namespace yato {

namespace conf {

    struct manual_builder_state
    {
        std::unique_ptr<manual_config> conf;
    };

    manual_builder::manual_builder(details::object_tag_t t)
        : m_impl(std::make_unique<manual_builder_state>())
    {
        m_impl->conf = std::make_unique<manual_config>(t);
    }

    manual_builder::manual_builder(details::array_tag_t t)
        : m_impl(std::make_unique<manual_builder_state>())
    {
        m_impl->conf = std::make_unique<manual_config>(t);
    }

    manual_builder::manual_builder(const manual_builder& other)
        : m_impl(std::make_unique<manual_builder_state>())
    {
        if (other.m_impl->conf) {
            m_impl->conf = other.m_impl->conf->shallow_copy();
        }
    }

    manual_builder::manual_builder(manual_builder&& other) noexcept
        : m_impl(std::move(other.m_impl))
    {
        other.m_impl = nullptr;
    }

    manual_builder::~manual_builder() = default;


    manual_builder& manual_builder::operator=(const manual_builder& other)
    {
        YATO_REQUIRES(&other != this);
        m_impl->conf.reset();
        if (other.m_impl->conf) {
            m_impl->conf = other.m_impl->conf->shallow_copy();
        }
        return *this;
    }


    manual_builder& manual_builder::operator=(manual_builder&& other) noexcept
    {
        YATO_REQUIRES(&other != this);
        m_impl = std::move(other.m_impl);
        other.m_impl = nullptr;
        return *this;
    }


    manual_builder_state* manual_builder::checked_handle_() const
    {
        if (!m_impl || !m_impl->conf) {
            throw config_error("manual_builder[validate_]: Invalid builder state.");
        }
        return m_impl.get();
    }


    manual_builder & manual_builder::put(const std::string & name, int8_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::put(const std::string & name, int16_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::put(const std::string & name, int32_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::put(const std::string & name, int64_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(val));
        return *this;
    }


    manual_builder & manual_builder::put(const std::string & name, uint8_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::put(const std::string & name, uint16_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::put(const std::string & name, uint32_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::put(const std::string & name, uint64_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(yato::narrow_cast<int64_t>(val)));
        return *this;
    }


    manual_builder & manual_builder::put(const std::string & name, float val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::real>>(static_cast<double>(val)));
        return *this;
    }

    manual_builder & manual_builder::put(const std::string & name, double val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::real>>(val));
        return *this;
    }


    manual_builder & manual_builder::put(const std::string & name, bool val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::boolean>>(val));
        return *this;
    }


    manual_builder & manual_builder::put(const std::string & name, const char* val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::string>>(std::string(val)));
        return *this;
    }

    manual_builder & manual_builder::put(const std::string & name, std::string val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::string>>(std::move(val)));
        return *this;
    }


    manual_builder & manual_builder::put(const std::string & name, config val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::config>>(val.backend_handle_()));
        return *this;
    }


    manual_builder & manual_builder::add(int8_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::add(int16_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::add(int32_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::add(int64_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(val));
        return *this;
    }


    manual_builder & manual_builder::add(uint8_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::add(uint16_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::add(uint32_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    manual_builder & manual_builder::add(uint64_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(yato::narrow_cast<int64_t>(val)));
        return *this;
    }


    manual_builder & manual_builder::add(float val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::real>>(static_cast<double>(val)));
        return *this;
    }

    manual_builder & manual_builder::add(double val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::real>>(val));
        return *this;
    }


    manual_builder & manual_builder::add(bool val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::boolean>>(val));
        return *this;
    }


    manual_builder & manual_builder::add(const char* val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::string>>(std::string(val)));
        return *this;
    }

    manual_builder & manual_builder::add(std::string val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::string>>(std::move(val)));
        return *this;
    }


    manual_builder & manual_builder::add(config val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::config>>(val.backend_handle_()));
        return *this;
    }

    config manual_builder::create()
    {
        config res = config(static_cast<std::shared_ptr<manual_config>>(std::move(checked_handle_()->conf)));
        m_impl.reset();
        return res;
    }

} // namespace conf

} // namespace yato

