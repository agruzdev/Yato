/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_PATH_H_
#define _YATO_CONFIG_PATH_H_

#include <string>
#include "yato/token_iterator.h"

namespace yato {

namespace conf {

    /**
     * Returns default path separator
     */
    template <typename CharTy_>
    YATO_CONSTEXPR_FUNC
    CharTy_ path_separator() = delete;

    template <>
    YATO_CONSTEXPR_FUNC
    char path_separator<char>()
    {
        return '.';
    }

    template <>
    YATO_CONSTEXPR_FUNC
    wchar_t path_separator<wchar_t>()
    {
        return L'.';
    }


    /**
     * Immutable wrapper for config key path
     */
    template <typename CharTy_>
    class basic_path
    {
        using this_type = basic_path<CharTy_>;
    public:
        using char_type  = CharTy_;
        using value_type = CharTy_;
        using string_type = std::basic_string<char_type>;

        using iterator = yato::token_iterator<typename string_type::const_iterator>;

        static YATO_CONSTEXPR_VAR bool skips_empty_names = true;
        //--------------------------------------------------------------

    private:
        string_type m_path;
        char_type   m_separator;
        //--------------------------------------------------------------

    public:
        basic_path(string_type path, char_type separator = conf::path_separator<char_type>())
            : m_path(std::move(path)), m_separator(separator)
        { }

        basic_path(const basic_path&) = default;
        basic_path(basic_path&&) = default;

        basic_path& operator=(const basic_path&) = default;
        basic_path& operator=(basic_path&&) = default;

        ~basic_path() = default;

        iterator tokenize() const
        {
            return yato::tokenize_if(m_path.cbegin(), m_path.cend(), yato::equal_to_value<CharTy_>{ m_separator }, skips_empty_names);
        }

        const std::string & str() const
        {
            return m_path;
        }

        const char_type & separator() const
        {
            return m_separator;
        }
    };

    using path = basic_path<char>;

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_PATH_H_
