/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_CMD_CONFIG_H_
#define _YATO_CONFIG_CMD_CONFIG_H_

#include <yato/array_view.h>
#include "../config.h"

namespace yato {

namespace conf {

    class cmd_config_state;

    class cmd_config
        : public config_backend
    {
    private:
        std::unique_ptr<cmd_config_state> m_impl;

        bool do_is_object() const noexcept override;
        details::value_variant do_get_by_name(const std::string & name, config_type type) const noexcept override;

        bool do_is_array() const noexcept override;
        details::value_variant do_get_by_index(size_t index, config_type type) const noexcept override;

        size_t do_get_size() const noexcept override;

    public:
        cmd_config(std::unique_ptr<cmd_config_state> && impl);
        ~cmd_config();

        cmd_config(const cmd_config&) = delete;
        cmd_config(cmd_config&&) noexcept;

        cmd_config& operator=(const cmd_config&) = delete;
        cmd_config& operator=(cmd_config&&) noexcept;
    };


    /**
     * Specifies required arguments and parces command line generating config instance.
     */
    class cmd_builder
    {
        std::unique_ptr<cmd_config_state> m_impl;
        
    public:
        explicit
        cmd_builder(const std::string & description);

        cmd_builder(const std::string & description, const std::string & version);

        ~cmd_builder();

        cmd_builder(const cmd_builder &) = delete;
        cmd_builder(cmd_builder&&) noexcept;

        cmd_builder& operator=(const cmd_builder&) = delete;
        cmd_builder& operator=(cmd_builder&&) noexcept;

        /**
         * Add integer argument.
         * @param flag One letter flag.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value. If empty, then value is required to be passed.
         */
        cmd_builder& integer(const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<int64_t> & default_value  = yato::nullopt_t{});

        /**
         * Add floating-point argument.
         * @param flag One letter flag.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value. If empty, then value is required to be passed.
         */
        cmd_builder& floating(const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<double> & default_value  = yato::nullopt_t{});

        /**
         * Add string argument.
         * @param flag One letter flag.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value. If empty, then value is required to be passed.
         */
        cmd_builder& string(const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<std::string> & default_value  = yato::nullopt_t{});

        /**
         * Add boolean flag.
         * @param flag One letter flag.
         * @param name Full argument name.
         * @param description Argumant description.
         */
        cmd_builder& boolean(const std::string & flag, const std::string & name, const std::string & description);

        /**
         * Parce command line
         */
        config parse(int argc, const char* const* argv);

        /**
         * Parce command line
         */
        config parse(const yato::array_view_1d<std::string> & args);
    };

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_JSON_CONFIG_H_
