/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_OBJECT_H_
#define _YATO_CONFIG_MANUAL_OBJECT_H_

#include "../config.h"

namespace yato {

namespace conf {



    namespace details
    {
        struct object_tag_t {};
        struct array_tag_t  {};

        using manual_scalar = yato::variant<int64_t, double, bool, std::string>;
    }

    class manual_config_state;



    class manual_config
        : public basic_config
    {
    private:
        std::unique_ptr<manual_config_state> m_impl;

        bool do_is_object() const noexcept override;
        details::value_variant do_get_by_name(const std::string & name, config_type type) const noexcept override;

        bool do_is_array() const noexcept override;
        details::value_variant do_get_by_index(size_t index, config_type type) const noexcept override;

        size_t do_get_size() const noexcept override;

    public:
        manual_config(std::unique_ptr<manual_config_state> && impl);
        ~manual_config() = default;
    };




    class manual_builder
    {
        std::unique_ptr<manual_config_state> m_impl;

        void put_scalar_(const std::string & key, details::manual_scalar && val);
        void put_object_(const std::string & key, config_ptr && conf);

        void append_scalar_(details::manual_scalar && val);
        void append_object_(config_ptr && conf);

        manual_builder(details::object_tag_t);
        manual_builder(details::array_tag_t);

    public:
        ~manual_builder();

        manual_builder(const manual_builder&) = delete;
        manual_builder(manual_builder &&) noexcept;

        manual_builder& operator=(const manual_builder&) = delete;
        manual_builder& operator=(manual_builder&&) noexcept;

        static
        manual_builder object() {
            return manual_builder(details::object_tag_t{});
        }

        static
        manual_builder array() {
            return manual_builder(details::array_tag_t{});
        }

        manual_builder & put(const std::string & key, int8_t val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, int16_t val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, int32_t val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, int64_t val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, uint8_t val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, uint16_t val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, uint32_t val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, uint64_t val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<int64_t>{}, yato::narrow_cast<int64_t>(val)));
            return *this;
        }

        manual_builder & put(const std::string & key, float val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<double>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, double val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<double>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, bool val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<bool>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, const char* val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<std::string>{}, std::string(val)));
            return *this;
        }

        manual_builder & put(const std::string & key, const std::string & val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<std::string>{}, val));
            return *this;
        }

        manual_builder & put(const std::string & key, std::string && val)
        {
            put_scalar_(key, details::manual_scalar(yato::in_place_type_t<std::string>{}, std::move(val)));
            return *this;
        }

        manual_builder & put(const std::string & key, const config_ptr & val)
        {
            put_object_(key, config_ptr(val));
            return *this;
        }

        manual_builder & put(const std::string & key, config_ptr && val)
        {
            put_object_(key, std::move(val));
            return *this;
        }

        manual_builder & add(int8_t val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & add(int16_t val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & add(int32_t val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & add(int64_t val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & add(uint8_t val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & add(uint16_t val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & add(uint32_t val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<int64_t>{}, val));
            return *this;
        }

        manual_builder & add(uint64_t val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<int64_t>{}, yato::narrow_cast<int64_t>(val)));
            return *this;
        }

        manual_builder & add(float val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<double>{}, val));
            return *this;
        }

        manual_builder & add(double val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<double>{}, val));
            return *this;
        }

        manual_builder & add(bool val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<bool>{}, val));
            return *this;
        }

        manual_builder & add(const char* val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<std::string>{}, std::string(val)));
            return *this;
        }

        manual_builder & add(const std::string & val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<std::string>{}, val));
            return *this;
        }

        manual_builder & add(std::string && val)
        {
            append_scalar_(details::manual_scalar(yato::in_place_type_t<std::string>{}, std::move(val)));
            return *this;
        }

        manual_builder & add(const config_ptr & val)
        {
            append_object_(config_ptr(val));
            return *this;
        }

        manual_builder & add(config_ptr && val)
        {
            append_object_(std::move(val));
            return *this;
        }

        config_ptr create() noexcept;
    };

} // namespace yato

} // namespace conf

#endif // _YATO_CONFIG_MANUAL_OBJECT_H_
