/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#ifndef _YATO_CONFIG_OPERATIONS_H_
#define _YATO_CONFIG_OPERATIONS_H_

#include <algorithm>
#include <set>
#include <vector>

#include "config.h"
#include "yato/assertion.h"

namespace yato {

namespace conf {

    enum class filter_mode
    {
        whitelist,
        blacklist
    };


    namespace details
    {
        template <typename Ty1_, typename Ty2_, typename Pred_>
        void erase2_if(std::vector<Ty1_> & vec1, std::vector<Ty2_> & vec2, const Pred_ & predicate)
        {
            YATO_REQUIRES(vec1.size() == vec2.size());
            size_t size = vec1.size();
            size_t src_idx = 0, dst_idx = 0;
            for (; src_idx < size; ++src_idx) {
                if (!predicate(vec1[src_idx])) {
                    if (dst_idx != src_idx) {
                        vec1[dst_idx] = std::move(vec1[src_idx]);
                        vec2[dst_idx] = std::move(vec2[src_idx]);
                    }
                    ++dst_idx;
                }
            }
            vec1.erase(std::next(vec1.cbegin(), dst_idx), vec1.cend());
            vec2.erase(std::next(vec2.cbegin(), dst_idx), vec2.cend());
        }



        class config_value_wrapper
            : public config_value
        {
        public:
            static
            config_backend::key_value_t wrap_value(const backend_ptr& backend, config_backend::key_value_t kv)
            {
                if (kv.second) {
                    kv.second = new config_value_wrapper(backend, kv.second);
                }
                return kv;
            }

            config_value_wrapper(backend_ptr backend, config_value* value)
                : m_backend(std::move(backend)), m_value(value)
            {
                YATO_REQUIRES(m_backend != nullptr);
                YATO_REQUIRES(m_value != nullptr);
            }

            ~config_value_wrapper() override
            {
                m_backend->release(m_value);
            }

            stored_type type() const noexcept override
            {
                return m_value->type();
            }

            stored_variant get() const noexcept override
            {
                return m_value->get();
            }

            bool next() noexcept override
            {
                return m_value->next();
            }


            config_value_wrapper(const config_value_wrapper&) = delete;
            config_value_wrapper(config_value_wrapper&&) = delete;

            config_value_wrapper& operator=(const config_value_wrapper&) = delete;
            config_value_wrapper& operator=(config_value_wrapper&&) = delete;

        private:
            backend_ptr m_backend;
            config_value* m_value;
        };



        class joined_named_value
            : public config_value
        {
        public:
            joined_named_value(backend_ptr left_conf, backend_ptr right_conf, std::string key)
                : m_left(std::move(left_conf)), m_right(std::move(right_conf)), m_key(std::move(key))
            {
                m_value = m_left->find(m_key).second;
                if (!m_value) {
                    m_is_right_value = true;
                    m_value = m_right->find(m_key).second;
                }
            }

            ~joined_named_value() override
            {
                if (m_value) {
                    if (m_is_right_value) {
                        m_right->release(m_value);
                    }
                    else {
                        m_left->release(m_value);
                    }
                }
            }

            stored_type type() const noexcept override
            {
                return m_value->type();
            }

            stored_variant get() const noexcept override
            {
                return m_value->get();
            }

            bool next() noexcept override
            {
                if (m_value->next()) {
                    return true;
                }
                if (!m_is_right_value) {
                    m_is_right_value = true;
                    m_value = m_right->find(m_key).second;
                    return (m_value != nullptr);
                }
                return false;
            }

            operator bool() const
            {
                return m_value != nullptr;
            }


            joined_named_value(const joined_named_value&) = delete;
            joined_named_value(joined_named_value&&) = delete;

            joined_named_value& operator=(const joined_named_value&) = delete;
            joined_named_value& operator=(joined_named_value&&) = delete;

        private:
            backend_ptr m_left;
            backend_ptr m_right;
            std::string m_key;
            config_value* m_value = nullptr;
            bool m_is_right_value = false;
        };



        /**
         * Join implementation for arrays
         * Left is always high priority
         */
        class joined_array
            : public config_backend
        {
        public:

            joined_array(const backend_ptr& left, const backend_ptr& right)
                : m_left(left), m_right(right)
            {
                YATO_REQUIRES(m_left  != nullptr);
                YATO_REQUIRES(m_right != nullptr);

                YATO_REQUIRES(!m_left->is_object());
                YATO_REQUIRES(!m_right->is_object());

                m_left_size = m_left->size();
                m_size = m_left_size + m_right->size();
            }

            ~joined_array() override = default;


            bool do_is_object() const noexcept override
            {
                return false;
            }

            key_value_t do_find(size_t index) const noexcept override
            {
                key_value_t res = config_backend::novalue;
                if (index < m_left_size) {
                    res = config_value_wrapper::wrap_value(m_left, m_left->find(index));
                }
                else if(index < m_size) {
                    res = config_value_wrapper::wrap_value(m_right, m_right->find(index - m_left_size));
                }
                return res;
            }

            size_t do_size() const noexcept override
            {
                return m_size;
            }

            void do_release(const config_value* val) const noexcept override
            {
                YATO_REQUIRES(dynamic_cast<const config_value_wrapper*>(val) != nullptr);
                delete val;
            }


            joined_array(const joined_array&) = delete;
            joined_array(joined_array&&) noexcept = delete;

            joined_array& operator=(const joined_array&) = delete;
            joined_array& operator=(joined_array&&) noexcept = delete;

        private:
            backend_ptr m_left;
            backend_ptr m_right;
            size_t m_left_size = 0;
            size_t m_size = 0;
        };


        /**
         * Join implementation for objects
         * Left is always high priority
         */
        class joined_object
            : public config_backend
        {
        public:
            joined_object(const backend_ptr & left, const backend_ptr & right)
                : m_left(left), m_right(right)
            {
                YATO_REQUIRES(m_left  != nullptr);
                YATO_REQUIRES(m_right != nullptr);

                YATO_REQUIRES(m_left->is_object());
                YATO_REQUIRES(m_right->is_object());

                std::vector<std::string> left_keys  = m_left->keys();
                std::vector<std::string> right_keys = m_right->keys();

                m_keys.reserve(m_left->size());

                const auto left_set = std::set<std::string>(left_keys.cbegin(), left_keys.cend());

                for (auto& lkey : left_keys) {
                    m_keys.push_back(std::move(lkey));
                }

                for (auto& rkey : right_keys) {
                    if (left_set.find(rkey) == left_set.cend()) {
                        m_keys.push_back(std::move(rkey));
                    }
                }
            }

            ~joined_object() override = default;

            joined_object(const joined_object&) = default;
            joined_object(joined_object&&) noexcept = default;

            joined_object& operator=(const joined_object&) = default;
            joined_object& operator=(joined_object&&) noexcept = default;

            bool do_is_object() const noexcept override
            {
                return true;
            }

            key_value_t do_find(const std::string & name) const noexcept override
            {
                key_value_t res = config_backend::novalue;
                auto joined = std::make_unique<details::joined_named_value>(m_left, m_right, name);
                if (*joined) {
                    res.first  = name;
                    res.second = joined.release();
                }
                return res;
            }

            std::vector<std::string> do_keys() const noexcept override
            {
                return m_keys;
            }

            key_value_t do_find(size_t index) const noexcept override
            {
                return (index < m_keys.size())
                    ? joined_object::do_find(m_keys[index])
                    : config_backend::novalue;
            }

            size_t do_size() const noexcept override
            {
                return m_keys.size();
            }

            void do_release(const config_value* val) const noexcept override
            {
                YATO_REQUIRES(dynamic_cast<const details::joined_named_value*>(val) != nullptr);
                delete val;
            }

        private:
            backend_ptr m_left;
            backend_ptr m_right;
            std::vector<std::string> m_keys;
        };

        /**
         * Intersection implementation
         * Left is always high priority
         */
        class filter_backend
            : public config_backend
        {
        private:
            backend_ptr m_conf;
            std::vector<std::string> m_keys;
            std::vector<size_t> m_indexes;
            filter_mode m_mode;

        public:
            filter_backend(const backend_ptr & conf, const std::vector<std::string> & filter_keys, filter_mode mode)
                : m_conf(conf), m_mode(mode)
            {
                YATO_REQUIRES(m_conf != nullptr);
                if(!conf->is_object()) {
                    throw yato::argument_error("Config filter can be created only for an objects");
                }

                m_keys = m_conf->keys();
                m_indexes.resize(m_keys.size());
                std::iota(m_indexes.begin(), m_indexes.end(), static_cast<size_t>(0));

                const auto filter_set = std::set<std::string>(std::make_move_iterator(filter_keys.cbegin()), std::make_move_iterator(filter_keys.cend()));

                switch(m_mode) {
                    case filter_mode::whitelist:
                        erase2_if(m_keys, m_indexes, [&](const std::string & k){ return filter_set.find(k) == filter_set.cend(); });
                        break;
                    case filter_mode::blacklist:
                        erase2_if(m_keys, m_indexes, [&](const std::string & k){ return filter_set.find(k) != filter_set.cend(); });
                        break;
                }
            }

            ~filter_backend() override = default;

            filter_backend(const filter_backend&) = default;
            filter_backend(filter_backend&&) noexcept = default;

            filter_backend& operator=(const filter_backend&) = default;
            filter_backend& operator=(filter_backend&&) noexcept = default;

            bool do_is_object() const noexcept override
            {
                return true;
            }

            // ToDo (gruzdev.a): Make at least log(N) check
            key_value_t do_find(const std::string & name) const noexcept override
            {
                key_value_t res{};
                const auto it = std::find(m_keys.cbegin(), m_keys.cend(), name);
                if (it != m_keys.cend()) {
                    res = m_conf->find(name);
                }
                return res;
            }

            std::vector<std::string> do_keys() const noexcept override
            {
                return m_keys;
            }

            key_value_t do_find(size_t index) const noexcept override
            {
                key_value_t res{};
                const auto it = std::lower_bound(m_indexes.cbegin(), m_indexes.cend(), index);
                if ((it != m_indexes.cend()) && (*it == index)) {
                    const size_t orig_idx = m_indexes[std::distance(m_indexes.cbegin(), it)];
                    res = m_conf->find(orig_idx);
                }
                return res;
            }

            size_t do_size() const noexcept override
            {
                return m_keys.size();
            }

            void do_release(const config_value* val) const noexcept override
            {
                m_conf->release(val);
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
     * Joins two configurations. 
     * If configurations have no same keys, then the join result will be equvalent to their concatenation (in priority order),
     * otherwise the result will be equal to the first configuration (in priority order) with added unique keys from the second configuration.
     * In other words, the first configuration always remainds unchanged.
     * 
     * Join can be applied to two objects or two arrays.
     *
     * Examples:
     *  Join( {'a', 'b'}, {'c'}) = {'a', 'b', 'c'}
     *  Join( {'a', 'b'}, {'c', 'b'}) = {'a', 'b', 'c'}
     *  Join( {'a', 'b'}, {'b'}) = {'a', 'b'}
     *  Join( { }, {'b'}) = {'b'}
     *  Join( { }, { }) = { }
     */
    inline
    config join(const config& left, const config& right, priority p = priority::left)
    {
        backend_ptr left_backend  = left.get_backend();
        backend_ptr right_backend = right.get_backend();
        if (left_backend != nullptr && right_backend != nullptr) {
            if (p != priority::left) {
                left_backend.swap(right_backend);
            }

            const bool left_is_object  = left_backend->is_object();
            const bool right_is_object = right_backend->is_object();

            if (left_is_object && right_is_object) {
                return config(std::make_shared<details::joined_object>(left_backend, right_backend));
            }
            else if (!left_is_object && !right_is_object) {
                return config(std::make_shared<details::joined_array>(left_backend, right_backend));
            }
            else {
                throw yato::argument_error("Only two objects or two arrays can be joined.");
            }
        }
        if (left_backend != nullptr) {
            return left;
        }
        if (right_backend != nullptr) {
            return right;
        }
        return config{};
    }

    /**
     * Creates a new configuration with white-listed or black-listed keys.
     */
    inline
    config filter(const config & conf, const std::vector<std::string> & filter_keys, filter_mode mode = filter_mode::whitelist)
    {
        backend_ptr conf_backend = conf.get_backend();
        if(conf_backend != nullptr) {
            return config(std::make_shared<details::filter_backend>(conf_backend, filter_keys, mode));
        }
        return config{};
    }

} // namespace conf

} // namespace yato

#endif // _YATO_CONFIG_OPERATIONS_H_
