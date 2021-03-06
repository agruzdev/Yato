/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */


#include "utility.h"

#include <cstring>
#include <istream>

#include "yato/variant_match.h"


namespace yato {

namespace conf {

    stored_type get_type(const stored_variant& var)
    {
        switch (var.type_index()) {
        case yato::meta::list_find<stored_variant::alternativies_list, stored_type_trait<stored_type::integer>::return_type>::value:
            return stored_type::integer;
        case yato::meta::list_find<stored_variant::alternativies_list, stored_type_trait<stored_type::real>::return_type>::value:
            return stored_type::real;
        case yato::meta::list_find<stored_variant::alternativies_list, stored_type_trait<stored_type::boolean>::return_type>::value:
            return stored_type::boolean;
        case yato::meta::list_find<stored_variant::alternativies_list, stored_type_trait<stored_type::string>::return_type>::value:
            return stored_type::string;
        case yato::meta::list_find<stored_variant::alternativies_list, stored_type_trait<stored_type::config>::return_type>::value:
            return stored_type::config;
        default:
            YATO_ASSERT(false, "conf::get_type: Unexpected variant type");
            return stored_type::config;
        }
    }


    value_converter& value_converter::instance()
    {
        static value_converter ins;
        return ins;
    }

    template <stored_type DstType_>
    stored_variant cvt_from_string_(const stored_variant& src)
    {
        using string_t = typename stored_type_trait<stored_type::string>::return_type;
        YATO_REQUIRES(src.is_type<string_t>());
        stored_variant dst;
        if (!cvt_from<DstType_>(src.get<string_t>(), dst)) {
            dst = src;
        }
        return dst;
    }

    template <stored_type DstType_>
    stored_variant cvt_to_string_(const stored_variant& src)
    {
        using dst_t    = typename stored_type_trait<DstType_>::return_type;
        using string_t = typename stored_type_trait<stored_type::string>::return_type;
        YATO_REQUIRES(src.is_type<dst_t>());
        return stored_variant(yato::in_place_type_t<string_t>{}, serializer<DstType_>::to_string(src.get<dst_t>()));
    }

    template <stored_type DstType_>
    stored_variant cvt_identity_(const stored_variant& src)
    {
        return src;
    }

    value_converter::value_converter() {
        using integer_t = stored_type_trait<stored_type::integer>::return_type;
        using real_t    = stored_type_trait<stored_type::real>::return_type;
        using boolean_t = stored_type_trait<stored_type::boolean>::return_type;

        m_cvt_functions[std::make_pair(stored_type::integer, stored_type::integer)] = &cvt_identity_<stored_type::integer>;
        m_cvt_functions[std::make_pair(stored_type::real,    stored_type::real)]    = &cvt_identity_<stored_type::real>;
        m_cvt_functions[std::make_pair(stored_type::boolean, stored_type::boolean)] = &cvt_identity_<stored_type::boolean>;
        m_cvt_functions[std::make_pair(stored_type::string,  stored_type::string)]  = &cvt_identity_<stored_type::string>;
        m_cvt_functions[std::make_pair(stored_type::config,  stored_type::config)]  = &cvt_identity_<stored_type::config>;

        m_cvt_functions[std::make_pair(stored_type::integer, stored_type::string)] = &cvt_from_string_<stored_type::integer>;
        m_cvt_functions[std::make_pair(stored_type::real,    stored_type::string)] = &cvt_from_string_<stored_type::real>;
        m_cvt_functions[std::make_pair(stored_type::boolean, stored_type::string)] = &cvt_from_string_<stored_type::boolean>;

        m_cvt_functions[std::make_pair(stored_type::string, stored_type::integer)] = &cvt_to_string_<stored_type::integer>;
        m_cvt_functions[std::make_pair(stored_type::string, stored_type::real)]    = &cvt_to_string_<stored_type::real>;
        m_cvt_functions[std::make_pair(stored_type::string, stored_type::boolean)] = &cvt_to_string_<stored_type::boolean>;

        m_cvt_functions[std::make_pair(stored_type::integer, stored_type::real)] = [](const stored_variant& src) {
            YATO_REQUIRES(src.is_type<real_t>());
            const auto real_val = src.get<real_t>();
            const auto int_val = static_cast<integer_t>(real_val);
            stored_variant dst;
            if (real_val == static_cast<real_t>(int_val)) {
                dst.emplace<integer_t>(int_val);
            }
            return dst;
        };

        m_cvt_functions[std::make_pair(stored_type::integer, stored_type::boolean)] = [](const stored_variant& src) {
            YATO_REQUIRES(src.is_type<boolean_t>());
            return stored_variant(yato::in_place_type_t<integer_t>{}, static_cast<integer_t>(src.get<boolean_t>()));
        };

        m_cvt_functions[std::make_pair(stored_type::real, stored_type::integer)] = [](const stored_variant& src) {
            YATO_REQUIRES(src.is_type<integer_t>());
            return stored_variant(yato::in_place_type_t<real_t>{}, static_cast<real_t>(src.get<integer_t>()));
        };

        m_cvt_functions[std::make_pair(stored_type::boolean, stored_type::integer)] = [](const stored_variant& src) {
            YATO_REQUIRES(src.is_type<integer_t>());
            return stored_variant(yato::in_place_type_t<boolean_t>{}, static_cast<boolean_t>(src.get<integer_t>()));
        };
    }


    const value_converter::cvt_funtion_t* value_converter::dispatch(stored_type dst_type, stored_type src_type) const 
    {
        const auto it = m_cvt_functions.find(std::make_pair(dst_type, src_type));
        return (it != m_cvt_functions.cend()) ? &(*it).second : nullptr;
    }

    stored_variant value_converter::apply(stored_type dst_type, const stored_variant& src) const
    {
        stored_variant dst;
        if (src) {
            const auto cvt_function = dispatch(dst_type, get_type(src));
            if (cvt_function) {
                dst = (*cvt_function)(src);
            }
        };
        return dst;
    }


    std::string get_text_stream_content(std::istream& is)
    {
        const std::streampos start = is.tellg();
        is.seekg(0, is.end);
        const size_t len = yato::narrow_cast<size_t>(is.tellg() - start);
        is.seekg(start);
        std::string result(len, '\0');
        is.read(&result[0], len);
        return result;
    }


} // namespace conf

} // namespace yato

