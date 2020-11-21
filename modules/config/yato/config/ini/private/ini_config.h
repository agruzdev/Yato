/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_PRIVATE_INI_CONFIG_H_
#define _YATO_CONFIG_INI_PRIVATE_INI_CONFIG_H_

#include <map>
#include <memory>

#include "../../config_backend.h"
#include "ini_parser.h"

namespace yato {

namespace conf {

    class ini_config final
        : public config_backend
    {
    public:
        /**
         * Global section config
         */
        ini_config(std::shared_ptr<ini_parser> parser);

        /**
         * Nested section config
         */
        ini_config(std::shared_ptr<ini_parser> parser, const ini_parser::kv_multimap* section_values);


        ~ini_config() override;

        size_t do_size() const noexcept override;

        std::vector<std::string> do_enumerate_keys() const noexcept override;

        key_value_t do_find(size_t index) const noexcept override;

        key_value_t do_find(const std::string & name) const noexcept override;

        void do_release(const config_value* val) const noexcept override;

        bool do_has_property(config_property p) const noexcept override;


        ini_config(const ini_config&) = delete;
        ini_config(ini_config&&) = delete;

        ini_config& operator=(const ini_config&) = delete;
        ini_config& operator=(ini_config&&) = delete;

    private:
        std::shared_ptr<ini_parser> m_parser;
        const ini_parser::kv_multimap* m_values = nullptr;
        bool m_is_global;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_INI_PRIVATE_INI_CONFIG_H_
