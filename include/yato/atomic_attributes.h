/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ATOMIC_ATTRIBUTES_H_
#define _YATO_ATOMIC_ATTRIBUTES_H_

#include <atomic>
#include <unordered_map>

#include "attributes_interface.h"
#include "any.h"

namespace yato
{

    namespace details
    {
        /**
         *  Implements non-atomic assign operation for atomics stored in yato::any
         */
        struct any_assigner
        {
            virtual ~any_assigner() = default;

            virtual void from_atomic(yato::any & dst, const yato::any & src) const = 0;

            virtual void from_value(yato::any & dst, const yato::any & src) const = 0;

            virtual std::unique_ptr<any_assigner> clone() const = 0;
        };

        template <typename Ty, std::memory_order MemOrder = std::memory_order_seq_cst>
        struct any_atomic_assigner
            : public any_assigner
        {
            void from_atomic(yato::any & dst, const yato::any & src) const override
            {
                dst.emplace<std::atomic<Ty>>(src.get_as<std::atomic<Ty>>().load(MemOrder));
            }

            void from_value(yato::any & dst, const yato::any & src) const override
            {
                dst.get_as<std::atomic<Ty>>().store(src.get_as<Ty>(), MemOrder);
            }

            std::unique_ptr<any_assigner> clone() const override
            {
                return std::unique_ptr<any_assigner>(new any_atomic_assigner<Ty, MemOrder>());
            }
        };

        template <typename Enum>
        struct enum_hash
        {
            size_t operator()(const Enum & value) const
            {
                using underlying_t = typename std::underlying_type<Enum>::type;
                return std::hash<underlying_t>{}(static_cast<underlying_t>(value));
            }
        };

        template <typename KeyType, typename = void>
        struct hash_chooser
        {
            using type = std::hash<KeyType>;
        };

        template <typename KeyType>
        struct hash_chooser<KeyType, typename std::enable_if<std::is_enum<KeyType>::value>::type>
        {
            using type = enum_hash<KeyType>;
        };
    }

    /**
     *  Stores attributes with atomic access
     *  All valid attributes should be registered with default value
     */
    template <
        typename KeyType = std::string, 
        std::memory_order MemoryOrder = std::memory_order_seq_cst,
#ifdef YATO_ANDROID
        typename KeyHash = typename details::hash_chooser<KeyType>::type,
#else
        typename KeyHash = std::hash<KeyType>,
#endif
        typename KeyEqual = std::equal_to<KeyType>
    >
    class atomic_attributes
        : public attributes_interface<KeyType>
    {
    private:
        using this_type = atomic_attributes<KeyType>;

    public:
        using key_type = KeyType;
        static YATO_CONSTEXPR_VAR std::memory_order atomic_memory_order = MemoryOrder;
        template <typename Ty>
        using is_valid_attribute_type = std::integral_constant<bool,
            std::is_arithmetic<Ty>::value || std::is_pointer<Ty>::value>;

    private:
        struct attribute_info
        {
            yato::any value;
            std::unique_ptr<details::any_assigner> assigner;

            attribute_info(const yato::any & value, std::unique_ptr<details::any_assigner> && assigner)
                : value(value), assigner(std::move(assigner))
            { }

            attribute_info(const attribute_info &) = delete;

            attribute_info(attribute_info && other)
                : value(std::move(other.value)), assigner(std::move(other.assigner))
            { }

            attribute_info & operator = (const attribute_info &) = delete;

            attribute_info & operator = (attribute_info &&) = delete;

            ~attribute_info() = default;
        };
        static_assert(std::is_move_constructible<attribute_info>::value, "Poor attribute_info");
        using attributes_map = std::unordered_map<key_type, attribute_info, KeyHash, KeyEqual>;
        //--------------------------------------------------------------------

        attributes_map m_attributes;
        //--------------------------------------------------------------------

        template <typename ValTy>
        bool try_set_(const key_type & key, ValTy && value)
        {
            auto pos = m_attributes.find(key);
            if (pos != m_attributes.end()) {
                (*pos).second.assigner->from_value((*pos).second.value, value);
                return true;
            }
            return false;
        }
        //--------------------------------------------------------------------

    protected:
        /**
         *  Register new attribute without default value
         *  @returns true on success, false if such attribute exists
         */
        template<typename AttributeType, typename ValueType>
        auto register_attribute(const key_type & key, ValueType && default_value)
            -> typename std::enable_if<is_valid_attribute_type<AttributeType>::value, bool>::type
        {
            std::unique_ptr<details::any_assigner> assigner(new details::any_atomic_assigner<AttributeType, atomic_memory_order>());
            auto pos_status = m_attributes.emplace(key, attribute_info(yato::nullany_t(), std::move(assigner)));
            (*pos_status.first).second.value.template emplace<std::atomic<AttributeType>>(std::forward<ValueType>(default_value));
            return pos_status.second;
        }
        //--------------------------------------------------------------------
        
        bool do_set_attribute(const KeyType & key, const yato::any & value) override
        {
            return try_set_(key, value);
        }
        //--------------------------------------------------------------------
        
        bool do_set_attribute(const KeyType & key, yato::any && value) override
        {
            return try_set_(key, std::move(value));
        }
        //--------------------------------------------------------------------

        void do_clear_attribute(const KeyType & key) override
        {
            auto pos = m_attributes.find(key);
            if (pos != m_attributes.end()) {
                (*pos).second.value.clear();
            }
        }
        //--------------------------------------------------------------------

        bool do_is_valide_attribute(const KeyType & key) const override
        {
            return (m_attributes.find(key) != m_attributes.end());
        }

    public:
        atomic_attributes() = default;

        /**
         *  Makes empty attributes set, since atomics are not copyable
         */
        atomic_attributes(const atomic_attributes & other)
        {
            for (const auto & attr_entry : other.m_attributes) {
                auto pos_status = m_attributes.emplace(attr_entry.first, attribute_info(nullany_t(), attr_entry.second.assigner->clone()));
                if (pos_status.second) {
                    attribute_info & info = (*pos_status.first).second;
                    info.assigner->from_atomic(info.value, attr_entry.second.value);
                }
            }
        }

        void swap(atomic_attributes & other) YATO_NOEXCEPT_KEYWORD
        {
            m_attributes.swap(other.m_attributes);
        }

        atomic_attributes & operator = (const atomic_attributes & other)
        {
            this_type tmp(other);
            tmp.swap(*this);
            return *this;
        }

        ~atomic_attributes() = default;

        /**
         *  Check if this attribute was set
         */
        bool has_attribute(const key_type & key) const
        {
            return do_is_valide_attribute(key);
        }

        /**
         *  Get attribute as given type
         *  If attribute doesn't exist then throws bad_attribute
         */
        template <typename AttrType>
        AttrType get_attribute_as(const key_type & key, std::memory_order mem_order = atomic_memory_order) const
        {
            auto pos = m_attributes.find(key);
            if (pos == m_attributes.cend()) {
                throw yato::bad_attribute();
            }
            const yato::any & attr = (*pos).second.value;
            if (std::type_index(attr.type()) != std::type_index(typeid(std::atomic<AttrType>))) {
                throw yato::bad_attribute();
            }
            return attr.get_as<std::atomic<AttrType>>().load(mem_order);
        }

        /**
         *  Get attribute as given type
         *  If attribute doesn't exist then throws bad_attribute
         */
        template <typename AttrType>
        AttrType get_attribute_as(const key_type & key, const AttrType & default_value, std::memory_order mem_order = atomic_memory_order) const YATO_NOEXCEPT_KEYWORD
        {
            auto pos = m_attributes.find(key);
            if (pos == m_attributes.cend()) {
                return default_value;
            }
            const yato::any & attr = (*pos).second.value;
            if (std::type_index(attr.type()) != std::type_index(typeid(std::atomic<AttrType>))) {
                return default_value;
            }
            return attr.get_as<std::atomic<AttrType>>().load(mem_order);
        }

        /**
         *  Get attribute as given type as optional value
         *  Doesn't throw bad_attribute
         */
        template <typename AttrType>
        yato::optional<AttrType> get_attribute_optional(const key_type & key, std::memory_order mem_order = atomic_memory_order) const
        {
            auto pos = m_attributes.find(key);
            if (pos == m_attributes.cend()) {
                return yato::nullopt_t{};
            }
            const yato::any & attr = (*pos).second.value;
            if (std::type_index(attr.type()) != std::type_index(typeid(std::atomic<AttrType>))) {
                return yato::nullopt_t{};
            }
            return yato::make_optional<AttrType>(attr.get_as<std::atomic<AttrType>>().load(mem_order));
        }

    };

    template <typename... Args>
    inline 
    void swap(yato::atomic_attributes<Args...> & one, yato::atomic_attributes<Args...> & another) YATO_NOEXCEPT_KEYWORD
    {
        one.swap(another);
    }

}

#endif
