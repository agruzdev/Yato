/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_MANUAL_ARRAY_H_
#define _YATO_CONFIG_MANUAL_ARRAY_H_

#include "../config.h"

namespace yato {

namespace conf {

    class manual_object;
    struct manual_array_impl;

    class manual_array
        : public config_array
    {
    private:
        std::unique_ptr<manual_array_impl> m_pimpl;

        size_t do_get_size() const noexcept override;
        config_value do_get_value(size_t idx) const noexcept override;
        const config_object* do_get_object(size_t idx) const noexcept override;
        const config_array* do_get_array(size_t idx) const noexcept override;
        void* do_get_underlying_type() noexcept override;

    public:
        manual_array();
        ~manual_array();
    
        manual_array(const manual_array&);
        manual_array(manual_array&&) noexcept;
    
        manual_array& operator =(const manual_array&);
        manual_array& operator =(manual_array&&) noexcept;
    
        void resize(size_t size);
    
        void put(size_t idx, const config_value & val);
    
        void put(size_t idx, config_value && val);
    
        void put(size_t idx, const manual_array & arr);
    
        void put(size_t idx, manual_array && arr);
    
        void put(size_t idx, const manual_object & arr);
    
        void put(size_t idx, manual_object && arr);
    
        template <typename Ty_>
        void append(Ty_ && obj) {
            const size_t len = do_get_size();
            resize(len + 1);
            put(len, std::forward<Ty_>(obj));
        }
    
        void swap(manual_array & other) noexcept;
    };

    inline
    void swap(manual_array & lhs, manual_array & rhs) noexcept {
        lhs.swap(rhs);
    }

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_MANUAL_ARRAY_H_
