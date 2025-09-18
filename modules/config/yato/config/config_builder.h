/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_BUILDER_H_
#define _YATO_CONFIG_BUILDER_H_

#include <string>
#include <memory>
#include <optional>
#include "yato/types.h"
#include "yato/config/config.h"

namespace yato {

namespace conf {


    /**
     * Builder for a config.
     * After building config builder becomes empty and can't be reused.
     */
    class config_builder
    {
    public:
        /**
         * Make root object
         */
        static
        config_builder object(bool multi_associative = false)
        {
            return config_builder(details::object_tag_t{}, multi_associative);
        }

        /**
         * Make root array
         */
        static
        config_builder array()
        {
            return config_builder(details::array_tag_t{});
        }

        /**
         * Convert from a complex type
         */
        template <typename Ty_, typename Converter_ = typename config_value_trait<Ty_>::converter_type>
        static
        config cvt(const Ty_& val, const Converter_& converter = Converter_{})
        {
            config res{};
            if (!invoke_skip(converter, val)) {
                res = invoke_store(converter, val);
            }
            return res;
        }

        /**
         * Uses a copy of existing config as initial state.
         * If 'deep_copy' is enabled then nexted objects are copyied, otherwise - shared
         */
        config_builder(const config& c, bool deep_copy = false);

        /**
         * Makes a shallow copy
         * Builder state is copyied, nested objects are shared.
         */
        config_builder(const config_builder& other);

        /**
         * Moves builder state
         */
        config_builder(config_builder&& other) noexcept;


        ~config_builder();

        /**
         * Makes a shallow copy
         * Builder state is copyied, nested objects are shared.
         */
        config_builder& operator=(const config_builder& other);

        /**
         * Moves builder state
         */
        config_builder& operator=(config_builder&& other) noexcept;


        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, int8_t val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, int16_t val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, int32_t val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, int64_t val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, uint8_t val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, uint16_t val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, uint32_t val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, uint64_t val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, float val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, double val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, bool val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, const char* val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, std::string val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& name, config val);

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        template <typename Ty_, typename Converter_ = typename config_value_trait<Ty_>::converter_type>
        config_builder& put(const std::string& name, const Ty_& val, const Converter_& converter = Converter_{})
        {
            if (!invoke_skip(converter, val)) {
                put(name, invoke_store(converter, val));
            }
            return *this;
        }

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& /*name*/, yato::nullopt_t)
        {
            return *this;
        }

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& /*name*/, std::nullopt_t)
        {
            return *this;
        }

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& put(const std::string& /*name*/, std::nullptr_t)
        {
            return *this;
        }

        /**
         * Removes values by name.
         * Is valid only for associative configs.
         */
        config_builder& remove(const std::string& name);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(int8_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(int16_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(int32_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(int64_t val);
        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(uint8_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(uint16_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(uint32_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(uint64_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(float val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(double val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(bool val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(const char* val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(std::string val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        config_builder& add(config val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        template <typename Ty_, typename Converter_ = typename config_value_trait<Ty_>::converter_type>
        config_builder& add(const Ty_& val, const Converter_& converter = Converter_{})
        {
            if (!invoke_skip(converter, val)) {
                add(invoke_store(converter, val));
            }
            return *this;
        }

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& add(yato::nullopt_t)
        {
            return *this;
        }

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& add(std::nullopt_t)
        {
            return *this;
        }

        /**
         * Add named value.
         * Is valid only for associative configs.
         */
        config_builder& add(std::nullptr_t)
        {
            return *this;
        }

        /**
         * Remove the last array element.
         * Is valid only for array.
         */
        config_builder& pop();

        /**
         * Finish building of config object
         */
        config create();


    private:
        struct builder_state;

        config_builder(details::object_tag_t, bool multi_associative);
        config_builder(details::array_tag_t);

        builder_state* checked_handle_() const;

        std::unique_ptr<builder_state> m_impl;
    };

} // namespace conf

    // import names
    using conf::config_builder;

} // namespace yato

#endif //_YATO_CONFIG_BUILDER_H_
