/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_H_
#define _YATO_CONFIG_MANUAL_H_

#include "yato/config/config.h"

namespace yato {

namespace conf {

    struct manual_builder_state;

    /**
     * Builder for manual config.
     * After building config builder becomes empty and can't be reused.
     */
    class manual_builder
    {
    public:
        /**
         * Make root object
         */
        static
        manual_builder object() {
            return manual_builder(details::object_tag_t{});
        }

        /**
         * Make root array
         */
        static
        manual_builder array() {
            return manual_builder(details::array_tag_t{});
        }


        /**
         * Deep copy builder state
         */
        manual_builder(const manual_builder& other);

        /**
         * Move builder state
         */
        manual_builder(manual_builder&& other) noexcept;


        ~manual_builder();

        /**
         * Deep copy builder state
         */
        manual_builder& operator=(const manual_builder& other);

        /**
         * Move builder state
         */
        manual_builder& operator=(manual_builder&& other) noexcept;


        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, int8_t val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, int16_t val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, int32_t val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, int64_t val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, uint8_t val);
        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, uint16_t val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, uint32_t val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, uint64_t val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, float val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, double val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, bool val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, const char* val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, std::string val);

        /**
         * Add named value.
         * Is valid only for object.
         */
        manual_builder & put(const std::string & name, config val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(int8_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(int16_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(int32_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(int64_t val);
        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(uint8_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(uint16_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(uint32_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(uint64_t val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(float val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(double val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(bool val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(const char* val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(std::string val);

        /**
         * Append value to array.
         * Is valid only for array.
         */
        manual_builder & add(config val);

        /**
         * Finish building of config object
         */
        config create();


    private:
        manual_builder(details::object_tag_t);
        manual_builder(details::array_tag_t);

        manual_builder_state* checked_handle_() const;

        std::unique_ptr<manual_builder_state> m_impl;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_MANUAL_H_
