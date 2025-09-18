/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "yato/config/config_builder.h"
#include "yato/config/private/manual_array.h"
#include "yato/config/private/manual_map.h"
#include "yato/config/private/manual_multimap.h"
#include "yato/assertion.h"

namespace yato {

namespace conf {

    struct config_builder::builder_state
    {
        std::shared_ptr<manual_config_base> conf;
    };

    config_builder::config_builder(details::object_tag_t, bool multi_associative)
        : m_impl(std::make_unique<builder_state>())
    {
        m_impl->conf = multi_associative
            ? std::static_pointer_cast<manual_config_base>(std::make_shared<manual_multimap>())
            : std::static_pointer_cast<manual_config_base>(std::make_shared<manual_map>());
    }

    config_builder::config_builder(details::array_tag_t)
        : m_impl(std::make_unique<builder_state>())
    {
        m_impl->conf = std::make_shared<manual_array>();
    }

    config_builder::config_builder(const config& c, bool deep_copy)
        : m_impl(std::make_unique<builder_state>())
    {
        m_impl->conf = manual_config_base::copy(c, deep_copy);
    }

    config_builder::config_builder(const config_builder& other)
        : m_impl(std::make_unique<builder_state>())
    {
        if (other.m_impl->conf) {
            m_impl->conf = manual_config_base::shallow_copy(config(other.checked_handle_()->conf));
        }
    }

    config_builder::config_builder(config_builder&& other) noexcept
        : m_impl(std::move(other.m_impl))
    {
        other.m_impl = nullptr;
    }

    config_builder::~config_builder() = default;

    config_builder& config_builder::operator=(const config_builder& other)
    {
        YATO_REQUIRES(&other != this);
        m_impl->conf.reset();
        if (other.m_impl->conf) {
            m_impl->conf = manual_config_base::shallow_copy(config(other.checked_handle_()->conf));
        }
        return *this;
    }

    config_builder& config_builder::operator=(config_builder&& other) noexcept
    {
        YATO_REQUIRES(&other != this);
        m_impl = std::move(other.m_impl);
        other.m_impl = nullptr;
        return *this;
    }

    config_builder::builder_state* config_builder::checked_handle_() const
    {
        if (!m_impl || !m_impl->conf) {
            throw config_error("config_builder[checked_handle_]: Invalid builder state.");
        }
        return m_impl.get();
    }

    config_builder& config_builder::put(const std::string& name, int8_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, int16_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, int32_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, int64_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(val));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, uint8_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, uint16_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, uint32_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, uint64_t val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::integer>>(yato::narrow_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, float val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::real>>(static_cast<double>(val)));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, double val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::real>>(val));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, bool val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::boolean>>(val));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, const char* val)
    {
        if (val) {
            checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::string>>(std::string(val)));
        }
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, std::string val)
    {
        checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::string>>(std::move(val)));
        return *this;
    }

    config_builder& config_builder::put(const std::string& name, config val)
    {
        if (!val.is_null()) {
            checked_handle_()->conf->put(name, std::make_unique<manual_value<stored_type::config>>(val.backend_handle_()));
        }
        return *this;
    }

    config_builder& config_builder::remove(const std::string& name)
    {
        checked_handle_()->conf->remove(name);
        return *this;
    }

    config_builder& config_builder::add(int8_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::add(int16_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::add(int32_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::add(int64_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(val));
        return *this;
    }


    config_builder& config_builder::add(uint8_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::add(uint16_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::add(uint32_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(static_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::add(uint64_t val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::integer>>(yato::narrow_cast<int64_t>(val)));
        return *this;
    }

    config_builder& config_builder::add(float val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::real>>(static_cast<double>(val)));
        return *this;
    }

    config_builder& config_builder::add(double val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::real>>(val));
        return *this;
    }

    config_builder& config_builder::add(bool val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::boolean>>(val));
        return *this;
    }

    config_builder& config_builder::add(const char* val)
    {
        if (val) {
            checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::string>>(std::string(val)));
        }
        return *this;
    }

    config_builder& config_builder::add(std::string val)
    {
        checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::string>>(std::move(val)));
        return *this;
    }

    config_builder& config_builder::add(config val)
    {
        if (!val.is_null()) {
            checked_handle_()->conf->add(std::make_unique<manual_value<stored_type::config>>(val.backend_handle_()));
        }
        return *this;
    }

    config_builder& config_builder::pop()
    {
        checked_handle_()->conf->pop();
        return *this;
    }

    config config_builder::create()
    {
        YATO_ASSERT(checked_handle_()->conf.use_count() == 1, "Builder must be unique owner of the config");
        config res = config(std::move(checked_handle_()->conf));
        m_impl.reset();
        return res;
    }

} // namespace conf

} // namespace yato

