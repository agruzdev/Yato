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
        virtual bool do_set_attribute(const KeyType & key, const yato::any & value) = 0;

        virtual bool do_set_attribute(const KeyType & key, yato::any && value) = 0;

        virtual void do_clear_attribute(const KeyType & key) = 0;

        virtual bool do_is_valide_attribute(const KeyType & key) const = 0;
        //--------------------------------------------------------------------

    public:
        virtual ~attributes_interface() = default;
        
        /**
         *  Set a new attribute of the object
         *  @return true if attribute was accepted, false - if ignored
         */
        template <typename AttrType>
        bool set_attribute(const KeyType & key, AttrType && value)
        {
            return do_set_attribute(key, yato::any(std::forward<AttrType>(value)));
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
        : public std::runtime_error
    {
    public:
        bad_attribute()
            : std::runtime_error("yato::bad_attribute: attribute with the given name doesn't exist")
        { }
    };

    /**
     *  Thrown on if locking is not supported
     */
    class bad_lock
        : public std::runtime_error
    {
    public:
        bad_lock()
            : std::runtime_error("yato::bad_lock: lock is not provided")
        { }
    };

    /**
     *  Validator class for attributes_map
     *  Allows any key values
     */
    template <typename KeyType>
    class accept_any
    {
    public:
        bool operator()(const KeyType &) const YATO_NOEXCEPT_KEYWORD
        {
            return true;
        }
    };

    /**
     *  Validator class for attributes_map
     *  Disallows all key values
     */
    template <typename KeyType>
    class accept_none
    {
    public:
        bool operator()(const KeyType &) const YATO_NOEXCEPT_KEYWORD
        {
            return false;
        }
    };


    /**
     *  Lock provider based on std::mutex and std::unique_lock
     */
    class mutex_lock
    {
    private:
        std::mutex m_mutex;
    public:
        using lock_type = std::unique_lock<std::mutex>;

        lock_type operator()()
        {
            return std::unique_lock<std::mutex>(m_mutex);
        }
    };

    /**
     *  Empty lock provider
     */
    class no_lock
    {
    public:
        using lock_type = void*;

        YATO_NORETURN
        lock_type operator()()
        {
            throw bad_lock();
        }
    };


    /**
     *  Basic imlementation of attributes interface based on std::map
     *  Validator is not propagated on copy/move/assign
     *  LockProvider is not propagated on copy/move/assign
     */
    template <typename KeyType   = std::string, 
              typename Validator = accept_any<KeyType>,
              typename LockProvider = mutex_lock>
    class attributes_map
        : public attributes_interface<KeyType>
    {
    private:
        using this_type = attributes_map<KeyType>;
    public: 
        using key_type  = KeyType;
        using validator_type = Validator;
        using lock_provider_type = LockProvider;
        using lock_type = typename LockProvider::lock_type;
        static YATO_CONSTEXPR_VAR bool is_thread_safe = false;
        //--------------------------------------------------------------------

    private:
        std::map<key_type, yato::any> m_attributes;
        lock_provider_type m_lock_provider;
        validator_type m_validator;
        //--------------------------------------------------------------------

        template <typename ValTy_>
        void insert_or_assign_(const key_type & key, ValTy_ && value)
        {
            auto pos = m_attributes.lower_bound(key);
            if (pos == m_attributes.end() || key < (*pos).first) {
                m_attributes.emplace_hint(pos, key, std::forward<ValTy_>(value));
            }
            else {
                (*pos).second = std::forward<ValTy_>(value);
            }
        }
        //--------------------------------------------------------------------

    protected:
        bool do_set_attribute(const KeyType & key, const yato::any & value) override
        {
            insert_or_assign_(key, value);
            return true;
        }

        bool do_set_attribute(const KeyType & key, yato::any && value) override
        {
            insert_or_assign_(key, std::move(value));
            return true;
        }

        void do_clear_attribute(const KeyType & key) override
        {
            m_attributes.erase(key);
        }

        bool do_is_valide_attribute(const KeyType & key) const override
        {
            return m_validator(key);
        }
        //--------------------------------------------------------------------

    public:
        attributes_map() = default;

        explicit
        attributes_map(const validator_type & validator)
            : m_validator(validator)
        { }

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
            return attr.get_as<AttrType>();
        }

        /**
         *  Get attribute as given type
         *  If attribute doesn't exist then returns default_value
         */
        template <typename AttrType>
        const AttrType & get_attribute_as(const key_type & key, const AttrType & default_value) const YATO_NOEXCEPT_KEYWORD
        {
            auto it = m_attributes.find(key);
            if (it == m_attributes.cend()) {
                return default_value;
            }
            const yato::any & attr = (*it).second;
            return attr.get_as<AttrType>(default_value);
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
            return yato::make_optional<AttrType>(yato::any_cast<AttrType>(attr));
        }
#endif

        /**
         *  Acquire lock for this interface
         *  Can be used for thread-safe access
         */
        lock_type lock_attributes()
        {
            return m_lock_provider();
        }
    };


    /**
     *  Base class for types which ignore any attributes
     */
    template <typename KeyType = std::string>
    class ignores_attributes
        : public yato::attributes_interface<KeyType>
    {
    public:
        using this_type = ignores_attributes<KeyType>;
        using key_type = KeyType;
        static YATO_CONSTEXPR_VAR bool is_thread_safe = true;
        //--------------------------------------------------------------------

    protected:
        bool do_set_attribute(const KeyType & /*key*/, const yato::any & /*value*/) override
        {
            return false;
        }

        bool do_set_attribute(const KeyType & /*key*/, yato::any && /*value*/) override
        {
            return false;
        }

        void do_clear_attribute(const KeyType & /*key*/) override
        { }

        bool do_is_valide_attribute(const KeyType & /*key*/) const override
        {
            return false;
        }
        //--------------------------------------------------------------------

    public:
        ignores_attributes() = default;

        ignores_attributes(const this_type &) = default;

        ignores_attributes & operator = (const this_type &) = default;

#ifndef YATO_MSVC_2013
        ignores_attributes(this_type&&) = default;
        ignores_attributes & operator = (this_type&&) = default;
#else
        ignores_attributes(this_type&&)
        { }

        ignores_attributes & operator = (this_type&&)
        {
            return *this;
        }
#endif
        ~ignores_attributes() override = default;
    };
}

#endif
