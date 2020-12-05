/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_OPTIONAL_H_
#define _YATO_OPTIONAL_H_

#include "assertion.h"
#include "type_traits.h"
#include "types.h"
#include "memory_utility.h"

namespace yato
{
    class nullopt_t {};

#ifndef YATO_MSVC_2013
    YATO_INLINE_VARIABLE constexpr nullopt_t nullopt{};
#endif

    class bad_optional_access
        : public yato::runtime_error
    {
    public:
        bad_optional_access(const char* what)
            : yato::runtime_error(what)
        { }
    };

    namespace details
    {
        template <typename DstTy_, typename SrcTy_, typename = void>
        struct opt_assign_op_
        {
            static
            void apply(DstTy_* dst, const SrcTy_ * src) {
                dst->~DstTy_();
                new (static_cast<void*>(dst)) DstTy_(*src);
            }
        };

        template <typename DstTy_, typename SrcTy_>
        struct opt_assign_op_ <
                DstTy_,
                SrcTy_,
                std::enable_if_t<std::is_assignable<DstTy_, SrcTy_>::value>
            >
        {
            static
            void apply(DstTy_* dst, const SrcTy_ * src) {
                *dst = *src;
            }
        };


        template <typename DstTy_, typename SrcTy_, typename = void>
        struct opt_move_op_ 
        {
            static
            void apply(DstTy_* dst, SrcTy_ * src) {
                dst->~DstTy_();
                new (static_cast<void*>(dst)) DstTy_(std::move(*src));
            }
        };

        template <typename DstTy_, typename SrcTy_>
        struct opt_move_op_ <
                DstTy_,
                SrcTy_,
                yato::void_t<decltype(std::declval<DstTy_>() = std::move(std::declval<SrcTy_>()))>
            >
        {
            static
            void apply(DstTy_* dst, SrcTy_ * src) {
                *dst = std::move(*src);
            }
        };


        template <typename ValTy_, typename = void>
        class optional_storage
        {
            using value_type   = ValTy_;
            using storage_type = typename std::aligned_storage<sizeof(value_type), std::alignment_of<value_type>::value>::type;
        public:

            constexpr
            optional_storage()
                : m_stored(false)
            { }

            optional_storage(const optional_storage&)
            { }

            optional_storage(optional_storage&&) noexcept
            { }

            ~optional_storage()
            { }

            optional_storage& operator=(const optional_storage&) 
            { }

            optional_storage& operator=(optional_storage&&) noexcept
            { }

            template <typename... Args_>
            YATO_CONSTEXPR_FUNC_CXX14
            void construct(Args_ &&... args)
            {
                YATO_REQUIRES(!m_stored);
                new (address_()) value_type(std::forward<Args_>(args)...);
            }

            void destroy() noexcept
            {
                YATO_REQUIRES(m_stored);
                address_()->~value_type();
            }

            constexpr
            bool is_stored() const
            {
                return m_stored;
            }

            constexpr
            void set_stored(bool val)
            {
                m_stored = val;
            }

            constexpr
            value_type & ref()
            {
                return *address_();
            }

            constexpr
            const value_type & cref() const
            {
                return *caddress_();
            }


            //-------------------------------------------------------------
        private:
            storage_type m_value{};
            bool m_stored;

            constexpr
            value_type* address_()
            {
                return yato::launder(yato::pointer_cast<value_type*>(&m_value));
            }

            constexpr
            const value_type* caddress_() const
            {
                return yato::launder(yato::pointer_cast<const value_type*>(&m_value));
            }
        };


        template <typename ValTy_>
        class optional_storage<
                ValTy_,
                std::enable_if_t<std::is_pointer<ValTy_>::value>
            >
        {
            using pointer_type = ValTy_;
        public:

            constexpr
            optional_storage() = default;

            optional_storage(const optional_storage&) = default;
            optional_storage(optional_storage&&) = default;

            ~optional_storage() = default;

            optional_storage& operator=(const optional_storage&) = default;
            optional_storage& operator=(optional_storage&&) = default;

            YATO_CONSTEXPR_FUNC_CXX14
            void construct(pointer_type ptr)
            {
                m_ptr = ptr;
            }

            void destroy() noexcept
            {
                m_ptr = nullptr;
            }

            constexpr
            bool is_stored() const
            {
                return m_ptr != nullptr;
            }

            constexpr
            void set_stored(bool /*val*/)
            { }

            constexpr
            pointer_type & ref()
            {
                return m_ptr;
            }

            constexpr
            const pointer_type & cref() const
            {
                return m_ptr;
            }
            //-------------------------------------------------------------

        private:
            pointer_type m_ptr = nullptr;
        };


        template <typename ValTy_>
        class optional_storage<
                ValTy_,
                std::enable_if_t<std::is_reference<ValTy_>::value>
            >
        {
            static_assert(!std::is_rvalue_reference<ValTy_>::value, "yato::optional can't store reference.");
        };

    } // namespace details


    template <typename Ty_>
    struct is_optional
        : std::false_type
    { };


    template <typename ValueType_>
    class basic_optional
    {
        using storage_type = details::optional_storage<ValueType_>;
        using this_type = basic_optional<ValueType_>;

    public:
        using value_type = ValueType_;

        static constexpr bool is_copy_constructible_v = std::is_copy_constructible<value_type>::value;
        static constexpr bool is_move_constructible_v = std::is_move_constructible<value_type>::value;
        static constexpr bool is_copy_assignable_v    = std::is_copy_constructible<value_type>::value && std::is_copy_assignable<value_type>::value;
        static constexpr bool is_move_assignable_v    = std::is_move_constructible<value_type>::value && std::is_move_assignable<value_type>::value;

        static constexpr bool is_nothrow_move_constructible_v = std::is_nothrow_move_constructible<value_type>::value;
        static constexpr bool is_nothrow_move_assignable_v    = std::is_nothrow_move_constructible<value_type>::value && std::is_nothrow_move_assignable<value_type>::value;

        static constexpr bool is_swappable_v         = this_type::is_move_constructible_v && this_type::is_move_assignable_v;
        static constexpr bool is_nothrow_swappable_v = this_type::is_nothrow_move_assignable_v;

        template <typename... Args_>
        using is_constructible = std::is_constructible<value_type, Args_...>; 

        template <typename... Args_>
        using is_nothrow_constructible = std::is_nothrow_constructible<value_type, Args_...>;

        template <typename... Args_>
        using is_assignable = yato::boolean_constant<std::is_constructible<value_type, Args_...>::value && std::is_constructible<value_type, Args_...>::value>; 

        template <typename... Args_>
        using is_nothrow_assignable = yato::boolean_constant<std::is_nothrow_constructible<value_type, Args_...>::value && std::is_nothrow_assignable<value_type, Args_...>::value>;


        /**
         * Creates emtpy optional.
         */
        constexpr
        basic_optional() = default;

        /**
         * Creates emtpy optional.
         */
        constexpr
        basic_optional(nullopt_t)
        { }

        /**
         * Creates optional with value_type iniialized from the provided arguments.
         */
        template <typename... Args_>
        YATO_CONSTEXPR_FUNC_CXX14 explicit
        basic_optional(yato::in_place_t, Args_ && ... args)
        {
            construct_(std::forward<Args_>(args)...);
        }

        /**
         * Creates optional with value_type iniialized from a value.
         * Workaround for deleted move constructors.
         */
        template <typename Uy_, typename = 
            std::enable_if_t<this_type::is_constructible<yato::add_lvalue_reference_to_const_t<Uy_>>::value>
        >
        YATO_CONSTEXPR_FUNC_CXX14 explicit
        basic_optional(const Uy_ & val)
        {
            construct_(val);
        }

        /**
         * Creates optional with value_type iniialized from a value.
         */
        template <typename Uy_, typename = 
            std::enable_if_t<this_type::is_constructible<Uy_>::value>
        >
        YATO_CONSTEXPR_FUNC_CXX14 explicit
        basic_optional(Uy_ && val) noexcept(this_type::is_nothrow_constructible<Uy_>::value)
        {
            construct_(std::forward<Uy_>(val));
        }

        /**
         * Copy constructor will be defined if and only if value_type is copy constructible.
         */
        YATO_CONSTEXPR_FUNC_CXX14
        basic_optional(const yato::disable_if_not_t<this_type::is_copy_constructible_v, this_type>& other)
        {
            copy_constructor_(other);
        }

        /**
         * Move constructor will be defined if and only if value_type is move constructible.
         * Is noexcept if and only if value_type is nothrow move constructible.
         */
        YATO_CONSTEXPR_FUNC_CXX14
        basic_optional(yato::disable_if_not_t<this_type::is_move_constructible_v, this_type>&& other) noexcept(this_type::is_nothrow_move_constructible_v)
        {
            move_constructor_(std::move(other));
        }

        /**
         * Copy from another optional holding a type convertible to value_type.
         */
        template <typename OtherVal_, typename =
            std::enable_if_t<is_constructible<yato::add_lvalue_reference_to_const_t<OtherVal_>>::value>
        >
        basic_optional(const basic_optional<OtherVal_> & other)
        {
            copy_constructor_(other);
        }

        /**
         * Move from another optional holding a type convertible to value_type.
         */
        template <typename OtherVal_, typename =
            std::enable_if_t<is_constructible<std::add_rvalue_reference_t<OtherVal_>>::value>
        >
        basic_optional(basic_optional<OtherVal_> && other) 
        {
            move_constructor_(std::move(other));
        }

        /**
         * Destroyes held value.
         */
        ~basic_optional()
        {
            destroy_();
        }

        /**
         * Constructs the contained value in-place.
         */
        template <typename... Args_>
        void emplace(Args_ && ... args)
        {
            destroy_();
            construct_(std::forward<Args_>(args)...);
        }

        /**
         * Copy assignment operator will be defined if and only if value_type is copy assignable.
         */
        basic_optional& operator=(const yato::disable_if_not_t<this_type::is_copy_assignable_v, this_type>& other)
        {
            YATO_REQUIRES(this != &other);
            return copy_assign_(other);
        }

        /**
         * Move assignment operator will be defined if and only if value_type is move assignable.
         * Is noexcept if and only if value_type is nothrow move assignable.
         */
        basic_optional& operator=(yato::disable_if_not_t<this_type::is_move_assignable_v, this_type>&& other) noexcept(this_type::is_nothrow_move_assignable_v)
        {
            YATO_REQUIRES(this != &other);
            return move_assign_(std::move(other));
        }

        /**
         * Assign from another optional holdign a type assignable to value_type.
         */
        template <typename OtherVal_, typename =
            std::enable_if_t<is_assignable<yato::add_lvalue_reference_to_const_t<OtherVal_>>::value>
        >
        basic_optional & operator=(const basic_optional<OtherVal_> & other)
        {
            return copy_assign_(other);
        }

        /**
         * Move from another optional holdign a type move assignable to value_type.
         */
        template <typename OtherVal_, typename =
            std::enable_if_t<is_assignable<std::add_rvalue_reference_t<OtherVal_>>::value>
        >
        basic_optional & operator= (basic_optional<OtherVal_> && other) 
            noexcept(is_nothrow_assignable<std::add_rvalue_reference_t<OtherVal_>>::value)
        {
            return move_assign_(std::move(other));
        }

        /**
         * Swap two optionals
         * value_type must be move copyable and move assignable.
         */
        template <typename ThisType_ = this_type, typename = 
            std::enable_if_t<ThisType_::is_swappable_v>
        >
        void swap(basic_optional<ValueType_> & other) noexcept(ThisType_::is_nothrow_swappable_v)
        {
            swap_(other);
        }

        /**
         * If optional contains a value, returns a reference to this value.
         * Otherwise, throws a yato::bad_optional_access exception. 
         */
        YATO_CONSTEXPR_FUNC_CXX14
        const value_type & get() const &
        {
            if (!m_storage.is_stored()) {
                throw yato::bad_optional_access("empty optional");
            }
            return m_storage.cref();
        }

        /**
         * If optional contains a value, returns a reference to this value.
         * Otherwise, throws a yato::bad_optional_access exception. 
         */
        YATO_CONSTEXPR_FUNC_CXX14
        value_type & get() &
        {
            if (!m_storage.is_stored()) {
                throw yato::bad_optional_access("empty optional");
            }
            return m_storage.ref();
        }

        /**
         * If optional contains a value, returns a reference to this value.
         * Otherwise, throws a yato::bad_optional_access exception. 
         */
        YATO_CONSTEXPR_FUNC_CXX14
        const value_type && get() const && 
        {
            if (!m_storage.is_stored()) {
                throw yato::bad_optional_access("empty optional");
            }
            return std::move(m_storage.cref());
        }

        /**
         * If optional contains a value, returns a reference to this value.
         * Otherwise, throws a yato::bad_optional_access exception. 
         */
        YATO_CONSTEXPR_FUNC_CXX14
        value_type && get() &&
        {
            if (!m_storage.is_stored()) {
                throw yato::bad_optional_access("empty optional");
            }
            return std::move(m_storage.ref());
        }

        /**
         * If optional contains a value, returns a reference to this value.
         * Makes no checks, if optional was empty then behaviour is undefined.
         */
        YATO_CONSTEXPR_FUNC_CXX14
        const value_type & get_unsafe() const &
        {
            YATO_REQUIRES(m_storage.is_stored());
            return m_storage.cref();
        }

        /**
         * If optional contains a value, returns a reference to this value.
         * Makes no checks, if optional was empty then behaviour is undefined.
         */
        YATO_CONSTEXPR_FUNC_CXX14
        value_type & get_unsafe() &
        {
            YATO_REQUIRES(m_storage.is_stored());
            return m_storage.ref();
        }

        /**
         * If optional contains a value, returns a reference to this value.
         * Makes no checks, if optional was empty then behaviour is undefined.
         */
        YATO_CONSTEXPR_FUNC_CXX14
        const value_type && get_unsafe() const &&
        {
            YATO_REQUIRES(m_storage.is_stored());
            return std::move(m_storage.cref());
        }

        /**
         * If optional contains a value, returns a reference to this value.
         * Makes no checks, if optional was empty then behaviour is undefined.
         */
        YATO_CONSTEXPR_FUNC_CXX14
        value_type && get_unsafe() &&
        {
            YATO_REQUIRES(m_storage.is_stored());
            return std::move(m_storage.ref());
        }


        /**
         * If optional contains a value, returns a reference to this value.
         * Otherwise, returns default_value.
         */
        template <typename Uy_>
        constexpr
        value_type get_or(Uy_ && default_value) const &
        {
            return m_storage.is_stored() ? m_storage.cref() : static_cast<value_type>(std::forward<Uy_>(default_value));
        }

        /**
         * If optional contains a value, returns a reference to this value.
         * Otherwise, returns default_value.
         */
        template <typename Uy_>
        constexpr
        value_type get_or(Uy_ && default_value) &&
        {
            return m_storage.is_stored() ? std::move(m_storage.ref()) : static_cast<value_type>(std::forward<Uy_>(default_value));
        }

        /**
          * Alias for get()
          */
        YATO_CONSTEXPR_FUNC_CXX14
        const value_type & operator*() const &
        {
            return get();
        }

        /**
         * Alias for get()
         */
        YATO_CONSTEXPR_FUNC_CXX14
        value_type & operator*() &
        {
            return get();
        }

        /**
         * Alias for get()
         */
        YATO_CONSTEXPR_FUNC_CXX14
        const value_type && operator*() const &&
        {
            return std::move(*this).get();
        }

        /**
         * Alias for get()
         */
        YATO_CONSTEXPR_FUNC_CXX14
        value_type && operator*() &&
        {
            return std::move(*this).get();
        }

        /**
         * If optional contains a value, returns false.
         * Otherwise, returns true.
         */
        constexpr
        bool empty() const
        {
            return !m_storage.is_stored();
        }

        /**
         * If optional contains a value, returns true.
         * Otherwise, returns false.
         */
        constexpr explicit
        operator bool() const
        {
            return m_storage.is_stored();
        }

        /** 
         * If optional contains a value, then destroys it, making optional empty.
         */
        void reset() noexcept
        {
            destroy_();
        }

        /**
         * Accept visitor for a stored value.
         */
        template <typename Visitor_>
        const basic_optional & visit(Visitor_ && v) const &
        {
            visit_impl_(*this, std::forward<Visitor_>(v));
            return *this;
        }

        /**
         * Accept visitor for a stored value.
         */
        template <typename Visitor_>
        basic_optional & visit(Visitor_ && v) &
        {
            visit_impl_(*this, std::forward<Visitor_>(v));
            return *this;
        }

        /**
         * Accept visitor for a stored value.
         */
        template <typename Visitor_>
        const basic_optional && visit(Visitor_ && v) const &&
        {
            visit_impl_(std::move(*this), std::forward<Visitor_>(v));
            return std::move(*this);
        }

        /**
         * Accept visitor for a stored value.
         */
        template <typename Visitor_>
        basic_optional && visit(Visitor_ && v) &&
        {
            visit_impl_(std::move(*this), std::forward<Visitor_>(v));
            return std::move(*this);
        }
 
        /**
         * Accepts a transform function and returns a new optional containing the function invokation result.
         */
        template <typename Function_>
        auto map(Function_ && f) const &
        {
            return map_impl_(*this, std::forward<Function_>(f));
        }

        /**
         * Accepts a transform function and returns a new optional containing the function invokation result.
         */
        template <typename Function_>
        auto map(Function_ && f) &
        {
            return map_impl_(*this, std::forward<Function_>(f));
        }

        /**
         * Accepts a transform function and returns a new optional containing the function invokation result.
         */
        template <typename Function_>
        auto map(Function_ && f) const &&
        {
            return map_impl_(std::move(*this), std::forward<Function_>(f));
        }

        /**
         * Accepts a transform function and returns a new optional containing the function invokation result.
         */
        template <typename Function_>
        auto map(Function_ && f) &&
        {
            return map_impl_(std::move(*this), std::forward<Function_>(f));
        }

        /**
         * If optional contains a value and predicate returns true for that value, then returns copy of the stored value.
         * Otherwise, returns empty optional.
         * Predicate must satisfy the following signature 'bool P(const value_type &)'.
         * value_type must be copy constructible.
         */
        template <typename Predicate_>
        basic_optional filter(Predicate_ && p) const &
        {
            return filter_impl_(*this, std::forward<Predicate_>(p));
        }

        /**
         * If optional contains a value and predicate returns true for that value, then returns copy of the stored value.
         * Otherwise, returns empty optional.
         * Predicate must satisfy the following signature 'bool P(const value_type &)'.
         * value_type must be copy constructible.
         */
        template <typename Predicate_>
        basic_optional filter(Predicate_ && p) &
        {
            return filter_impl_(*this, std::forward<Predicate_>(p));
        }

        /**
         * If optional contains a value and predicate returns true for that value, then returns copy of the stored value.
         * Otherwise, returns empty optional.
         * Predicate must satisfy the following signature 'bool P(const value_type &)'.
         * value_type must be move constructible.
         */
        template <typename Predicate_>
        basic_optional filter(Predicate_ && p) const &&
        {
            return filter_impl_(std::move(*this), std::forward<Predicate_>(p));
        }

        /**
         * If optional contains a value and predicate returns true for that value, then returns copy of the stored value.
         * Otherwise, returns empty optional.
         * Predicate must satisfy the following signature 'bool P(const value_type &)'.
         * value_type must be move constructible.
         */
        template <typename Predicate_>
        basic_optional filter(Predicate_ && p) &&
        {
            return filter_impl_(std::move(*this), std::forward<Predicate_>(p));
        }

        /**
         * If optional contains a value and predicate returns true for that value, then returns true.
         * Otherwise, returns false.
         * Predicate must satisfy the following signature 'bool P(const value_type &)'.
         */
        template <typename Predicate_>
        bool exists(Predicate_ && p) const &
        {
            return exists_impl_(*this, std::forward<Predicate_>(p));
        }

        /**
         * If optional contains a value and predicate returns true for that value, then returns true.
         * Otherwise, returns false.
         * Predicate must satisfy the following signature 'bool P(const value_type &)'.
         */
        template <typename Predicate_>
        bool exists(Predicate_ && p) &
        {
            return exists_impl_(*this, std::forward<Predicate_>(p));
        }

        /**
         * If optional contains a value and predicate returns true for that value, then returns true.
         * Otherwise, returns false.
         * Predicate must satisfy the following signature 'bool P(const value_type &)'.
         */
        template <typename Predicate_>
        bool exists(Predicate_ && p) const &&
        {
            return exists_impl_(std::move(*this), std::forward<Predicate_>(p));
        }

        /**
         * If optional contains a value and predicate returns true for that value, then returns true.
         * Otherwise, returns false.
         * Predicate must satisfy the following signature 'bool P(const value_type &)'.
         */
        template <typename Predicate_>
        bool exists(Predicate_ && p) &&
        {
            return exists_impl_(std::move(*this), std::forward<Predicate_>(p));
        }

        /**
         * Transforms nested options into one option.
         */
        auto flatten() const &
        {
            return flatten_impl_(*this);
        }

        /**
         * Transforms nested options into one option.
         */
        auto flatten() &
        {
            return flatten_impl_(*this);
        }

        /**
         * Transforms nested options into one option.
         */
        auto flatten() const &&
        {
            return flatten_impl_(std::move(*this));
        }

        /**
         * Transforms nested options into one option.
         */
        auto flatten() &&
        {
            return flatten_impl_(std::move(*this));
        }


        // For internal usage
        template <typename Ty_>
        static
        auto make_optional_(Ty_ && val)
        {
            return basic_optional<yato::remove_cvref_t<Ty_>>(std::forward<Ty_>(val));
        }


        basic_optional(const yato::disable_if_not_t<!this_type::is_copy_constructible_v, this_type>& other) = delete;
        basic_optional(yato::disable_if_not_t<!this_type::is_move_constructible_v, this_type>&& other) = delete;

        basic_optional& operator=(const yato::disable_if_not_t<!this_type::is_copy_assignable_v, this_type>& other) = delete;
        basic_optional& operator=(yato::disable_if_not_t<!this_type::is_move_assignable_v, this_type>&& other) = delete;

        //------------------------------------------------------------------------------
    private:
        template <typename... Args_>
        YATO_CONSTEXPR_FUNC_CXX14
        void construct_(Args_ && ... args)
        {
            YATO_REQUIRES(!m_storage.is_stored());
            m_storage.construct(std::forward<Args_>(args)...);
            m_storage.set_stored(true);
        }

        void destroy_() noexcept
        {
            if (m_storage.is_stored()) {
                m_storage.destroy();
                m_storage.set_stored(false);
            }
        }

        template <typename Ty_ = this_type, typename = std::enable_if_t<Ty_::is_swappable_v>>
        YATO_CONSTEXPR_FUNC_CXX14
        void swap_(basic_optional<value_type> & other) noexcept(is_nothrow_swappable_v)
        {
            if (m_storage.is_stored() && other.m_storage.is_stored()) {
                using std::swap;
                swap(m_storage.ref(), other.m_storage.ref());
            }
            else if(other.m_storage.is_stored()) {
                move_constructor_(std::move(other));
            }
            else {
                destroy_();
            }
        }

        template <typename Uy_>
        YATO_CONSTEXPR_FUNC_CXX14
        void copy_constructor_(const basic_optional<Uy_> & other)
        {
            if (other.m_storage.is_stored()) {
                construct_(other.m_storage.cref());
            }
            else {
                m_storage.set_stored(false);
            }
        }

        template <typename Uy_>
        YATO_CONSTEXPR_FUNC_CXX14
        void move_constructor_(basic_optional<Uy_> && other)
        {
            if (other.m_storage.is_stored()) {
                construct_(std::move(other.m_storage.ref()));
                other.destroy_();
            }
            else {
                m_storage.set_stored(false);
            }
        }

        template <typename Uy_>
        YATO_CONSTEXPR_FUNC_CXX14
        basic_optional& copy_assign_(const basic_optional<Uy_> & other)
        {
            if(m_storage.is_stored() && other.m_storage.is_stored()) {
                m_storage.ref() = other.m_storage.cref();
            }
            else {
                destroy_();
                if (other.m_storage.is_stored()) {
                    construct_(other.m_storage.cref());
                }
            }
            return *this;
        }

        template <typename Uy_>
        YATO_CONSTEXPR_FUNC_CXX14
        basic_optional& move_assign_(basic_optional<Uy_>&& other)
        {
            if (m_storage.is_stored() && other.m_storage.is_stored()) {
                m_storage.ref() = std::move(other.m_storage.ref());
            }
            else {
                destroy_();
                if (other.m_storage.is_stored()) {
                    construct_(std::move(other.m_storage.ref()));
                }
            }
            return *this;
        }

        const value_type & get_cref_() const &
        {
            YATO_REQUIRES(m_storage.is_stored());
            return m_storage.cref();
        }

        template <typename This_, typename Visitor_>
        static
        void visit_impl_(This_ && thiz, Visitor_&& v)
        {
            if (!thiz.empty()) {
                std::forward<Visitor_>(v)(std::forward<This_>(thiz).get_unsafe());
            }
        }

        template <typename This_, typename Function_>
        static
        auto map_impl_(This_ && thiz, Function_&& f)
        {
            return static_cast<bool>(thiz)
                ? make_optional_(std::forward<Function_>(f)(std::forward<This_>(thiz).get_unsafe()))
                : yato::nullopt_t{};
        }

        template <typename This_, typename Predicate_>
        static
        basic_optional<value_type> filter_impl_(This_ && thiz, Predicate_&& p)
        {
            return static_cast<bool>(thiz) && std::forward<Predicate_>(p)(thiz.get_cref_())
                ? basic_optional<value_type>(std::forward<This_>(thiz))
                : yato::nullopt_t{};
        }

        template <typename This_, typename Predicate_>
        static
        bool exists_impl_(This_ && thiz, Predicate_&& p)
        {
            return static_cast<bool>(thiz) && std::forward<Predicate_>(p)(thiz.get_cref_());
        }

        template <typename Uy_, typename =
            std::enable_if_t<is_optional<Uy_>::value>
        >
        static
        auto flatten_impl_(const basic_optional<Uy_> & opt)
        {
            return static_cast<bool>(opt)
                ? flatten_impl_(opt.get_unsafe())
                : yato::nullopt_t{};
        }

        template <typename Uy_, typename =
            std::enable_if_t<is_optional<Uy_>::value>
        >
        static
        auto flatten_impl_(basic_optional<Uy_> && opt)
        {
            return static_cast<bool>(opt)
                ? flatten_impl_(std::move(opt).get_unsafe())
                : yato::nullopt_t{};
        }

        template <typename Uy_, typename =
            std::enable_if_t<!is_optional<Uy_>::value>
        >
        static
        basic_optional<Uy_> flatten_impl_(const basic_optional<Uy_> & opt)
        {
            return opt;
        }

        template <typename Uy_, typename =
            std::enable_if_t<!is_optional<Uy_>::value>
        >
        static
        basic_optional<Uy_> flatten_impl_(basic_optional<Uy_> && opt)
        {
            return std::move(opt);
        }

        void copy_constructor_(yato::disabled_t)
        { }

        void move_constructor_(yato::disabled_t)
        { }

        basic_optional& copy_assign_(yato::disabled_t)
        {
            return *this;
        }

        basic_optional& move_assign_(yato::disabled_t)
        {
            return *this;
        }
        //------------------------------------------------------------------------------

        storage_type m_storage;
        //------------------------------------------------------------------------------

        template <typename OtherVal_>
        friend class basic_optional;
    };


    template <typename Uy_>
    struct is_optional<basic_optional<Uy_>>
        : std::true_type
    { };


    /**
     * Swap specialization for yato::optional
     */
    template <typename Ty_>
    void swap(basic_optional<Ty_> & lhs, basic_optional<Ty_> & rhs)
    {
        lhs.swap(rhs);
    }


    /**
     * Container for optional value.
     * Provides specific no-copyable overload for stored pointer types.
     * Supports special overloads for:
     *  Ty_* - optional pointer: nullptr is treated as empty optional. Size is equal to sizeof(void*). Doesn't manage object lifetime.
     */
    template <typename Ty_>
    using optional = basic_optional<Ty_>;


    /**
     * Create optional from value with deduced type
     */
    template <typename Ty_>
    constexpr
    auto make_optional(Ty_ && val)
    {
        return basic_optional<yato::remove_cvref_t<Ty_>>(std::forward<Ty_>(val));
    }

    /**
     * Create optional of a specified type
     */
    template <typename Ty_, typename = 
        std::enable_if_t<!std::is_reference<Ty_>::value>
    >
    constexpr
    basic_optional<Ty_> some(Ty_ val)
    {
        return basic_optional<Ty_>(std::move(val));
    }

    /**
     * some() can't create empty optional
     */
    void some(nullopt_t) = delete;

    /**
     * some() can't create empty optional
     */
    void some(std::nullptr_t) = delete;

} // namespace yato

#endif

