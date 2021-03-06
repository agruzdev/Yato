/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "config.h"
#include "yato/config/private/manual_config_base.h"

namespace yato {

namespace conf {

    config config::clone() const
    {
        return config(manual_config_base::deep_copy(*this));
    }

    config config::with_value_(const yato::conf::path &path, yato::conf::stored_variant value) const
    {
        return config(manual_config_base::copy_with_path(*this, path, std::move(value)));
    }

    config config::without_path(const conf::path &path) const
    {
        return config(manual_config_base::copy_without_path(*this, path));
    }

    config config::with_only_path(const conf::path &path) const
    {
        return config(manual_config_base::copy_only_path(*this, path));
    }

    config config::merged_with(const config& other, priority p) const
    {
        return config(manual_config_base::merge(*this, other, p));
    }

    config config::with_whitelist(std::vector<std::string> names) const
    {
        return config(manual_config_base::copy_with_whitelist(*this, std::move(names)));
    }

    config config::with_blacklist(std::vector<std::string> names) const
    {
        return config(manual_config_base::copy_with_blacklist(*this, std::move(names)));
    }

} // namespace conf

} // namespace yato