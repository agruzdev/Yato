/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_OBJECT_H_
#define _YATO_CONFIG_MANUAL_OBJECT_H_

#include "manual_value.h"

namespace yato {

    class manual_array;
    struct manual_object_impl;

    class manual_object
        : public config_object
    {
    private:
        std::unique_ptr<manual_object_impl> m_pimpl;

        size_t do_get_size() const noexcept override;
        std::vector<std::string> do_get_keys() const noexcept override;
    
        bool do_has_value(const std::string & key) const noexcept override;
        bool do_has_object(const std::string & key) const noexcept override;
        bool do_has_array(const std::string & key) const noexcept override;

        const config_value*  do_get_value(const std::string & key) const noexcept override;
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
    
        void put(const std::string & key, const manual_value & val);
    
        void put(const std::string & key, manual_value && val);
    
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

#endif // _YATO_CONFIG_MANUAL_OBJECT_H_
