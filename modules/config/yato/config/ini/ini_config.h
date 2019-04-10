/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_CONFIG_H_
#define _YATO_CONFIG_INI_CONFIG_H_

#include "../config.h"

namespace yato {

namespace conf {

    struct ini_config_state;

    class ini_config
        : public config_backend
    {
    private:
        std::unique_ptr<ini_config_state> m_impl;

        size_t size() const noexcept override;

        bool is_object() const noexcept override;

        stored_variant get_by_key(const std::string & name, stored_type type) const noexcept override;

        stored_variant get_by_index(size_t index, stored_type type) const noexcept override;

        std::vector<std::string> keys() const noexcept override;

        stored_variant decode_value_(const char* raw, stored_type type) const noexcept;

    public:
        ini_config(std::unique_ptr<ini_config_state> && impl);
        ~ini_config();

        ini_config(const ini_config&) = delete;
        ini_config(ini_config&&) noexcept;

        ini_config& operator=(const ini_config&) = delete;
        ini_config& operator=(ini_config&&) noexcept;
    };


    class ini_builder
    {
        std::unique_ptr<ini_config_state> m_impl;

        config finalize();

    public:
        ini_builder();

        ~ini_builder();

        ini_builder(const ini_builder &) = delete;
        ini_builder(ini_builder&&) noexcept = default;

        ini_builder& operator=(const ini_builder&) = delete;
        ini_builder& operator=(ini_builder&&) noexcept = default;

        /**
         * Allow data values to span multiple lines in the input
         */
        ini_builder& multiline(bool enable);

        /**
         * Parse ini file
         */
        config parse_file(const char* filename);

        /**
         * Parse ini file 
         */
        config parse_file(const std::string & filename);

        /**
         * Parse from stream
         */
        config parse(std::istream & is);

        /**
         * Parse from string
         */
        config parse(const char* ini);

        /**
         * Parse from string
         */
        config parse(const std::string & ini);
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_INI_CONFIG_H_
