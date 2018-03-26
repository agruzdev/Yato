/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#include <map>
#include "manual_array.h"
#include "manual_object.h"

namespace yato {

namespace conf {

    using element_type = yato::variant<void, config_value, manual_array, manual_object>;

    struct manual_object_impl {
        std::map<std::string, element_type> data;
    };


    template <typename Ty_>
    const Ty_* get_field_impl_(const manual_object_impl & self, const std::string & key)
    {
        const auto it = self.data.find(key);
        if (it == self.data.cend()) {
            return nullptr;
        }
        if(it->second.type() != typeid(Ty_)) {
            return nullptr;
        }
        return &it->second.get_as_unsafe<Ty_>();
    }

    template <typename Ty_>
    void put_impl_(manual_object_impl & self, const std::string & key, Ty_ && obj) 
    {
        self.data[key] = static_cast<element_type>(std::forward<Ty_>(obj));
    }

    std::vector<std::string> manual_object::do_get_keys() const noexcept
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        std::vector<std::string> keys;
        keys.reserve(m_pimpl->data.size());
        for(auto it : m_pimpl->data) {
            keys.push_back(it.first);
        }
        return keys;
    }

    yato::optional<config_value> manual_object::do_get_value(const std::string & key) const noexcept
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        const auto it = get_field_impl_<config_value>(*m_pimpl, key);
        if(it != nullptr) {
            return yato::make_optional(*it);
        }
        return yato::nullopt_t{};
    }

    const config_object* manual_object::do_get_object(const std::string & key) const noexcept
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        return get_field_impl_<manual_object>(*m_pimpl, key);
    }

    const config_array* manual_object::do_get_array(const std::string & key) const noexcept
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        return get_field_impl_<manual_array>(*m_pimpl, key);
    }

    void* manual_object::do_get_underlying_type() noexcept
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        return &m_pimpl->data;
    }

    manual_object::manual_object()
        : m_pimpl(new manual_object_impl())
    { }

    manual_object::~manual_object() = default;

    manual_object::manual_object(const manual_object & other)
        : m_pimpl(new manual_object_impl())
    {
        YATO_ASSERT(other.m_pimpl != nullptr, "null impl");
        m_pimpl->data = std::map<std::string, element_type>(other.m_pimpl->data);
    }

    manual_object::manual_object(manual_object&&) noexcept = default;

    manual_object& manual_object::operator =(const manual_object & other)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        YATO_ASSERT(other.m_pimpl != nullptr, "null impl");
        m_pimpl->data = other.m_pimpl->data;
        return *this;
    }

    manual_object& manual_object::operator =(manual_object&&) noexcept = default;

    void manual_object::put(const std::string & key, const config_value & val) 
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, key, val);
    }

    void manual_object::put(const std::string & key, config_value && val) 
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, key, std::move(val));
    }

    void manual_object::put(const std::string & key, const manual_array & val)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, key, val);
    }

    void manual_object::put(const std::string & key, manual_array && val)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, key, std::move(val));
    }

    void manual_object::put(const std::string & key, const manual_object & val)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, key, val);
    }

    void manual_object::put(const std::string & key, manual_object && val)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, key, std::move(val));
    }

    void manual_object::swap(manual_object & other) noexcept
    {
        m_pimpl.swap(other.m_pimpl);
    }


} // namespace conf

} // namespace yato
