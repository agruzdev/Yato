/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_JSON_PRIVATE_JSON_VALUE_H_
#define _YATO_CONFIG_JSON_PRIVATE_JSON_VALUE_H_


#include "yato/prerequisites.h"

YATO_PRAGMA_WARNING_PUSH
YATO_CLANG_WARNING_IGNORE("-Wtautological-overlap-compare")
#include <nlohmann/json.hpp>
YATO_PRAGMA_WARNING_POP

#include "../../config_backend.h"

namespace yato {

namespace conf {

namespace json {


    template <typename NJson_ = nlohmann::json>
    class json_value final
        : public config_value
    {
    public:
        using njson = NJson_;
        using njson_citerator  = typename NJson_::const_iterator;
        using njson_creference = typename NJson_::const_reference;


        json_value(std::shared_ptr<njson> root, njson_citerator iter);

        json_value(const json_value&) = delete;
        json_value(json_value&&) = delete;

        json_value& operator=(const json_value&) = delete;
        json_value& operator=(json_value&&) = delete;

        ~json_value();

        stored_type type() const noexcept override;

        stored_variant get() const noexcept override;

    private:
        std::shared_ptr<njson> m_root;
        njson_citerator m_iter;
    };

} //namespace json

} //namespace conf

} //namespace yato

#endif //_YATO_CONFIG_JSON_PRIVATE_JSON_VALUE_H_
