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


        template <typename TagType_>
        class tagged_config_value
            : public config_value
        {
        public:
            using tag_type = TagType_;

            tagged_config_value(const config_value* value, tag_type tag)
                : m_value(value), m_tag(std::move(tag))
            { }

            ~tagged_config_value() override = default;

            stored_type type() const noexcept override
            {
                return m_value->type();
            }

            stored_variant get() const noexcept override
            {
                return m_value->get();
            }

            const config_value* value() const
            {
                return m_value;
            }

            const tag_type & tag() const
            {
                return m_tag;
            }

        private:
            const config_value* m_value;
            tag_type m_tag;
        };


        /**
         * Join implementation
         * Left is always high priority
         */
        class join_backend
            : public config_backend
        {
        private:
            enum class value_origin
            {
                left,
                right
            };

            using value_wrapper = tagged_config_value<value_origin>;

            backend_ptr m_left;
            backend_ptr m_right;
            bool m_is_object;
            size_t m_left_size;
            std::vector<std::string> m_keys;
            std::vector<size_t> m_right_index;

            static
            key_value_t wrap_value_(key_value_t kv, value_origin tag)
            {
                key_value_t res = std::make_pair(std::string{}, nullptr);
                if (kv.second) {
                    res.first = std::move(kv.first);
                    res.second = new value_wrapper(kv.second, tag);
                }
                return res;
            }

        public:
            join_backend(const backend_ptr & left, const backend_ptr & right)
                : m_left(left), m_right(right)
            {
                YATO_REQUIRES(m_left  != nullptr);
                YATO_REQUIRES(m_right != nullptr);

                const bool left_is_object  = m_left->is_object();
                const bool right_is_object = m_right->is_object();

                if (left_is_object != right_is_object) {
                    throw yato::argument_error("Only two objects or two arrays can be joined.");
                }

                m_is_object = left_is_object;

                std::vector<std::string> left_keys  = m_left->keys();
                std::vector<std::string> right_keys = m_right->keys();

                m_right_index.resize(right_keys.size());
                std::iota(m_right_index.begin(), m_right_index.end(), static_cast<size_t>(0));

                const auto left_set = std::set<std::string>(left_keys.cbegin(), left_keys.cend());

                erase2_if(right_keys, m_right_index, [&](const std::string & k) { return left_set.find(k) != left_set.cend(); });

                m_left_size = left_keys.size();

                m_keys = std::move(left_keys);
                m_keys.insert(m_keys.cend(), std::make_move_iterator(right_keys.begin()), std::make_move_iterator(right_keys.end()));
            }

            ~join_backend() override = default;

            join_backend(const join_backend&) = default;
            join_backend(join_backend&&) noexcept = default;

            join_backend& operator=(const join_backend&) = default;
            join_backend& operator=(join_backend&&) noexcept = default;

            bool do_is_object() const noexcept override
            {
                return m_is_object;
            }

            key_value_t do_find(const std::string & name) const noexcept override
            {
                key_value_t res = config_backend::novalue;
                if (m_is_object) {
                    res = wrap_value_(m_left->find(name), value_origin::left);
                    if (!res.second) {
                        res = wrap_value_(m_right->find(name), value_origin::right);
                    }
                }
                return res;
            }
            
            std::vector<std::string> do_keys() const noexcept override
            {
                return m_keys;
            }

            key_value_t do_find(size_t index) const noexcept override
            {
                key_value_t res = config_backend::novalue;
                if (index < m_left_size) {
                    res = wrap_value_(m_left->find(index), value_origin::left);
                }
                else if(index < m_keys.size()) {
                    YATO_ASSERT(index - m_left_size < m_right_index.size(), "join_backend[get_by_index]: Invalid index remapping.");
                    res = wrap_value_(m_right->find(m_right_index[index - m_left_size]), value_origin::right);
                }
                return res;
            }

            size_t do_size() const noexcept override
            {
                return m_keys.size();
            }

            void do_release(const config_value* val) const noexcept override
            {
                YATO_REQUIRES(dynamic_cast<const value_wrapper*>(val) != nullptr);
                const value_wrapper* wrapper = static_cast<const value_wrapper*>(val);
                if (wrapper && wrapper->value()) {
                    switch (wrapper->tag()) {
                        case value_origin::left:
                            m_left->release(wrapper->value());
                            break;
                        case value_origin::right:
                            m_right->release(wrapper->value());
                            break;
                    }
                    delete wrapper;
                }
            }
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

            key_value_t do_find(const std::string & name) const noexcept override
            {
                key_value_t res{};
                const auto it = std::lower_bound(m_keys.cbegin(), m_keys.cend(), name);
                if ((it != m_keys.cend()) && (*it == name)) {
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
    config join(const config & left, const config & right, priority p = priority::left)
    {
        backend_ptr left_backend  = left.get_backend();
        backend_ptr right_backend = right.get_backend();
        if (left_backend != nullptr && right_backend != nullptr) {
            if (p != priority::left) {
                left_backend.swap(right_backend);
            }
            return config(std::make_shared<details::join_backend>(left_backend, right_backend));
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
