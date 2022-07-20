/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_CMD_CMD_H_
#define _YATO_CONFIG_CMD_CMD_H_

#include <string>
#include <vector>

#include "../config.h"
#include "yato/array_view.h"

namespace yato {

namespace conf {

    /**
     * Types of supported arguments
     */
    enum class argument_type
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

    class cmd_config;

    /**
     * Specifies required arguments and parces command line generating config instance.
     */
    class cmd_builder
    {
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
         * @param arg_type Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder& integer(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<int64_t> & default_value  = yato::nullopt_t{}) &;

        /**
         * Add integer argument.
         * @param arg_type Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder&& integer(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<int64_t> & default_value  = yato::nullopt_t{}) &&;

        /**
         * Add floating-point argument.
         * @param arg_type Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder& floating(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<double> & default_value  = yato::nullopt_t{}) &;

        /**
         * Add floating-point argument.
         * @param arg_type Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder&& floating(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<double> & default_value  = yato::nullopt_t{}) &&;

        /**
         * Add string argument.
         * @param arg_type Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder& string(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<std::string> & default_value  = yato::nullopt_t{}) &;

        /**
         * Add string argument.
         * @param arg_type Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder&& string(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<std::string> & default_value  = yato::nullopt_t{}) &&;

        /**
         * Add boolean flag. Always optional.
         * @param flag One letter flag.
         * @param name Full argument name.
         * @param description Argumant description.
         */
        cmd_builder& boolean(const std::string & flag, const std::string & name, const std::string & description) &;

        /**
         * Add boolean flag. Always optional.
         * @param flag One letter flag.
         * @param name Full argument name.
         * @param description Argumant description.
         */
        cmd_builder&& boolean(const std::string & flag, const std::string & name, const std::string & description) &&;

        /**
         * Redirects diagnostic messages to the provided ostream.
         * If ostream is nullptr, then exception will be thrown instead.
         */
        cmd_builder& set_ostream(std::ostream* os) &;

        /**
         * Redirects diagnostic messages to the provided ostream.
         * If ostream is nullptr, then exception will be thrown instead.
         */
        cmd_builder&& set_ostream(std::ostream* os) &&;

        /**
         * Parce command line
         */
        config parse(int argc, const char* const* argv);

        /**
         * Parce command line
         */
        config parse(const yato::array_view_1d<std::string> & args);

        /**
         * Parce command line
         */
        config parse(const std::vector<std::string> & args);

    private:
        std::shared_ptr<cmd_config> m_conf;
    };


    /**
     * Dispatches alternative command line parsing according to first positional argument
     */
    class cmd_dispatcher
    {
    public:
        using return_type = int;
        using alternative_handler = std::function<return_type(config)>;

        explicit
        cmd_dispatcher(const std::string & usage_description);

        cmd_dispatcher(const cmd_dispatcher&) = delete;
        cmd_dispatcher(cmd_dispatcher&&) noexcept;

        ~cmd_dispatcher();

        cmd_dispatcher& operator=(const cmd_dispatcher&) = delete;
        cmd_dispatcher& operator=(cmd_dispatcher&&) noexcept;

        /**
         * Register a command line for parsing
         * If first positional argument is 'key' then the rest of arguments are parsed as 'cmd'
         */
        cmd_dispatcher& add_alternative(std::string key, alternative_handler handler, cmd_builder cmd) &;

        /**
         * Register a command line for parsing
         * If first positional argument is 'key' then the rest of arguments are parsed as 'cmd'
         */
        cmd_dispatcher&& add_alternative(std::string key, alternative_handler handler, cmd_builder cmd) &&;

        /**
         * Redirects diagnostic messages to the provided ostream.
         * If ostream is nullptr, then exception will be thrown instead.
         */
        cmd_dispatcher& set_ostream(std::ostream* os) &;

        /**
         * Redirects diagnostic messages to the provided ostream.
         * If ostream is nullptr, then exception will be thrown instead.
         */
        cmd_dispatcher&& set_ostream(std::ostream* os) &&;

        /**
         * Set value to be returned on parsing error
         */
        cmd_dispatcher& set_error_code(return_type value) &;

        /**
         * Set value to be returned on parsing error
         */
        cmd_dispatcher&& set_error_code(return_type value) &&;

        /**
         * Parce command line
         */
        return_type parse(int argc, const char* const* argv);

        /**
         * Parce command line
         */
        return_type parse(const yato::array_view_1d<std::string> & args);

        /**
         * Parce command line
         */
        return_type parse(const std::vector<std::string> & args);

    private:
        struct cmd_dispatcher_impl;
        std::unique_ptr<cmd_dispatcher_impl> m_pimpl;
    };


} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_CMD_CMD_H_
