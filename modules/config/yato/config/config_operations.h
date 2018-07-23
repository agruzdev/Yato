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

    /**
     * Type used to define arguments priority in config operations
     */
    enum class priority
    {
        left,
        right
    };

    namespace details
    {

        /**
         * Union implementation
         */
        class union_backend
            : public config_backend
        {
        private:
            backend_ptr m_left;
            backend_ptr m_right;
            priority m_priority;

        public:
            union_backend(const backend_ptr & left, const backend_ptr & right, priority p)
                : m_left(left), m_right(right), m_priority(p)
            {
                YATO_REQUIRES(m_left  != nullptr);
                YATO_REQUIRES(m_right != nullptr);
                if(!(m_left->do_is_object() && m_right->do_is_object())) {
                    throw yato::argument_error("Config union can be created only for two objects");
                }
            }

            ~union_backend() override = default;

            union_backend(const union_backend&) = default;
            union_backend(union_backend&&) noexcept = default;

            union_backend& operator=(const union_backend&) = default;
            union_backend& operator=(union_backend&&) noexcept = default;

            bool do_is_object() const noexcept override
            {
                return m_left->do_is_object();
            }

            stored_variant do_get_by_name(const std::string & name, config_type type) const noexcept override
            {
                if(m_priority == priority::left){
                    const auto left_res = m_left->do_get_by_name(name, type);
                    if(!left_res.is_type<void>()) {
                        return left_res;
                    }
                    else {
                        return m_right->do_get_by_name(name, type);
                    }
                }
                else {
                    const auto right_res = m_right->do_get_by_name(name, type);
                    if(!right_res.is_type<void>()) {
                        return right_res;
                    }
                    else {
                        return m_left->do_get_by_name(name, type);
                    }
                }
            }

            bool do_is_array() const noexcept override
            {
                return false;
            }

            stored_variant do_get_by_index(size_t index, config_type type) const noexcept override
            {
                YATO_MAYBE_UNUSED(index);
                YATO_MAYBE_UNUSED(type);
                return stored_variant{};
            }

            size_t do_get_size() const noexcept override
            {
                return 0;
            }
        };

        /**
         * Intersection implementation
         */
        class intersection_backend
            : public config_backend
        {
        private:
            backend_ptr m_left;
            backend_ptr m_right;
            priority m_priority;

        public:
            intersection_backend(const backend_ptr & left, const backend_ptr & right, priority p)
                : m_left(left), m_right(right), m_priority(p)
            {
                YATO_REQUIRES(m_left  != nullptr);
                YATO_REQUIRES(m_right != nullptr);
                if(!(m_left->do_is_object() && m_right->do_is_object())) {
                    throw yato::argument_error("Config union can be created only for two objects");
                }
            }

            ~intersection_backend() override = default;

            intersection_backend(const intersection_backend&) = default;
            intersection_backend(intersection_backend&&) noexcept = default;

            intersection_backend& operator=(const intersection_backend&) = default;
            intersection_backend& operator=(intersection_backend&&) noexcept = default;

            bool do_is_object() const noexcept override
            {
                return m_left->do_is_object();
            }

            stored_variant do_get_by_name(const std::string & name, config_type type) const noexcept override
            {
                const auto left_res  = m_left->do_get_by_name(name, type);
                const auto right_res = m_right->do_get_by_name(name, type);
                if(!left_res.is_type<void>() && !right_res.is_type<void>()) {
                    return m_priority == priority::left ? left_res : right_res;
                }
                return stored_variant{};
            }

            bool do_is_array() const noexcept override
            {
                return false;
            }

            stored_variant do_get_by_index(size_t index, config_type type) const noexcept override
            {
                YATO_MAYBE_UNUSED(index);
                YATO_MAYBE_UNUSED(type);
                return stored_variant{};
            }

            size_t do_get_size() const noexcept override
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
                if(!(m_left->do_is_array() && m_right->do_is_array())) {
                    throw yato::argument_error("Config concatination can be created only for two arrays");
                }
            }

            ~concatenation_backend() override = default;

            concatenation_backend(const concatenation_backend&) = default;
            concatenation_backend(concatenation_backend&&) noexcept = default;

            concatenation_backend& operator=(const concatenation_backend&) = default;
            concatenation_backend& operator=(concatenation_backend&&) noexcept = default;

            bool do_is_object() const noexcept override
            {
                return false;
            }

            stored_variant do_get_by_name(const std::string & name, config_type type) const noexcept override
            {
                YATO_MAYBE_UNUSED(name);
                YATO_MAYBE_UNUSED(type);
                return stored_variant{};
            }

            bool do_is_array() const noexcept override
            {
                return true;
            }

            stored_variant do_get_by_index(size_t index, config_type type) const noexcept override
            {
                const size_t offset = m_left->do_get_size();
                if(index < offset) {
                    return m_left->do_get_by_index(index, type);
                }
                else {
                    return m_right->do_get_by_index(index - offset, type);
                }
            }

            size_t do_get_size() const noexcept override
            {
                return m_left->do_get_size() + m_right->do_get_size();
            }
        };
    }

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
            return config(std::make_shared<details::union_backend>(left_backend, right_backend, p));
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
        return config(std::make_shared<details::intersection_backend>(left_backend, right_backend, p));
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
