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
            const yato::optional<int64_t> & default_value  = yato::nullopt_t{});

        /**
         * Add floating-point argument.
         * @param arg_type Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder& floating(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description,
            const yato::optional<double> & default_value  = yato::nullopt_t{});

        /**
         * Add string argument.
         * @param arg_type Type of parameter, defining parsing requirements.
         * @param flag One letter flag. In the case of positional argument, flag is ignored.
         * @param name Full argument name.
         * @param description Argumant description.
         * @param default_value Default value for optional arguments, otherwise ignored.
         */
        cmd_builder& string(argument_type arg_type, const std::string & flag, const std::string & name, const std::string & description,
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

        /**
         * Parce command line
         */
        config parse(const std::vector<std::string> & args);

    private:
        std::shared_ptr<cmd_config> m_conf;
    };


} // namespace conf

} // namespace yato

#endif //_YATO_CONFIG_CMD_CMD_H_
