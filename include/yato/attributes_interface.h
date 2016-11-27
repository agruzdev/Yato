/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ATTRIBUTES_INTERFACE_H_
#define _YATO_ATTRIBUTES_INTERFACE_H_

#include <map>
#include <mutex>
#include <string>
#include "any.h"
#include "optional.h"


namespace yato
{

    /**
     *  @interface
     *  Declares attributes interface trait
     */
    template <typename KeyType = std::string>
    class attributes_interface
    {
    public:
        using key_type = KeyType;
        //--------------------------------------------------------------------

    protected:
        virtual void do_set_attribute(const KeyType & key, const yato::any & value) = 0;

        virtual void do_set_attribute(const KeyType & key, yato::any && value) = 0;

        virtual void do_clear_attribute(const KeyType & key) = 0;

        virtual bool do_is_valide_attribute(const KeyType & key) const = 0;
        //--------------------------------------------------------------------

    public:
        virtual ~attributes_interface() = default;
        
        /**
         *  Set a new attribute of the object
         */
        template <typename AttrType>
        void set_attribute(const KeyType & key, AttrType && value)
        {
            do_set_attribute(key, yato::any(std::forward<AttrType>(value)));
        }

        /**
         *  Remove an attribute
         *  Does nothing if such attribute doesn't exist
         */
        void clear_attribute(const KeyType & key)
        {
            do_clear_attribute(key);
        }

        /**
         *  Check if the key value is valid attribute of the object
         *  If attribute is not valid, then setting such attribute doesn't affect behavior of the object
         */
        bool is_valide_attribute(const KeyType & key) const
        {
            return do_is_valide_attribute(key);
        }
    };

    /**
     *  Thrown on bad access
     */
    class bad_attribute
        : public std::exception
    {
    public:
        const char* what() const YATO_NOEXCEPT_KEYWORD override
        {
            return "yato::bad_attribute: attribute with the given name doesn't exist";
        }
    };


    /**
     *  Basic imlementation of attributes interface based on std::map
     */
    template <typename KeyType = std::string>
    class attributes_map
        : public attributes_interface<KeyType>
    {
    public: 
        using this_type = attributes_map<KeyType>;
        using key_type  = KeyType;
        static YATO_CONSTEXPR_VAR bool is_thread_safe = false;
        //--------------------------------------------------------------------

    private:
        std::map<key_type, yato::any> m_attributes;
        std::mutex m_mutex;
        //--------------------------------------------------------------------

    protected:
        void do_set_attribute(const KeyType & key, const yato::any & value) override
        {
            m_attributes[key] = value;
        }

        void do_set_attribute(const KeyType & key, yato::any && value) override
        {
            m_attributes[key] = std::move(value);
        }

        void do_clear_attribute(const KeyType & key) override
        {
            m_attributes.erase(key);
        }

        bool do_is_valide_attribute(const KeyType & /*key*/) const override
        {
            return true;
        }
        //--------------------------------------------------------------------

    public:
        attributes_map() = default;

        ~attributes_map() override = default;

        attributes_map(const this_type & other)
            : m_attributes(other.m_attributes)
        { }

        attributes_map & operator = (const this_type & other)
        {
            YATO_REQUIRES(this != &other);
            m_attributes = other.m_attributes;
            return *this;
        }

        attributes_map(this_type && other)
            : m_attributes(std::move(other.m_attributes))
        { }

        attributes_map & operator=(this_type && other)
        {
            YATO_REQUIRES(this != &other);
            m_attributes = std::move(other.m_attributes);
            return *this;
        }

        void swap(this_type & other)
        {
            YATO_REQUIRES(this != &other);
            m_attributes.swap(other.m_attributes);
        }

        /**
         *  Check if this attribute was set
         */
        bool has_attribute(const key_type & key) const
        {
            return (m_attributes.find(key) != m_attributes.cend());
        }

        /**
         *  Get attribute by key
         *  If attribute doesn't exist then throws bad_attribute
         */
        const yato::any & get_attribute(const key_type & key) const
        {
            auto it = m_attributes.find(key);
            if (it == m_attributes.cend()) {
                throw yato::bad_attribute();
            }
            return (*it).second;
        }

        /**
         *  Get attribute as given type
         *  If attribute doesn't exist then throws bad_attribute
         */
        template <typename AttrType>
        const AttrType & get_attribute_as(const key_type & key) const
        {
            const yato::any & attr = get_attribute(key);
            if (attr.type() != typeid(AttrType)) {
                throw yato::bad_attribute();
            }
            return yato::unsafe_any_cast<AttrType>(attr);
        }

        /**
         *  Get attribute as given type
         *  If attribute doesn't exist then returns default_value
         */
        template <typename AttrType>
        const AttrType & get_attribute_as(const key_type & key, const AttrType & default_value) const
        {
            auto it = m_attributes.find(key);
            if (it == m_attributes.cend()) {
                return default_value;
            }
            const yato::any & attr = (*it).second;
            if (attr.type() != typeid(AttrType)) {
                return default_value;
            }
            return yato::unsafe_any_cast<AttrType>(attr);
        }

        /**
         *  Get attribute by key
         *  If attribute doesn't exist then behavior is undefined
         */
        const yato::any & unsafe_get_attribute(const key_type & key) const
        {
            return (*m_attributes.find(key)).second;
        }

        /**
         *  Get attribute as given type
         *  If attribute doesn't exist then throws bad_attribute
         */
        template <typename AttrType>
        const AttrType & unsafe_get_attribute_as(const key_type & key) const
        {
            return yato::unsafe_any_cast<AttrType>(unsafe_get_attribute(key));
        }

#ifdef YATO_HAS_OPTIONAL
        /**
         *  Get attribute as given type as optional value
         *  Doesn't throw bad_attribute
         */
        template <typename AttrType>
        yato::optional<AttrType> get_attribute_optional(const key_type & key) const
        {
            auto it = m_attributes.find(key);
            if (it == m_attributes.cend()) {
                return yato::nullopt;
            }
            const yato::any & attr = (*it).second;
            if (attr.type() != typeid(AttrType)) {
                return yato::nullopt;
            }
            return yato::make_optional<AttrType>(yato::unsafe_any_cast<AttrType>(attr));
        }
#endif

        /**
         *  Acquire lock for this interface
         *  Can be used for thread-safe access
         */
        std::unique_lock<std::mutex> lock_attributes()
        {
            return std::unique_lock<std::mutex>(m_mutex);
        }
    };
}

#endif
