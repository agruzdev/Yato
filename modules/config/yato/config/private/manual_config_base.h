/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_CONFIG_BASE_H_
#define _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_CONFIG_BASE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "yato/config/config.h"
#include "yato/variant_match.h"
#include "manual_value.h"

namespace yato {

namespace conf {


    class manual_config_base
        : public config_backend
    {
    public:
        static
        std::shared_ptr<manual_config_base> copy(const yato::config& c, bool deep_copy);

        static
        std::shared_ptr<manual_config_base> deep_copy(const yato::config& c);

        static
        std::shared_ptr<manual_config_base> shallow_copy(const yato::config& c);

        static
        std::shared_ptr<manual_config_base> copy_with_path(const yato::config& other, const conf::path& path, stored_variant value);

        static
        std::shared_ptr<manual_config_base> copy_without_path(const yato::config& other, const conf::path& path);

        static
        std::shared_ptr<manual_config_base> copy_only_path(const yato::config& other, const conf::path& path);

        static
        std::shared_ptr<manual_config_base> copy_with_whitelist(const yato::config& other, std::vector<std::string> names);

        static
        std::shared_ptr<manual_config_base> copy_with_blacklist(const yato::config& other, std::vector<std::string> names);

        static
        std::shared_ptr<manual_config_base> merge(const config& lhs, const config& rhs, conf::priority p = conf::priority::left);


        virtual void put(std::string name, std::unique_ptr<config_value>&& value) = 0;

        virtual void remove(const std::string& name) = 0;

        virtual void add(std::unique_ptr<config_value>&& value) = 0;

        virtual void pop() = 0;
    };


} // namespace yato

} // namespace conf

#endif // _YATO_CONFIG_MANUAL_PRIVATE_MANUAL_CONFIG_BASE_H_
