/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_INI_PRIVATE_INI_CONFIG_H_
#define _YATO_CONFIG_INI_PRIVATE_INI_CONFIG_H_

#include <map>
#include <memory>

#include "../../config_backend.h"
#include "ini_lib.h"

namespace yato {

namespace conf {

    class ini_config final
        : public config_backend
    {
    public:
        /**
         * Global section config
         */
        ini_config(std::shared_ptr<CSimpleIniA> reader);

        /**
         * Nested section config
         */
        ini_config(std::shared_ptr<CSimpleIniA> reader, ini_section_ptr section);

        ini_config(const ini_config&) = delete;
        ini_config(ini_config&&) = delete;

        ini_config& operator=(const ini_config&) = delete;
        ini_config& operator=(ini_config&&) = delete;

        ~ini_config();

        size_t do_size() const noexcept override;

        key_value_t do_find(size_t index) const noexcept override;

        key_value_t do_find(const std::string & name) const noexcept override;

        void do_release(const config_value* val) const noexcept override;

        bool do_is_object() const noexcept override;

    private:
        std::shared_ptr<CSimpleIniA> m_reader;
        ini_section_ptr m_section;
        std::map<std::string, ini_section_ptr> m_nested_sections;
    };

} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_INI_PRIVATE_INI_CONFIG_H_
