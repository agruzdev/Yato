/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#include "manual_array.h"
#include "manual_object.h"

namespace yato {

namespace conf {

    using element_type = yato::variant<void, config_value, manual_array, manual_object>;

    struct manual_array_impl {
        std::vector<element_type> data;
    };

    template <typename Ty_>
    const Ty_* get_impl_(const manual_array_impl & self, size_t idx) {
        if(idx >= self.data.size()) {
            return nullptr;
        }
        const element_type & elem = self.data[idx];
        if(elem.type() == typeid(Ty_)) {
            return &elem.get_as_unsafe<Ty_>();
        }
        else {
            return nullptr;
        }
    }

    template <typename Ty_>
    void put_impl_(manual_array_impl & self, size_t idx, Ty_ && obj) {
        if(idx < self.data.size()) {
            self.data[idx] = static_cast<element_type>(std::forward<Ty_>(obj));
        }
    }


    size_t manual_array::do_get_size() const noexcept {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        return m_pimpl->data.size();
    }

    config_value manual_array::do_get_value(size_t idx) const noexcept {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        return *get_impl_<config_value>(*m_pimpl, idx);
    }

    const config_object* manual_array::do_get_object(size_t idx) const noexcept {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        return get_impl_<config_object>(*m_pimpl, idx);
    }

    const config_array* manual_array::do_get_array(size_t idx) const noexcept {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        return get_impl_<manual_array>(*m_pimpl, idx);
    }

    void* manual_array::do_get_underlying_type() noexcept {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        return &m_pimpl->data;
    }

    manual_array::manual_array()
        : m_pimpl(new manual_array_impl())
    { }

    manual_array::~manual_array() = default;
    
    manual_array::manual_array(const manual_array & other)
        : m_pimpl(new manual_array_impl())
    {
        YATO_ASSERT(other.m_pimpl != nullptr, "null impl");
        m_pimpl->data = std::vector<element_type>(other.m_pimpl->data);
    }

    manual_array::manual_array(manual_array&&) noexcept = default;
    
    manual_array& manual_array::operator =(const manual_array & other)
    {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        YATO_ASSERT(other.m_pimpl != nullptr, "null impl");
        m_pimpl->data = other.m_pimpl->data;
        return *this;
    }

    manual_array& manual_array::operator =(manual_array&&) noexcept = default;
    
    void manual_array::resize(size_t size) {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        m_pimpl->data.resize(size);
    }
    
    void manual_array::put(size_t idx, const config_value & val) {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, idx, val);
    }

    void manual_array::put(size_t idx, config_value && val) {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, idx, std::move(val));
    }

    void manual_array::put(size_t idx, const manual_array & arr) {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, idx, arr);
    }

    void manual_array::put(size_t idx, manual_array && arr) {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, idx, std::move(arr));
    }

    void manual_array::put(size_t idx, const manual_object & obj) {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, idx, obj);
    }

    void manual_array::put(size_t idx, manual_object && obj) {
        YATO_ASSERT(m_pimpl != nullptr, "null impl");
        put_impl_(*m_pimpl, idx, std::move(obj));
    }

    void manual_array::swap(manual_array & other) noexcept {
        m_pimpl.swap(other.m_pimpl);
    }

} // namespace conf

} // namespace yato
