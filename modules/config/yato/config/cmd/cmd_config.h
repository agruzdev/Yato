/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016-2018 Alexey Gruzdev
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

        bool is_object() const noexcept override;
        stored_variant get_by_name(const std::string & name, config_type type) const noexcept override;

        bool is_array() const noexcept override;
        stored_variant get_by_index(size_t index, config_type type) const noexcept override;

        size_t size() const noexcept override;
        std::vector<std::string> keys() const noexcept override;

    public:
        cmd_config(std::unique_ptr<cmd_config_state> && impl);
        ~cmd_config();

        cmd_config(const cmd_config&) = delete;
        cmd_config(cmd_config&&) noexcept;

        cmd_config& operator=(const cmd_config&) = delete;
        cmd_config& operator=(cmd_config&&) noexcept;
    };

    /**
     * Types of supported arguments
     */
    enum class cmd_argument
    {
        /**
         * Identified by their position in the command line. 
         * Positional arguemnts are parsed in the same order, as added to the builder.
         */
        positional,
        
        /**
         * Arguments required to be presented. If any of them is missing, then parsing excetion will be thrown.
         */
        required,

        /**
         * Optianally presended in the command line.
         */
        optional
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
         * @param kind Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder& integer(cmd_argument kind, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<int64_t> & default_value  = yato::nullopt_t{});

        /**
         * Add floating-point argument.
         * @param kind Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder& floating(cmd_argument kind, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<double> & default_value  = yato::nullopt_t{});

        /**
         * Add string argument.
         * @param kind Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder& string(cmd_argument kind, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<std::string> & default_value  = yato::nullopt_t{});

        /**
         * Add boolean flag. Always optional.
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
