/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_OBJECT_H_
#define _YATO_CONFIG_MANUAL_OBJECT_H_

#include "../config.h"

namespace yato {

namespace conf {

    class manual_array;
    struct manual_object_impl;

    class manual_object
        : public config_object
    {
    private:
        std::unique_ptr<manual_object_impl> m_pimpl;

        std::vector<std::string> do_get_keys() const noexcept override;

        yato::optional<config_value> do_get_value(const std::string & key) const noexcept override;
        const config_object* do_get_object(const std::string & key) const noexcept override;
        const config_array*  do_get_array(const std::string & key) const noexcept override;
    
        void* do_get_underlying_type() noexcept override;

    public:
        manual_object();
        ~manual_object();
    
        manual_object(const manual_object&);
        manual_object(manual_object&&) noexcept;
    
        manual_object& operator =(const manual_object&);
        manual_object& operator =(manual_object&&) noexcept;
    
        void put(const std::string & key, const config_value & val);
    
        void put(const std::string & key, config_value && val);
    
        void put(const std::string & key, const manual_array & val);
    
        void put(const std::string & key, manual_array && val);
    
        void put(const std::string & key, const manual_object & val);
    
        void put(const std::string & key, manual_object && val);
    
        void swap(manual_object & other) noexcept;
    };

    inline
    void swap(manual_object & lhs, manual_object & rhs) noexcept {
        lhs.swap(rhs);
    }

} // namespace yato

} // namespace conf

#endif // _YATO_CONFIG_MANUAL_OBJECT_H_
