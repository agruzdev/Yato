/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_OPTIONAL_H_
#define _YATO_OPTIONAL_H_

#include "type_traits.h"
#include "assert.h"

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



        template <typename Ty_>
        class optional_core
        {
        public:
            using value_type   = Ty_;
            using storage_type = typename std::aligned_storage<sizeof(Ty_), std::alignment_of<Ty_>::value>::type;

        private:
            storage_type m_value{};
            bool m_stored = false;

            const value_type* get_ptr_() const {
                return yato::pointer_cast<const value_type*>(&m_value);
            }
        
            value_type* get_ptr_() {
                return yato::pointer_cast<value_type*>(&m_value);
            }

        protected:
            static YATO_CONSTEXPR_VAR bool is_nothrow_move_constructible = std::is_nothrow_constructible<value_type>::value;
            static YATO_CONSTEXPR_VAR bool is_nothrow_move_assignable = is_nothrow_move_constructible && std::is_nothrow_move_assignable<value_type>::value;

            template <typename... Args_>
            void construct_(Args_ && ... args) {
                new (static_cast<void*>(get_ptr_())) value_type(std::forward<Args_>(args)...);
                m_stored = true;
            }

            template <typename... Args_>
            void destroy_(Args_ && ... args) {
                get_ptr_()->~value_type();
                m_stored = false;
            }

            template <typename OtherVal_>
            void copy_impl_(const optional_core<OtherVal_> & other) {
                if(other.m_stored) {
                    construct_(*other.get_ptr_());
                }
                else {
                    m_stored = false;
                }
            }
    
            template <typename OtherVal_>
            void move_copy_impl_(optional_core<OtherVal_> && other) noexcept(is_nothrow_move_constructible) {
                if(other.m_stored) {
                    construct_(std::move(*other.get_ptr_()));
                }
                else {
                    m_stored = false;
                }
            }

            template <typename OtherVal_>
            void move_assign_impl_(optional_core<OtherVal_> && other) noexcept(is_nothrow_move_assignable) {
                if(m_stored && other.m_stored) {
                    details::opt_move_op_<value_type, OtherVal_>::apply(get_ptr_(), other.get_ptr_());
                }
                else if(!m_stored && other.m_stored) {
                    construct_(std::move(*other.get_ptr_()));
                    other.clear();
                }
                else if(m_stored && !other.m_stored) {
                    clear();
                }
                else {
                    // do nothing
                }
            }

            template <typename OtherVal_>
            void assign_impl_(const optional_core<OtherVal_> & other) {
                if(m_stored && other.m_stored) {
                    details::opt_assign_op_<value_type, OtherVal_>::apply(get_ptr_(), other.get_ptr_());
                }
                else if(!m_stored && other.m_stored) {
                    construct_(*other.get_ptr_());
                }
                else if(m_stored && !other.m_stored) {
                    clear();
                }
                else {
                    // do nothing
                }
            }

            template <typename OtherVal_>
            void swap_impl_(optional_core<OtherVal_> & other)
            {
                if(m_stored && other.m_stored) {
                    using std::swap;
                    swap(*get_ptr_(), *other.get_ptr_());
                }
                else if(!m_stored && other.m_stored) {
                    construct_(std::move(*other.get_ptr_()));
                    other.clear();
                }
                else if(m_stored && !other.m_stored) {
                    other.construct_(std::move(*get_ptr_()));
                    clear();
                }
                else {
                    // do nothing
                }
            }

        public:
            constexpr
            optional_core() = default;

            ~optional_core()
            {
                clear();
            }

            template <typename... Args_>
            constexpr explicit
            optional_core(yato::in_place_t, Args_ && ... args)
            {
                construct_(std::forward<Args_>(args)...);
            }

            optional_core(const optional_core&) = delete;
            optional_core(optional_core&&) = delete;

            optional_core& operator = (const optional_core&) = delete;
            optional_core& operator = (optional_core&&) = delete;

            constexpr
            const value_type & get() const & {
                if(!m_stored) {
                    throw yato::bad_optional_access("empty optional");
                }
                return *get_ptr_();
            }

            YATO_CONSTEXPR_FUNC_EX
            value_type & get() & {
                if(!m_stored) {
                    throw yato::bad_optional_access("empty optional");
                }
                return *get_ptr_();
            }

            constexpr
            const value_type && get() const && {
                if(!m_stored) {
                    throw yato::bad_optional_access("empty optional");
                }
                return std::move(*get_ptr_());
            }

            YATO_CONSTEXPR_FUNC_EX
            value_type && get() && {
                if(!m_stored) {
                    throw yato::bad_optional_access("empty optional");
                }
                return std::move(*get_ptr_());
            }

            constexpr
            const value_type & get_unsafe() const & {
                YATO_REQUIRES(m_stored);
                return *get_ptr_();
            }

            YATO_CONSTEXPR_FUNC_EX
            value_type & get_unsafe() & {
                YATO_REQUIRES(m_stored);
                return *get_ptr_();
            }

            constexpr
            const value_type && get_unsafe() const && {
                YATO_REQUIRES(m_stored);
                return std::move(*get_ptr_());
            }

            YATO_CONSTEXPR_FUNC_EX
            value_type && get_unsafe() && {
                YATO_REQUIRES(m_stored);
                return std::move(*get_ptr_());
            }

            template <typename DefTy_>
            constexpr
            value_type get_or(DefTy_ && default_value) const & {
                return m_stored ? *get_ptr_() : static_cast<value_type>(std::forward<DefTy_>(default_value));
            }

            template <typename DefTy_>
            YATO_CONSTEXPR_FUNC_EX
            value_type get_or(DefTy_ && default_value) && {
                return m_stored ? std::move(*get_ptr_()) : static_cast<value_type>(std::forward<DefTy_>(default_value));
            }

            constexpr
            bool empty() const {
                return !m_stored;
            }
    
            constexpr
            operator bool() const {
                return m_stored;
            }

            /** 
             * Destory stored value and make empty
             */
            void clear() {
                if(m_stored) {
                    destroy_();
                }
            }
    
            template <typename... Args_>
            void emplace(Args_ && ... args) {
                clear();
                construct_(std::forward<Args_>(args)...);
            }

            template <typename SomeTy_>
            friend class optional_core;
        };




        template <typename TyPtr_>
        class optional_ptr
        {
        private:
            static_assert(std::is_pointer<TyPtr_>::value, "Only for pointers");
            using this_type  = optional_ptr<TyPtr_>;

        public:
            using value_type = TyPtr_;
            using reference_type = std::remove_pointer_t<TyPtr_>;

        private:
            TyPtr_ m_ptr = nullptr;

        public:
            constexpr
            optional_ptr() = default;

            constexpr explicit
            optional_ptr(TyPtr_ ptr)
                : m_ptr(ptr)
            { }

            constexpr
            optional_ptr(yato::nullopt_t)
                : optional_ptr()
            { }

            ~optional_ptr() = default;

            optional_ptr(const optional_ptr&) = delete;

            optional_ptr(optional_ptr && other) noexcept
                : m_ptr(other.m_ptr)
            {
                other.m_ptr = nullptr;
            }

            template <typename Ptr_, typename = std::enable_if_t<std::is_convertible<Ptr_, value_type>::value>>
            optional_ptr(optional_ptr<Ptr_> && other) noexcept
                : m_ptr(other.m_ptr)
            {
                other.m_ptr = nullptr;
            }

            optional_ptr& operator = (const optional_ptr&) = delete;

            optional_ptr& operator = (optional_ptr && other) noexcept
            {
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
                return *this;
            }

            template <typename Ptr_, typename = std::enable_if_t<std::is_convertible<Ptr_, value_type>::value>>
            optional_ptr& operator = (optional_ptr<Ptr_> && other) noexcept
            {
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
                return *this;
            }

            constexpr
            this_type clone() const
            {
                return this_type(m_ptr);
            }

            constexpr
            bool empty() const {
                return m_ptr == nullptr;
            }

            constexpr
            operator bool() const {
                return m_ptr != nullptr;
            }

            void swap(this_type & other) noexcept {
                std::swap(m_ptr, other.m_ptr);
            }

            void clear() {
                m_ptr = nullptr;
            }

            constexpr
            value_type get() const {
                if(empty()) {
                    throw yato::bad_optional_access("empty optional");
                }
                return m_ptr;
            }

            constexpr
            value_type get_unsafe() const noexcept {
                YATO_REQUIRES(!empty());
                return m_ptr;
            }

            template <typename DefTy_>
            constexpr
            value_type get_or(DefTy_ && default_value) const {
                return (!empty()) ? m_ptr : static_cast<value_type>(std::forward<DefTy_>(default_value));
            }

            constexpr
            value_type get_or_null() const {
                return get_or(nullptr);
            }

            reference_type deref() const {
                return *get();
            }

            reference_type operator * () const {
                return deref();
            }

            template <typename DefTy_>
            reference_type deref_or(DefTy_ && default_value) const {
                return (!empty()) ? *m_ptr : static_cast<reference_type>(std::forward<DefTy_>(default_value));
            }

            template <typename Function_>
            auto map(Function_ && transform) const &;

            template <typename Ptr_>
            friend class optional_ptr;
        };
        
        template <typename Ty_, bool IsCopy_ = false, bool IsMove_ = false>
        class basic_optional;





        template <typename Ty_, typename = void>
        struct choose_optional
        {
            using type = basic_optional<Ty_, std::is_copy_constructible<Ty_>::value, std::is_move_constructible<Ty_>::value>;
        };
        
        template <typename Ty_>
        struct choose_optional
            <
                Ty_,
                std::enable_if_t<std::is_pointer<Ty_>::value>
            >
        {
            using type = yato::details::optional_ptr<Ty_>;
        };
        
        template <typename Ty_>
        struct choose_optional
            <
                Ty_,
                std::enable_if_t<std::is_reference<Ty_>::value>
            >
        {
            // Reference can't be stored in the optional
        };
        
        template <typename Ty_>
        using choose_optional_t = typename choose_optional<Ty_>::type;






        template <typename Ty_>
        auto make_optional_(Ty_ && val) 
            -> choose_optional_t<std::decay_t<Ty_>>
        {
            return choose_optional_t<std::decay_t<Ty_>>(std::forward<Ty_>(val));
        }



        template <typename Ty_, typename = void>
        struct is_optional
            : std::false_type
        { };

        template <typename Ty_, bool B1_, bool B2_>
        struct is_optional
        <
            basic_optional<Ty_, B1_, B2_>,
            void
        >
            : std::true_type
        { };

        template <typename Ty_>
        struct is_optional
        <
            optional_ptr<Ty_>,
            void
        >
            : std::true_type
        { };


        
        template <typename ValTy_, typename = void>
        struct flatten_op
        {
            template <typename OptTy_>
            static
            OptTy_ apply(OptTy_ && opt)
            {
                return std::forward<OptTy_>(opt);
            }
        };
        
        template <typename ValTy_>
        struct flatten_op
        <
            ValTy_,
            std::enable_if_t<is_optional<ValTy_>::value>
        >
        {
            template <typename OptTy_>
            static
            auto apply(OptTy_ && opt)
            {
                return static_cast<bool>(opt)
                    ? std::forward<OptTy_>(opt).get_unsafe().flatten()
                    : yato::nullopt_t{};
            }
        };




        template <typename Ty_>
        class basic_optional<Ty_, /*IsCopy=*/false, /*IsMove=*/false>
            : public optional_core<Ty_>
        {
        private:
            using super_type = optional_core<Ty_>;

        public:
            using typename super_type::value_type;
            using super_type::is_nothrow_move_constructible;
            using super_type::is_nothrow_move_assignable;

            constexpr
            basic_optional() = default;
    
            ~basic_optional() = default;

            basic_optional(const basic_optional&) = delete;
            basic_optional(basic_optional&&) = delete;

            constexpr
            basic_optional(nullopt_t)
                : super_type()
            { }

            template <typename... Args_>
            constexpr explicit
            basic_optional(yato::in_place_t, Args_ && ... args)
                : super_type(yato::in_place_t{}, std::forward<Args_>(args)...)
            { }

            basic_optional& operator = (const basic_optional&) = delete;
            basic_optional& operator = (basic_optional&&) = delete;

        };



        template <typename Ty_>
        class basic_optional <
                Ty_, /*IsCopy=*/true, /*IsMove=*/false
            >
            : public optional_core<Ty_>
        {
        private:
            using super_type = optional_core<Ty_>;
            using this_type  = basic_optional<Ty_, true, false>;

        public:
            using typename super_type::value_type;
            using super_type::is_nothrow_move_constructible;
            using super_type::is_nothrow_move_assignable;

            constexpr
            basic_optional() = default;
    
            ~basic_optional() = default;

            constexpr
            basic_optional(nullopt_t)
                : super_type()
            { }

            template <typename... Args_>
            constexpr explicit
            basic_optional(yato::in_place_t, Args_ && ... args)
                : super_type(yato::in_place_t{}, std::forward<Args_>(args)...)
            { }

            constexpr
            explicit
            basic_optional(const value_type & val)
                : super_type(yato::in_place_t{}, val)
            { }

            basic_optional(const basic_optional & other)
                : super_type()
            {
                super_type::copy_impl_(other);
            }

            template <typename OtherVal_, bool OtherCopy_, bool OtherMove_, 
                typename = std::enable_if_t<std::is_constructible<value_type, OtherVal_>::value>
            >
            basic_optional(const basic_optional<OtherVal_, OtherCopy_, OtherMove_> & other)
                : super_type()
            {
                super_type::copy_impl_(other);
            }

            basic_optional & operator= (const basic_optional & other) {
                super_type::assign_impl_(other);
                return *this;
            }

            template <typename OtherVal_, bool OtherCopy_, bool OtherMove_, 
                typename = std::enable_if_t<std::is_constructible<value_type, OtherVal_>::value>
            >
            basic_optional & operator= (const basic_optional<OtherVal_, OtherCopy_, OtherMove_> & other) {
                super_type::assign_impl_(other);
                return *this;
            }

            void swap(this_type & other)
            {
                super_type::swap_impl_(other);
            }

            template <typename Function_>
            auto map(Function_ && transform) const &
            {
                return (!super_type::empty())
                    ? make_optional_(transform(super_type::get_unsafe()))
                    : nullopt_t{};
            }

            auto flatten() const &
            {
                return flatten_op<value_type>::apply(*this);
            }

            auto flatten() &&
            {
                return flatten_op<value_type>::apply(std::move(*this));
            }
        };



        template <typename Ty_>
        class basic_optional <
                Ty_, /*IsCopy=*/false, /*IsMove=*/true
            >
            : public optional_core<Ty_>
        {
        private:
            using super_type = optional_core<Ty_>;
            using this_type  = basic_optional<Ty_, false, true>;

        public:
            using typename super_type::value_type;
            using super_type::is_nothrow_move_constructible;
            using super_type::is_nothrow_move_assignable;

            constexpr
            basic_optional() = default;
    
            ~basic_optional() = default;

            constexpr
            basic_optional(nullopt_t)
                : super_type()
            { }

            template <typename... Args_>
            constexpr explicit
            basic_optional(yato::in_place_t, Args_ && ... args)
                : super_type(yato::in_place_t{}, std::forward<Args_>(args)...)
            { }

            explicit
            basic_optional(value_type && val)
                : super_type(yato::in_place_t{}, std::move(val))
            { }

            basic_optional(const basic_optional&) = delete;

            basic_optional(basic_optional && other) noexcept(is_nothrow_move_constructible)
                : super_type()
            {
                super_type::move_copy_impl_(std::move(other));
            }

            template <typename OtherVal_, bool OtherCopy_, bool OtherMove_, 
                typename = std::enable_if_t<std::is_constructible<value_type, typename std::add_rvalue_reference<OtherVal_>::type>::value>
            >
            basic_optional(basic_optional<OtherVal_, OtherCopy_, OtherMove_> && other)
                : super_type()
            {
                super_type::move_copy_impl_(std::move(other));
            }

            basic_optional & operator= (const basic_optional&) = delete;

            basic_optional & operator= (basic_optional && other) noexcept(is_nothrow_move_assignable) {
                super_type::move_assign_impl_(std::move(other));
                return *this;
            }
    
            template <typename OtherVal_, bool OtherCopy_, bool OtherMove_, 
                typename = std::enable_if_t<std::is_constructible<value_type, typename std::add_rvalue_reference<OtherVal_>::type>::value>
            >
            basic_optional & operator= (basic_optional<OtherVal_, OtherCopy_, OtherMove_> && other) {
                super_type::move_assign_impl_(std::move(other));
                return *this;
            }

            void swap(this_type & other)
            {
                super_type::swap_impl_(other);
            }

            template <typename Function_>
            auto map(Function_ && transform) const &
            {
                return (!super_type::empty())
                    ? make_optional_(transform(super_type::get_unsafe()))
                    : nullopt_t{};
            }
            
            template <typename Function_>
            auto map(Function_ && transform) &&
            {
                return (!super_type::empty())
                    ? make_optional_(transform(std::move(super_type::get_unsafe())))
                    : nullopt_t{};
            }

            auto flatten() &&
            {
                return flatten_op<value_type>::apply(std::move(*this));
            }
        };



        template <typename Ty_>
        class basic_optional <
                Ty_, /*IsCopy=*/true, /*IsMove=*/true
            >
            : public optional_core<Ty_>
        {
        private:
            using super_type = optional_core<Ty_>;
            using this_type  = basic_optional<Ty_, true, true>;

        public:
            using typename super_type::value_type;
            using super_type::is_nothrow_move_constructible;
            using super_type::is_nothrow_move_assignable;

            constexpr
            basic_optional() = default;
    
            ~basic_optional() = default;

            constexpr
            basic_optional(nullopt_t)
                : super_type()
            { }

            template <typename... Args_>
            constexpr explicit
            basic_optional(yato::in_place_t, Args_ && ... args)
                : super_type(yato::in_place_t{}, std::forward<Args_>(args)...)
            { }

            constexpr explicit
            basic_optional(const value_type & val)
                : super_type(yato::in_place_t{}, val)
            { }

            explicit
            basic_optional(value_type && val)
                : super_type(yato::in_place_t{}, std::move(val))
            { }
    
            basic_optional(const basic_optional & other)
                : super_type()
            {
                super_type::copy_impl_(other);
            }
    
            template <typename OtherVal_, bool OtherCopy_, bool OtherMove_, 
                typename = std::enable_if_t<std::is_constructible<value_type, OtherVal_>::value>
            >
            basic_optional(const basic_optional<OtherVal_, OtherCopy_, OtherMove_> & other)
                : super_type()
            {
                super_type::copy_impl_(other);
            }

            basic_optional(basic_optional && other) noexcept(is_nothrow_move_constructible)
                : super_type()
            {
                super_type::move_copy_impl_(std::move(other));
            }

            template <typename OtherVal_, bool OtherCopy_, bool OtherMove_, 
                typename = std::enable_if_t<std::is_constructible<value_type, typename std::add_rvalue_reference<OtherVal_>::type>::value>
            >
            basic_optional(basic_optional<OtherVal_, OtherCopy_, OtherMove_> && other)
                : super_type()
            {
                super_type::move_copy_impl_(std::move(other));
            }

            basic_optional & operator= (const basic_optional & other) {
                super_type::assign_impl_(other);
                return *this;
            }

            template <typename OtherVal_, bool OtherCopy_, bool OtherMove_, 
                typename = std::enable_if_t<std::is_constructible<value_type, OtherVal_>::value>
            >
            basic_optional & operator= (const basic_optional<OtherVal_, OtherCopy_, OtherMove_> & other) {
                super_type::assign_impl_(other);
                return *this;
            }

            basic_optional & operator= (basic_optional && other) noexcept(is_nothrow_move_assignable) {
                super_type::move_assign_impl_(std::move(other));
                return *this;
            }
    
            template <typename OtherVal_, bool OtherCopy_, bool OtherMove_, 
                typename = std::enable_if_t<std::is_constructible<value_type, typename std::add_rvalue_reference<OtherVal_>::type>::value>
            >
            basic_optional & operator= (basic_optional<OtherVal_, OtherCopy_, OtherMove_> && other) {
                super_type::move_assign_impl_(std::move(other));
                return *this;
            }

            void swap(this_type & other)
            {
                super_type::swap_impl_(other);
            }

            template <typename Function_>
            auto map(Function_ && transform) const &
            {
                return (!super_type::empty())
                    ? make_optional_(transform(super_type::get_unsafe()))
                    : nullopt_t{};
            }
            
            template <typename Function_>
            auto map(Function_ && transform) &&
            {
                return (!super_type::empty())
                    ? make_optional_(transform(std::move(super_type::get_unsafe())))
                    : nullopt_t{};
            }

            auto flatten() const &
            {
                return flatten_op<value_type>::apply(*this);
            }

            auto flatten() &&
            {
                return flatten_op<value_type>::apply(std::move(*this));
            }
        };



        template <typename Ty_>
        template <typename Function_>
        auto optional_ptr<Ty_>::map(Function_ && transform) const &
        {
            return (!empty())
                ? make_optional_(transform(get_unsafe()))
                : nullopt_t{};
        }
        
    }

    /**
     * Container for optional value.
     * Provides specific no-copyable overload for stored pointer types.
     */
    template <typename Ty_>
    using optional = details::choose_optional_t<Ty_>;

    /**
     * Check is type is optional
     */
    template <typename Ty_>
    struct is_optional
        : details::is_optional<Ty_>
    { };

    template <typename Ty_>
    void swap(optional<Ty_> & lhs, optional<Ty_> & rhs)
    {
        lhs.swap(rhs);
    }

    template <typename Ty_>
    optional<std::decay_t<Ty_>> make_optional(Ty_ && val) {
        return optional<std::decay_t<Ty_>>(std::forward<Ty_>(val));
    }



} // namespace yato

#endif

