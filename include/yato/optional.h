/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
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

            constexpr
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
                return *get_ptr_();
            }

            constexpr
            value_type && get() && {
                if(!m_stored) {
                    throw yato::bad_optional_access("empty optional");
                }
                return *get_ptr_();
            }

            constexpr
            const value_type & get_unsafe() const noexcept {
                return *get_ptr_();
            }

            constexpr
            value_type & get_unsafe() noexcept {
                return *get_ptr_();
            }

            template <typename DefTy_>
            constexpr
            value_type get_or(DefTy_ && default_value) const & {
                return m_stored ? *get_ptr_() : static_cast<value_type>(std::forward<DefTy_>(default_value));
            }

            template <typename DefTy_>
            constexpr
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

        template <typename Ty_, bool IsCopy_ = false, bool IsMove_ = false>
        class basic_optional;

        template <typename Ty_>
        using choose_optional = basic_optional<Ty_, std::is_copy_constructible<Ty_>::value, std::is_move_constructible<Ty_>::value>;

        template <typename Ty_>
        choose_optional<Ty_> make_optional_(Ty_ && val) {
            return choose_optional<Ty_>(std::forward<Ty_>(val));
        }

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

            basic_optional(basic_optional &&) = delete;

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

            basic_optional & operator= (basic_optional &&) = delete;

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

            constexpr
            basic_optional(const value_type & val)
                : super_type(yato::in_place_t{}, val)
            { }

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
        };

    }

    template <typename Ty_>
    using optional = details::choose_optional<Ty_>;

    template <typename Ty_>
    void swap(optional<Ty_> & lhs, optional<Ty_> & rhs)
    {
        lhs.swap(rhs);
    }

    template <typename Ty_>
    optional<Ty_> make_optional(Ty_ && val) {
        return optional<Ty_>(std::forward<Ty_>(val));
    }



} // namespace yato

#endif

