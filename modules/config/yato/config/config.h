/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_H_
#define _YATO_CONFIG_H_

#include <cstdint>
#include <string>

#include <yato/optional.h>
#include <yato/variant.h>
#include <yato/assert.h>

namespace yato {

    class config_error
        : public yato::runtime_error
    {
    public:
        config_error(const std::string & err)
            : yato::runtime_error(err)
        { }
    
        config_error(const char* err)
            : yato::runtime_error(err)
        { }
    };


    namespace details
    {
        struct config_integral_tag {};
        struct config_floating_tag {};
        struct config_string_tag   {};
    
        template <typename Ty_, typename = void>
        struct config_choose_stored_type
        { };
    
        template <typename Ty_>
        struct config_choose_stored_type
            <
                Ty_,
                std::enable_if_t<std::is_integral<Ty_>::value>
            >
        {
            using type = int64_t;
            using tag  = config_integral_tag;
        };
    
        template <typename Ty_>
        struct config_choose_stored_type
            <
                Ty_,
                std::enable_if_t<std::is_floating_point<Ty_>::value>
            >
        {
            using type = double;
            using tag  = config_floating_tag;
        };
    
        template <typename Ty_>
        struct config_choose_stored_type
            <
                Ty_,
                std::enable_if_t<std::is_same<std::string, Ty_>::value>
            >
        {
            using type = std::string;
            using tag  = config_string_tag;
        };

    }

    
    class config_value;
    class config_array;
    class config_object;

    /**
     * Scalar value interface
     */
    class config_value
    {
    private:
        virtual void* do_get_underlying_type() noexcept = 0;
    
        virtual yato::optional<int64_t>   do_get_int()  const noexcept = 0;
        virtual yato::optional<double>    do_get_real() const noexcept = 0;
        virtual yato::optional<std::string> do_get_string() const noexcept = 0;

        template <typename Ty_>
        yato::optional<Ty_> get_impl_(details::config_integral_tag) const noexcept {
            return do_get_int().map([](int64_t val) { return yato::narrow_cast<Ty_>(val); });
        }
    
        template <typename Ty_>
        yato::optional<Ty_> get_impl_(details::config_floating_tag) const noexcept {
            return do_get_real().map([](double val) { return yato::narrow_cast<Ty_>(val); });
        }
    
        template <typename Ty_>
        yato::optional<std::string> get_impl_(details::config_string_tag) const noexcept {
            return do_get_string();
        }
    
    public:
        virtual ~config_value() = default;

        template <typename Ty_>
        yato::optional<Ty_> get_opt() const noexcept {
            using stored_tag = typename details::config_choose_stored_type<typename std::decay<Ty_>::type>::tag;
            return get_impl_<Ty_>(stored_tag{});
        }
    
        template <typename Ty_>
        Ty_ get() const {
            if(const auto opt = get_opt<Ty_>()){
                return opt.get_unsafe();
            }
            else {
                throw config_error("Invalid config_value access.");
            }
        }
    
        template <typename Ty_, typename Uy_>
        Ty_ get(Uy_ && default_value) const noexcept {
            if(const auto opt = get_opt<Ty_>()){
                return opt.get_unsafe();
            }
            else {
                return static_cast<Ty_>(std::forward<Uy_>(default_value));
            }
        }

        const void* get_underlying_type() const {
            return const_cast<config_value*>(this)->do_get_underlying_type();
        }
    
        void* get_underlying_type() {
            return do_get_underlying_type();
        }
    };


    /**
     * Set of named values
     */
    class config_object {
    private:
        virtual void* do_get_underlying_type() noexcept = 0;

        virtual size_t do_get_size() const noexcept = 0;
        virtual std::vector<std::string> do_get_keys() const noexcept = 0;
    
        virtual bool do_has_value(const std::string & key) const noexcept = 0;
        virtual bool do_has_object(const std::string & key) const noexcept = 0;
        virtual bool do_has_array(const std::string & key) const noexcept = 0;

        virtual const config_value*  do_get_value(const std::string & key) const noexcept  = 0;
        virtual const config_object* do_get_object(const std::string & key) const noexcept = 0;
        virtual const config_array*  do_get_array(const std::string & key) const noexcept = 0;
    
    public:
        virtual ~config_object() = default;
    
        bool has_value(const std::string & key) {
            return do_has_value(key);
        }
    
        bool has_object(const std::string & key) {
            return do_has_object(key);
        }
    
        bool has_array(const std::string & key) {
            return do_has_array(key);
        }
    
        const config_value* get_value(const std::string & key) const {
            return do_get_value(key);
        }
    
        const config_object* get_object(const std::string & key) const {
            return do_get_object(key);
        }
    
        const config_array* get_array(const std::string & key) const {
            return do_get_array(key);
        }
    
        const void* get_underlying_type() const {
            return const_cast<config_object*>(this)->do_get_underlying_type();
        }
    
        size_t size() const noexcept {
            return do_get_size();
        }
    
        std::vector<std::string> keys() const {
            return do_get_keys();
        }
    
        void* get_underlying_type() {
            return do_get_underlying_type();
        }
    };

    /**
     * Array interface
     */
    class config_array {
    private:
        virtual void* do_get_underlying_type() noexcept = 0;

        virtual size_t do_get_size() const noexcept = 0;
        virtual const config_value*  do_get_value(size_t idx) const noexcept  = 0;
        virtual const config_object* do_get_object(size_t idx) const noexcept = 0;
        virtual const config_array*  do_get_array(size_t idx) const noexcept  = 0;

    public:
        virtual ~config_array() = default;
    
        size_t size() const {
            return do_get_size();
        }
    
        const config_value* get_value(size_t idx) const {
            YATO_REQUIRES(idx < size());
            return do_get_value(idx);
        }
    
        const config_object* get_object(size_t idx) const {
            YATO_REQUIRES(idx < size());
            return do_get_object(idx);
        }
    
        const config_array* get_array(size_t idx) const {
            YATO_REQUIRES(idx < size());
            return do_get_array(idx);
        }
    
        const void* get_underlying_type() const {
            return const_cast<config_array*>(this)->do_get_underlying_type();
        }
    
        void* get_underlying_type() {
            return do_get_underlying_type();
        }
    };

} // namespace yato

#endif // _YATO_CONFIG_H_
