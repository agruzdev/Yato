/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2018 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_OPERATIONS_H_
#define _YATO_CONFIG_OPERATIONS_H_

#include "config.h"

namespace yato {

namespace conf {


    namespace details
    {

        /**
         * Union implementation
         * Left is always high priority
         */
        class union_backend
            : public config_backend
        {
        private:
            backend_ptr m_left;
            backend_ptr m_right;

        public:
            union_backend(const backend_ptr & left, const backend_ptr & right)
                : m_left(left), m_right(right)
            {
                YATO_REQUIRES(m_left  != nullptr);
                YATO_REQUIRES(m_right != nullptr);
                if(!(m_left->is_object() && m_right->is_object())) {
                    throw yato::argument_error("Config union can be created only for two objects");
                }
            }

            ~union_backend() override = default;

            union_backend(const union_backend&) = default;
            union_backend(union_backend&&) noexcept = default;

            union_backend& operator=(const union_backend&) = default;
            union_backend& operator=(union_backend&&) noexcept = default;

            bool is_object() const noexcept override
            {
                return m_left->is_object();
            }

            stored_variant get_by_name(const std::string & name, config_type type) const noexcept override
            {
                const auto left_res = m_left->get_by_name(name, type);
                if(!left_res.is_type<void>()) {
                    return left_res;
                }
                else {
                    return m_right->get_by_name(name, type);
                }
            }
            
            std::vector<std::string> keys() const noexcept override
            {
                auto left_keys = m_left->keys();
                auto right_keys = m_right->keys();
                std::sort(left_keys.begin(), left_keys.end());
                std::sort(right_keys.begin(), right_keys.end());
                std::vector<std::string> res;
                res.reserve(left_keys.size() + right_keys.size());
                std::set_union(left_keys.cbegin(), left_keys.cend(), right_keys.cbegin(), right_keys.cend(), std::back_inserter(res));
                return res;
            }

            bool is_array() const noexcept override
            {
                return false;
            }

            stored_variant get_by_index(size_t index, config_type type) const noexcept override
            {
                YATO_MAYBE_UNUSED(index);
                YATO_MAYBE_UNUSED(type);
                return stored_variant{};
            }

            size_t size() const noexcept override
            {
                return 0;
            }
        };

        /**
         * Intersection implementation
         * Left is always high priority
         */
        class intersection_backend
            : public config_backend
        {
        private:
            backend_ptr m_left;
            backend_ptr m_right;

        public:
            intersection_backend(const backend_ptr & left, const backend_ptr & right)
                : m_left(left), m_right(right)
            {
                YATO_REQUIRES(m_left  != nullptr);
                YATO_REQUIRES(m_right != nullptr);
                if(!(m_left->is_object() && m_right->is_object())) {
                    throw yato::argument_error("Config union can be created only for two objects");
                }
            }

            ~intersection_backend() override = default;

            intersection_backend(const intersection_backend&) = default;
            intersection_backend(intersection_backend&&) noexcept = default;

            intersection_backend& operator=(const intersection_backend&) = default;
            intersection_backend& operator=(intersection_backend&&) noexcept = default;

            bool is_object() const noexcept override
            {
                return m_left->is_object();
            }

            stored_variant get_by_name(const std::string & name, config_type type) const noexcept override
            {
                const auto left_res  = m_left->get_by_name(name, type);
                const auto right_res = m_right->get_by_name(name, type);
                if(!left_res.is_type<void>() && !right_res.is_type<void>()) {
                    return left_res;
                }
                return stored_variant{};
            }

            std::vector<std::string> keys() const noexcept override
            {
                auto left_keys = m_left->keys();
                auto right_keys = m_right->keys();
                std::sort(left_keys.begin(), left_keys.end());
                std::sort(right_keys.begin(), right_keys.end());
                std::vector<std::string> res;
                res.reserve(std::max(left_keys.size(), right_keys.size()));
                std::set_intersection(left_keys.cbegin(), left_keys.cend(), right_keys.cbegin(), right_keys.cend(), std::back_inserter(res));
                return res;
            }

            bool is_array() const noexcept override
            {
                return false;
            }

            stored_variant get_by_index(size_t index, config_type type) const noexcept override
            {
                YATO_MAYBE_UNUSED(index);
                YATO_MAYBE_UNUSED(type);
                return stored_variant{};
            }

            size_t size() const noexcept override
            {
                return 0;
            }
        };


        /**
         * Implemets arrays concatination
         */
        class concatenation_backend
            : public config_backend
        {
        private:
            backend_ptr m_left;
            backend_ptr m_right;

        public:
            concatenation_backend(const backend_ptr & left, const backend_ptr & right)
                : m_left(left), m_right(right)
            {
                YATO_REQUIRES(m_left  != nullptr);
                YATO_REQUIRES(m_right != nullptr);
                if(!(m_left->is_array() && m_right->is_array())) {
                    throw yato::argument_error("Config concatination can be created only for two arrays");
                }
            }

            ~concatenation_backend() override = default;

            concatenation_backend(const concatenation_backend&) = default;
            concatenation_backend(concatenation_backend&&) noexcept = default;

            concatenation_backend& operator=(const concatenation_backend&) = default;
            concatenation_backend& operator=(concatenation_backend&&) noexcept = default;

            bool is_object() const noexcept override
            {
                return false;
            }

            stored_variant get_by_name(const std::string & name, config_type type) const noexcept override
            {
                YATO_MAYBE_UNUSED(name);
                YATO_MAYBE_UNUSED(type);
                return stored_variant{};
            }

            std::vector<std::string> keys() const noexcept override
            {
                return std::vector<std::string>{};
            }

            bool is_array() const noexcept override
            {
                return true;
            }

            stored_variant get_by_index(size_t index, config_type type) const noexcept override
            {
                const size_t offset = m_left->size();
                if(index < offset) {
                    return m_left->get_by_index(index, type);
                }
                else {
                    return m_right->get_by_index(index - offset, type);
                }
            }

            size_t size() const noexcept override
            {
                return m_left->size() + m_right->size();
            }
        };
    }

    /**
     * Type used to define arguments priority in config operations
     */
    enum class priority
    {
        left,
        right
    };

    /**
     * Creates a union of two configurations
     * Requested key will be found if and only if it is presented in the left or right sub-config.
     * If the requested key is presented in the both sub-configs, then the value is choosen according to the priority flag.
     * 
     * Union can be applied only to objects.
     */
    inline
    config object_union(const config & left, const config & right, priority p = priority::left)
    {
        backend_ptr left_backend  = left.get_backend();
        backend_ptr right_backend = right.get_backend();
        if(left_backend != nullptr && right_backend != nullptr) {
            if(p != priority::left) {
                left_backend.swap(right_backend);
            }
            return config(std::make_shared<details::union_backend>(left_backend, right_backend));
        }
        if(left_backend != nullptr) {
            return left;
        }
        if(right_backend != nullptr) {
            return right;
        }
        return config{};
    }

    /**
     * Creates an intersection of two configurations
     * Requested key will be found if and only if it is found in the both sub-config.
     * The value is choosen according to the priority flag.
     * 
     * Intersection can be applied only to objects.
     */
    inline
    config object_intersection(const config & left, const config & right, priority p = priority::left)
    {
        backend_ptr left_backend  = left.get_backend();
        backend_ptr right_backend = right.get_backend();
        if(left_backend == nullptr || right_backend == nullptr) {
            return config{};
        }
        if(p != priority::left) {
            left_backend.swap(right_backend);
        }
        return config(std::make_shared<details::intersection_backend>(left_backend, right_backend));
    }

    /**
     * Creates concatination of two config arrays
     */
    inline
    config array_cat(const config & left, const config & right)
    {
        backend_ptr left_backend  = left.get_backend();
        backend_ptr right_backend = right.get_backend();
        if(left_backend != nullptr && right_backend != nullptr) {
            return config(std::make_shared<details::concatenation_backend>(left_backend, right_backend));
        }
        if(left_backend != nullptr) {
            return left;
        }
        if(right_backend != nullptr) {
            return right;
        }
        return config{};
    }

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_OPERATIONS_H_
