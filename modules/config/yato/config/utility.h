/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_UTILITY_H_
#define _YATO_CONFIG_UTILITY_H_

#include <cstring>
#include <cctype>
#include <string>
#include <limits>

#include "config.h"

namespace yato {

namespace conf {

    YATO_INLINE_VARIABLE
    constexpr const size_t nolength = std::numeric_limits<size_t>::max();


    namespace details
    {
        inline
        char* skip_spaces(char* str)
        {
            while(std::isspace(*str)) {
                ++str;
            }
            return str;
        }

        inline
        const char* skip_spaces(const char* str)
        {
            return skip_spaces(const_cast<char*>(str));
        }

        template <typename Ty_, typename Decoder_>
        bool decode(const char* str, size_t len, Ty_* dst, Decoder_ && decoder)
        {
             if (str && len > 0) {
                const char* const beg = skip_spaces(str);
                if (len == conf::nolength) {
                    len = std::strlen(beg);
                }
                else {
                    len -= (beg - str);
                }
                char* end = nullptr;
                const auto res = decoder(beg, &end);
                if(!end) {
                    return false;
                }
                if(end != beg + len) {
                    end = skip_spaces(end);
                }
                if (end == beg + len) {
                    *dst = res;
                    return true;
                }
            }
            return false;
        }

    } // namespace details

    template <stored_type STy_>
    struct serializer;

    template <>
    struct serializer<stored_type::integer>
    {
        using value_type = conf::stored_type_trait<stored_type::integer>::return_type;

        static
        std::string to_string(value_type val)
        {
            return std::to_string(val);
        }

        static
        bool from_string(const char* str, size_t len, value_type* dst)
        {
            return details::decode(str, len, dst, [](const char* str, char** str_end) { return std::strtoll(str, str_end, 0); });
        }

        static
        bool from_string(const std::string & str, value_type* dst)
        {
            return from_string(str.data(), str.size(), dst);
        }
    };

    template <>
    struct serializer<stored_type::real>
    {
        using value_type = conf::stored_type_trait<stored_type::real>::return_type;

        static
        std::string to_string(value_type val)
        {
            return std::to_string(val);
        }

        static
        bool from_string(const char* str, size_t len, value_type* dst)
        {
            return details::decode(str, len, dst, &std::strtod);
        }

        static
        bool from_string(const std::string & str, value_type* dst)
        {
            return from_string(str.data(), str.size(), dst);
        }
    };

    template <>
    struct serializer<stored_type::boolean>
    {
        using value_type = conf::stored_type_trait<stored_type::boolean>::return_type;

        static constexpr std::array<const char*, 6> true_patterns  = { "1", "yes", "y", "true",  "t", "on"  };
        static constexpr std::array<const char*, 6> false_patterns = { "0", "no",  "n", "false", "f", "off" };
        static constexpr size_t pattern_length = 6;

        static
        bool match(const char* pattern, const char* str, size_t* match_len)
        {
            YATO_REQUIRES(pattern);
            YATO_REQUIRES(str);
            const char* beg = str;
            while (*pattern) {
                if(*pattern++ != *str++) {
                    return false;
                }
            }
            if (match_len) {
                *match_len = static_cast<size_t>(str - beg);
            }
            return true;
        }

        static
        bool strtobool(const char *str, char **str_end)
        {
            char tmp[pattern_length + 1] = {};
            for (size_t i = 0; str[i] && i < pattern_length; ++i) {
                tmp[i] = static_cast<char>(std::tolower(str[i]));
            }
            size_t offset = 0;
            for (const auto & pattern : true_patterns) {
                if (match(pattern, tmp, &offset)) {
                    if (str_end) {
                        *str_end = const_cast<char*>(str + offset);
                    }
                    return true;
                }
            }
            for (const auto & pattern : false_patterns) {
                if (match(pattern, tmp, &offset)) {
                    if (str_end) {
                        *str_end = const_cast<char*>(str + offset);
                    }
                    return false;
                }
            }
            return false;
        }

        static
        std::string to_string(value_type val)
        {
            return val ? "true" : "false";
        }

        static
        bool from_string(const char* str, size_t len, value_type* dst)
        {
            return details::decode(str, len, dst, &strtobool);
        }

        static
        bool from_string(const std::string & str, value_type* dst)
        {
            return from_string(str.data(), str.size(), dst);
        }
    };


} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_UTILITY_H_
