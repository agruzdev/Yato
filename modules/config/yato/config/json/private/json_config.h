/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_JSON_PRIVATE_JSON_CONFIG_H_
#define _YATO_CONFIG_JSON_PRIVATE_JSON_CONFIG_H_

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
    class json_config final
        : public config_backend
    {
    public:
        using njson = NJson_;
        using njson_citerator  = typename NJson_::const_iterator;
        using njson_creference = typename NJson_::const_reference;


        json_config(std::shared_ptr<NJson_> json);

        json_config(std::shared_ptr<NJson_> root, njson_citerator iter);

        json_config(const json_config&) = delete;

        json_config(json_config&&) = delete;

        ~json_config();

        json_config& operator=(const json_config&) = delete;

        json_config& operator=(json_config&&) = delete;

        size_t do_size() const noexcept override;

        find_index_result_t do_find(size_t index) const override;

        void do_release(const config_value* val) const noexcept override;

        bool do_has_property(config_property p) const noexcept override;

        find_key_result_t do_find(const std::string& name) const override;

    private:
        njson_creference get_() const;

        std::shared_ptr<njson> m_root;
        yato::optional<njson_citerator> m_iter;
    };


} //namespace json

} //namespace conf

} //namespace yato

#endif // _YATO_CONFIG_JSON_PRIVATE_JSON_CONFIG_H_
