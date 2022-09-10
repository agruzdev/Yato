/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_VARIANT_H_
#define _YATO_VARIANT_H_

#include "meta.h"
#include "types.h"
#include "memory_utility.h"
#include "optional.h"

namespace yato
{

#ifndef YATO_VARIANT_INDEX_TYPE
# define YATO_VARIANT_INDEX_TYPE uint8_t
#endif

    using variant_index_t = YATO_VARIANT_INDEX_TYPE;

    namespace details
    {
        static YATO_CONSTEXPR_VAR const variant_index_t variant_no_index  = std::numeric_limits<variant_index_t>::max();
        static YATO_CONSTEXPR_VAR const variant_index_t variant_max_index = std::numeric_limits<variant_index_t>::max() - 1;


        template <typename Ty_>
        struct sizeof_ext
            : public std::integral_constant<size_t, sizeof(Ty_)>
        { };

        template <>
        struct sizeof_ext<void>
            : public std::integral_constant<size_t, 1>
        { };

        template <typename Ty_>
        struct alignof_ext
            : public std::integral_constant<size_t, std::alignment_of<Ty_>::value>
        { };

        template <>
        struct alignof_ext<void>
            : public std::integral_constant<size_t, 1>
        { };

        template <size_t X_, size_t Y_>
        struct max_helper
            : public std::integral_constant<size_t, (X_ > Y_) ? X_ : Y_>
        { };


        template <typename TypesList_>
        struct max_types_size
            : public std::integral_constant<size_t, max_helper<
                sizeof_ext<typename TypesList_::head>::value,
                max_types_size<typename TypesList_::tail>::value
            >::value>
        { };

        template <>
        struct max_types_size<yato::meta::null_list>
            : public std::integral_constant<size_t, 1>
        { };


        template <typename TypesList_>
        struct max_types_alignment
            : public std::integral_constant<size_t, max_helper<
                alignof_ext<typename TypesList_::head>::value,
                max_types_alignment<typename TypesList_::tail>::value
            >::value>
        { };

        template <>
        struct max_types_alignment<yato::meta::null_list>
            : public std::integral_constant<size_t, 1>
        { };


        /**
         * Check if all types are copyable
         */
        template <typename TypesList_>
        struct all_copy_constructible
            : public std::integral_constant<bool,
                (std::is_same<void, typename TypesList_::head>::value || std::is_copy_constructible<typename TypesList_::head>::value) &&
                all_copy_constructible<typename TypesList_::tail>::value
            >
        { };

        template <>
        struct all_copy_constructible<yato::meta::null_list>
            : public std::integral_constant<bool, true>
        { };


        template <typename TypesList_>
        struct all_copy_assignable
            : public std::integral_constant<bool,
                (std::is_same<void, typename TypesList_::head>::value || std::is_copy_assignable<typename TypesList_::head>::value) &&
                all_copy_assignable<typename TypesList_::tail>::value
            >
        { };

        template <>
        struct all_copy_assignable<yato::meta::null_list>
            : public std::integral_constant<bool, true>
        { };



        /**
         * Check if all types are movable
         */
        template <typename TypesList_>
        struct all_move_constructible
            : public std::integral_constant<bool,
                (std::is_same<void, typename TypesList_::head>::value || std::is_move_constructible<typename TypesList_::head>::value) &&
                all_move_constructible<typename TypesList_::tail>::value
            >
        { };

        template <>
        struct all_move_constructible<yato::meta::null_list>
            : public std::integral_constant<bool, true>
        { };


        template <typename TypesList_>
        struct all_nothrow_move_constructible
            : public std::integral_constant<bool,
                (std::is_same<void, typename TypesList_::head>::value || std::is_nothrow_move_constructible<typename TypesList_::head>::value) &&
                all_nothrow_move_constructible<typename TypesList_::tail>::value
            >
        { };

        template <>
        struct all_nothrow_move_constructible<yato::meta::null_list>
            : public std::integral_constant<bool, true>
        { };


        template <typename TypesList_>
        struct all_move_assignable
            : public std::integral_constant<bool,
                (std::is_same<void, typename TypesList_::head>::value || std::is_move_assignable<typename TypesList_::head>::value) &&
                all_move_assignable<typename TypesList_::tail>::value
            >
        { };

        template <>
        struct all_move_assignable<yato::meta::null_list>
            : public std::integral_constant<bool, true>
        { };


        template <typename TypesList_>
        struct all_nothrow_move_assignable
            : public std::integral_constant<bool,
                (std::is_same<void, typename TypesList_::head>::value || std::is_nothrow_move_assignable<typename TypesList_::head>::value) &&
                all_nothrow_move_assignable<typename TypesList_::tail>::value
            >
        { };

        template <>
        struct all_nothrow_move_assignable<yato::meta::null_list>
            : public std::integral_constant<bool, true>
        { };


        class bad_variant_access
            : public std::runtime_error
        {
        public:
            bad_variant_access(const char* what)
                : std::runtime_error(what)
            { }
        };


        struct nullvar_t {};


        template <typename Ty_>
        struct variant_create
        {
            template <typename... Args_>
            static
            void apply(void* ptr, Args_&&... args)
            {
                new(ptr) Ty_(std::forward<Args_>(args)...);
            }
        };

        template <>
        struct variant_create<void>
        {
            template <typename... Args_>
            static
            void apply(void* /*ptr*/)
            { }
        };


        template <typename Ty_>
        struct variant_destroy
        {
            static
            void apply(void* ptr)
            {
                yato::launder(yato::pointer_cast<Ty_*>(ptr))->~Ty_();
            }
        };

        template <>
        struct variant_destroy<void>
        {
            static
            void apply(void* /*ptr*/)
            { }
        };


        template <typename Ty_, typename = 
            yato::boolean_constant<std::is_copy_constructible<Ty_>::value>
        >
        struct variant_copy_construct
        {
            static
            void apply(const void* src, void* dst)
            {
                const Ty_& other = *yato::launder(yato::pointer_cast<const Ty_*>(src));
                variant_create<Ty_>::apply(dst, other);
            }
        };

        template <typename Ty_>
        struct variant_copy_construct<Ty_, std::false_type>
        {
            static
            void apply(const void* /*src*/, void* /*dst*/)
            { }
        };


        template <typename Ty_, typename = 
            yato::boolean_constant<std::is_copy_assignable<Ty_>::value>
        >
        struct variant_copy_assign
        {
            static
            void apply(const void* src, void* dst)
            {
                Ty_& dst_ref = *yato::launder(yato::pointer_cast<Ty_*>(dst));
                const Ty_& src_ref = *yato::launder(yato::pointer_cast<const Ty_*>(src));
                dst_ref = src_ref;
            }
        };

        template <typename Ty_>
        struct variant_copy_assign<Ty_, std::false_type>
        {
            static
            void apply(const void* /*src*/, void* /*dst*/)
            { }
        };


        template <typename Ty_, typename = 
            yato::boolean_constant<std::is_move_constructible<Ty_>::value>
        >
        struct variant_move_construct
        {
            static
            void apply(void* src, void* dst)
            {
                Ty_& other = *yato::launder(yato::pointer_cast<Ty_*>(src));
                variant_create<Ty_>::apply(dst, std::move(other));
            }
        };

        template <typename Ty_>
        struct variant_move_construct<Ty_, std::false_type>
        {
            static
            void apply(void* /*src*/, void* /*dst*/)
            { }
        };


        template <typename Ty_, typename = 
            yato::boolean_constant<std::is_move_assignable<Ty_>::value>
        >
        struct variant_move_assign
        {
            static
            auto apply(void* src, void* dst)
            {
                Ty_& dst_ref = *yato::launder(yato::pointer_cast<Ty_*>(dst));
                Ty_& src_ref = *yato::launder(yato::pointer_cast<Ty_*>(src));
                dst_ref = std::move(src_ref);
            }
        };

        template <typename Ty_>
        struct variant_move_assign<Ty_, std::false_type>
        {
            static
            void apply(void* /*src*/, void* /*dst*/)
            { }
        };


        template <typename Ty_>
        struct variant_get_type
        {
            static
            const std::type_info& apply()
            {
                return typeid(Ty_);
            }
        };



        template <template <typename, typename...> class Operation_, typename... AltTypes_>
        struct variant_dispatch_table
        {
            static constexpr std::array<typename yato::callable_trait<decltype(&Operation_<void>::apply)>::pointer_type, sizeof...(AltTypes_)> apply = { &Operation_<AltTypes_>::apply... };
        };

        template <typename AltList_, template <typename, typename...> class Operation_, typename... Types_>
        struct variant_dispatch_impl
        {
            using type = typename variant_dispatch_impl<typename AltList_::tail, Operation_, Types_..., typename AltList_::head>::type;
        };

        template <template <typename, typename...> class Operation_, typename... Types_>
        struct variant_dispatch_impl<yato::meta::null_list, Operation_, Types_...>
        {
            using type = variant_dispatch_table<Operation_, Types_...>;
        };

        template <typename AltList_, template <typename, typename...> class Operation_>
        using variant_dispatch = typename variant_dispatch_impl<AltList_, Operation_>::type;


        /**
         *  Type-safe union. Holds one of possible alternatives
         *  Each alternative type should be CopyConstructible or void
         *  void alternative shows that variant can hold no value
         */
        template <typename AltsList_>
        class basic_variant
        {
        private:
            using this_type = basic_variant<AltsList_>;
        public:
            using alternativies_list = AltsList_;

            using storage_type = std::aligned_storage_t<details::max_types_size<alternativies_list>::value, details::max_types_alignment<alternativies_list>::value>;

            static YATO_CONSTEXPR_VAR size_t alternativies_number = yato::meta::list_length<alternativies_list>::value;

            static_assert(alternativies_number > 0, "yato::variant: alternatives must be not empty.");
            static_assert(alternativies_number <= details::variant_max_index, "yato::variant: too many alternatives.");
            static_assert(std::is_same<typename meta::list_unique<alternativies_list>::type, alternativies_list>::value, "yato::variant: alternatives must have unique types");

            /**
             * Variant can be null if the 'void' type is presented in the alternatives list
             */
            static YATO_CONSTEXPR_VAR bool is_nullable = (meta::list_find<AltsList_, void>::value != yato::meta::list_npos);

            static YATO_CONSTEXPR_VAR bool is_copy_constructible_v = details::all_copy_constructible<alternativies_list>::value;
            static YATO_CONSTEXPR_VAR bool is_move_constructible_v = details::all_move_constructible<alternativies_list>::value;
            static YATO_CONSTEXPR_VAR bool is_copy_assignable_v    = is_copy_constructible_v && details::all_copy_assignable<alternativies_list>::value;
            static YATO_CONSTEXPR_VAR bool is_move_assignable_v    = is_move_constructible_v && details::all_move_assignable<alternativies_list>::value;

            static YATO_CONSTEXPR_VAR bool is_nothrow_move_constructible_v = details::all_nothrow_move_constructible<alternativies_list>::value;
            static YATO_CONSTEXPR_VAR bool is_nothrow_move_assignable_v    = is_nothrow_move_constructible_v && details::all_nothrow_move_assignable<alternativies_list>::value;

            // ToDo (a.gruzdev): Add swap()
            //static constexpr bool is_swappable_v         = this_type::is_move_constructible_v && this_type::is_move_assignable_v;
            //static constexpr bool is_nothrow_swappable_v = this_type::is_nothrow_move_assignable_v;

            //--------------------------------------------------------------------
        private:
            storage_type m_storage{};
            variant_index_t m_type_idx{ details::variant_no_index };

            //--------------------------------------------------------------------

            template <typename Ty_>
            const Ty_* caddress_() const
            {
                return yato::launder(yato::pointer_cast<const Ty_*>(&m_storage));
            }

            template <typename Ty_>
            Ty_* address_()
            {
                return yato::launder(yato::pointer_cast<Ty_*>(&m_storage));
            }

            const void* storage_caddress_() const
            {
                return &m_storage;
            }

            void* storage_address_()
            {
                return &m_storage;
            }

            void copy_construct_(const this_type& other)
            {
                variant_dispatch<alternativies_list, variant_copy_construct>::apply[other.m_type_idx](other.storage_caddress_(), storage_address_());
                m_type_idx = other.m_type_idx;
            }

            void copy_construct_(yato::disabled_t)
            {
                YATO_ASSERT(false, "Disabled method should not be invoked.");
            }

            void copy_assign_(const this_type& other)
            {
                YATO_REQUIRES(this != &other);
                YATO_REQUIRES(m_type_idx != details::variant_no_index);
                if (m_type_idx == other.m_type_idx) {
                    variant_dispatch<alternativies_list, variant_copy_assign>::apply[m_type_idx](other.storage_caddress_(), storage_address_());
                }
                else {
                    variant_dispatch<alternativies_list, variant_destroy>::apply[m_type_idx](storage_address_());
                    m_type_idx = details::variant_no_index;
                    variant_dispatch<alternativies_list, variant_copy_construct>::apply[other.m_type_idx](other.storage_caddress_(), storage_address_());
                    m_type_idx = other.m_type_idx;
                }
            }

            void copy_assign_(yato::disabled_t)
            {
                YATO_ASSERT(false, "Disabled method should not be invoked.");
            }

            void move_construct_(this_type&& other) YATO_NOEXCEPT_OPERATOR(is_nothrow_move_constructible_v)
            {
                variant_dispatch<alternativies_list, variant_move_construct>::apply[other.m_type_idx](other.storage_address_(), storage_address_());
                m_type_idx = other.m_type_idx;
            }

            void move_construct_(yato::disabled_t) YATO_NOEXCEPT_KEYWORD
            {
                YATO_ASSERT(false, "Disabled method should not be invoked.");
            }

            void move_assign_(this_type&& other) YATO_NOEXCEPT_OPERATOR(is_nothrow_move_assignable_v)
            {
                YATO_REQUIRES(this != &other);
                YATO_REQUIRES(m_type_idx != details::variant_no_index);
                if (m_type_idx == other.m_type_idx) {
                    variant_dispatch<alternativies_list, variant_move_assign>::apply[m_type_idx](other.storage_address_(), storage_address_());
                }
                else {
                    variant_dispatch<alternativies_list, variant_destroy>::apply[m_type_idx](storage_address_());
                    m_type_idx = details::variant_no_index;
                    variant_dispatch<alternativies_list, variant_move_construct>::apply[other.m_type_idx](other.storage_address_(), storage_address_());
                    m_type_idx = other.m_type_idx;
                }
            }

            void move_assign_(yato::disabled_t) YATO_NOEXCEPT_KEYWORD
            {
                YATO_ASSERT(false, "Disabled method should not be invoked.");
            }

            YATO_CONSTEXPR_FUNC
            bool is_empty_(yato::boolean_constant<true> /*is_nullable*/) const YATO_NOEXCEPT_KEYWORD
            {
                return m_type_idx == meta::list_find<alternativies_list, void>::value;
            }

            YATO_CONSTEXPR_FUNC
            bool is_empty_(yato::boolean_constant<false> /*is_nullable*/) const YATO_NOEXCEPT_KEYWORD
            {
                return false;
            }

            YATO_CONSTEXPR_FUNC
            bool is_empty_() const YATO_NOEXCEPT_KEYWORD
            {
                return is_empty_(yato::boolean_constant<is_nullable>{});
            }

            //--------------------------------------------------------------------

        public:
            /**
             *  Creates empty variant
             *  As is void is stored
             */
            template <typename ThisType_ = this_type, typename = 
                std::enable_if_t<ThisType_::is_nullable>
            >
            YATO_CONSTEXPR_FUNC
            basic_variant(nullvar_t = nullvar_t{}) YATO_NOEXCEPT_KEYWORD
                : m_type_idx(yato::meta::list_find<typename ThisType_::alternativies_list, void>::value)
            { }

            /**
             *  Create variant from value
             */
            template <typename Ty_, typename = 
                std::enable_if_t<(yato::meta::list_find<alternativies_list, yato::remove_cvref_t<Ty_>>::value != yato::meta::list_npos)>
            >
            explicit YATO_CONSTEXPR_FUNC_CXX14
            basic_variant(Ty_ && value)
                : m_type_idx(yato::meta::list_find<alternativies_list, yato::remove_cvref_t<Ty_>>::value)
            {
                details::variant_create<yato::remove_cvref_t<Ty_>>::apply(storage_address_(), std::forward<Ty_>(value));
            }

            /**
             *  Create variant in place
             */
            template <typename Ty_,
                typename = std::enable_if_t<(yato::meta::list_find<alternativies_list, Ty_>::value != yato::meta::list_npos)>,
                typename... Args_
            >
            explicit YATO_CONSTEXPR_FUNC_CXX14
            basic_variant(yato::in_place_type_t<Ty_>, Args_&&... args)
                : m_type_idx(yato::meta::list_find<alternativies_list, Ty_>::value)
            {
                details::variant_create<Ty_>::apply(storage_address_(), std::forward<Args_>(args)...);
            }

            /**
             *  Create variant in place
             */
            template <size_t Idx_,
                typename = std::enable_if_t<(Idx_ < yato::meta::list_length<alternativies_list>::value)>, 
                typename... Args_
            >
            explicit YATO_CONSTEXPR_FUNC_CXX14
            basic_variant(yato::in_place_index_t<Idx_>, Args_&&... args)
                : m_type_idx(Idx_)
            {
                details::variant_create<yato::meta::list_at_t<alternativies_list, Idx_>>::apply(storage_address_(), std::forward<Args_>(args)...);
            }

            YATO_CONSTEXPR_FUNC_CXX14
            basic_variant(const yato::disable_if_not_t<this_type::is_copy_constructible_v, this_type>& other)
            {
                copy_construct_(other);
            }

            YATO_CONSTEXPR_FUNC_CXX14
            basic_variant(yato::disable_if_not_t<this_type::is_move_constructible_v, this_type>&& other) YATO_NOEXCEPT_OPERATOR(is_nothrow_move_constructible_v)
            {
                move_construct_(std::move(other));
            }

            ~basic_variant()
            {
                if (m_type_idx != details::variant_no_index) {
                    variant_dispatch<alternativies_list, variant_destroy>::apply[m_type_idx](storage_address_());
                }
            }

            /**
             *  If stored types of both variants are same, then calls copy assignment operator
             *  If the types are different or copy assignment is not available, then destroys current instance and creates copy
             */
            YATO_CONSTEXPR_FUNC_CXX14
            this_type& operator=(const yato::disable_if_not_t<this_type::is_copy_assignable_v, this_type>& other)
            {
                copy_assign_(other);
                return *this;
            }

            /**
             *  If stored types of both variants are same, then calls move assignment operator
             *  If the types are different or copy assignment is not available, then destroys current instance and creates moved copy
             */
            YATO_CONSTEXPR_FUNC_CXX14
            this_type& operator=(yato::disable_if_not_t<this_type::is_move_assignable_v, this_type>&& other) YATO_NOEXCEPT_OPERATOR(is_nothrow_move_assignable_v)
            {
                move_assign_(std::move(other));
                return *this;
            }

            /**
             * Returns true if stored type is not 'void'
             */
            YATO_CONSTEXPR_FUNC
            operator bool() const YATO_NOEXCEPT_KEYWORD
            {
                return !is_empty_();
            }

            /**
             *  Replaces content of the variant with a new type 
             *  Calling with Ty = void will clear the variant
             */
            template <typename Ty_,
                typename = std::enable_if_t<(yato::meta::list_find<alternativies_list, Ty_>::value != yato::meta::list_npos)>,
                typename... Args_
            >
            YATO_CONSTEXPR_FUNC_CXX14
            void emplace(Args_&& ... args)
            {
                YATO_REQUIRES(m_type_idx != details::variant_no_index);
                variant_dispatch<alternativies_list, variant_destroy>::apply[m_type_idx](storage_address_());
                m_type_idx = details::variant_no_index;
                details::variant_create<Ty_>::apply(storage_address_(), std::forward<Args_>(args)...);
                m_type_idx = yato::meta::list_find<alternativies_list, Ty_>::value;
            }

            /**
             *  Get typeid of the currently stored type
             *  Returns typeid(void) if variant is empty
             */
            const std::type_info & type() const YATO_NOEXCEPT_KEYWORD
            {
                return variant_dispatch<alternativies_list, variant_get_type>::apply[m_type_idx]();
            }

            /**
             * Get stored type index
             */
            YATO_CONSTEXPR_FUNC
            size_t type_index() const YATO_NOEXCEPT_KEYWORD
            {
                return m_type_idx;
            }

            /**
             * Check stored type
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC
            bool is_type() const YATO_NOEXCEPT_KEYWORD
            {
                return (yato::meta::list_find<alternativies_list, Ty_>::value == m_type_idx);
            }

            /**
             * Returns true if stored type is 'void'
             */
            YATO_CONSTEXPR_FUNC
            bool empty() const YATO_NOEXCEPT_KEYWORD
            {
                return is_empty_();
            }

            /**
             *  Get value by type index
             *  On error throws bad_variant_access
             */
            template <size_t Idx_>
            YATO_CONSTEXPR_FUNC_CXX14
            auto get() &
                -> std::enable_if_t<(Idx_ < alternativies_number), yato::meta::list_at_t<alternativies_list, Idx_>&>
            {
                if (m_type_idx == Idx_) {
                    return *address_<yato::meta::list_at_t<alternativies_list, Idx_>>();
                }
                else {
                    throw bad_variant_access("yato::variant_bad_access: Stored type differs from the type by given index");
                }
            }

            /**
             *  Get value by type index
             *  On error throws bad_variant_access 
             */
            template <size_t Idx_>
            YATO_CONSTEXPR_FUNC_CXX14
            auto get() const &
                -> std::enable_if_t<(Idx_ < alternativies_number), const yato::meta::list_at_t<alternativies_list, Idx_>&>
            {
                if (m_type_idx == Idx_) {
                    return *caddress_<yato::meta::list_at_t<alternativies_list, Idx_>>();
                }
                else {
                    throw bad_variant_access("yato::variant_bad_access: Stored type differs from the type by given index");
                }
            }

            /**
             *  Get value by type index
             *  On error throws bad_variant_access
             */
            template <size_t Idx_>
            YATO_CONSTEXPR_FUNC_CXX14
            auto get() &&
                -> std::enable_if_t<(Idx_ < alternativies_number), yato::meta::list_at_t<alternativies_list, Idx_>&&>
            {
                if (m_type_idx == Idx_) {
                    return std::move(*address_<yato::meta::list_at_t<alternativies_list, Idx_>>());
                }
                else {
                    throw bad_variant_access("yato::variant_bad_access: Stored type differs from the type by given index");
                }
            }

            /**
             *  Get value by type index
             *  On error throws bad_variant_access 
             */
            template <size_t Idx_>
            YATO_CONSTEXPR_FUNC_CXX14
            auto get() const &&
                -> std::enable_if_t<(Idx_ < alternativies_number), const yato::meta::list_at_t<alternativies_list, Idx_>&&>
            {
                if (m_type_idx == Idx_) {
                    return std::move(*caddress_<yato::meta::list_at_t<alternativies_list, Idx_>>());
                }
                else {
                    throw bad_variant_access("yato::variant_bad_access: Stored type differs from the type by given index");
                }
            }


            /**
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            Ty_& get() &
            {
                return get<yato::meta::list_find<alternativies_list, Ty_>::value>();
            }

            /**
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            const Ty_& get() const &
            {
                return get<yato::meta::list_find<alternativies_list, Ty_>::value>();
            }

            /**
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            Ty_&& get() &&
            {
                return std::move(get<yato::meta::list_find<alternativies_list, Ty_>::value>());
            }

            /**
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            const Ty_&& get() const &&
            {
                return std::move(get<yato::meta::list_find<alternativies_list, Ty_>::value>());
            }


            /**
             *  Get value by type index
             *  On error returns default value
             */
            template <size_t Idx_, typename Uy_>
            YATO_CONSTEXPR_FUNC_CXX14
            auto get_or(Uy_&& default_value) const & YATO_NOEXCEPT_KEYWORD
                -> std::enable_if_t<(Idx_ < alternativies_number), yato::meta::list_at_t<alternativies_list, Idx_>>
            {
                if (m_type_idx == Idx_) {
                    return *caddress_<yato::meta::list_at_t<alternativies_list, Idx_>>();
                }
                else {
                    return static_cast<yato::meta::list_at_t<alternativies_list, Idx_>>(std::forward<Uy_>(default_value));
                }
            }

            /**
             *  Get value by type index
             *  On error returns default value
             */
            template <size_t Idx_, typename Uy_>
            YATO_CONSTEXPR_FUNC_CXX14
            auto get_or(Uy_&& default_value) && YATO_NOEXCEPT_KEYWORD
                -> std::enable_if_t<(Idx_ < alternativies_number), yato::meta::list_at_t<alternativies_list, Idx_>>
            {
                if (m_type_idx == Idx_) {
                    return std::move(*address_<yato::meta::list_at_t<alternativies_list, Idx_>>());
                }
                else {
                    return static_cast<yato::meta::list_at_t<alternativies_list, Idx_>>(std::forward<Uy_>(default_value));
                }
            }

            /**
             *  Get value by type index
             *  On error returns default value
             */
            template <typename Ty_, typename Uy_>
            YATO_CONSTEXPR_FUNC_CXX14
            Ty_ get_or(Uy_&& default_value) const & YATO_NOEXCEPT_KEYWORD
            {
                return get_or<yato::meta::list_find<alternativies_list, Ty_>::value>(std::forward<Uy_>(default_value));
            }

            /**
             *  Get value by type index
             *  On error returns default value
             */
            template <typename Ty_, typename Uy_>
            YATO_CONSTEXPR_FUNC_CXX14
            Ty_ get_or(Uy_&& default_value) && YATO_NOEXCEPT_KEYWORD
            {
                return get_or<yato::meta::list_find<alternativies_list, Ty_>::value>(std::forward<Uy_>(default_value));
            }


            /**
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            auto get_opt() const & YATO_NOEXCEPT_KEYWORD
                -> std::enable_if_t<(yato::meta::list_find<alternativies_list, Ty_>::value != yato::meta::list_npos), yato::optional<Ty_>>
            {
                if (is_type<Ty_>()) {
                    return yato::make_optional(*caddress_<Ty_>());
                } else {
                    return yato::nullopt_t{};
                }
            }

            /**
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            auto get_opt() && YATO_NOEXCEPT_KEYWORD
                -> std::enable_if_t<(yato::meta::list_find<alternativies_list, Ty_>::value != yato::meta::list_npos), yato::optional<Ty_>>
            {
                if (is_type<Ty_>()) {
                    return yato::make_optional(std::move(*address_<Ty_>()));
                } else {
                    return yato::nullopt_t{};
                }
            }


            /**
             *  Get value without check
             *  Use only if you are sure what is stored type
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            Ty_& get_unsafe() & YATO_NOEXCEPT_KEYWORD
            {
                return *address_<Ty_>();
            }

            /**
             *  Get value without check
             *  Use only if you are sure what is stored type
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            const Ty_& get_unsafe() const & YATO_NOEXCEPT_KEYWORD
            {
                return *caddress_<Ty_>();
            }

             /**
             *  Get value without check
             *  Use only if you are sure what is stored type
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            Ty_&& get_unsafe() && YATO_NOEXCEPT_KEYWORD
            {
                return static_cast<Ty_&&>(*address_<Ty_>());
            }

            /**
             *  Get value without check
             *  Use only if you are sure what is stored type
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC_CXX14
            const Ty_&& get_unsafe() const && YATO_NOEXCEPT_KEYWORD
            {
                return static_cast<const Ty_&&>(*caddress_<Ty_>());
            }

        };


        class bad_variant_cast
            : public std::bad_cast
        {
        public:
            const char* what() const YATO_NOEXCEPT_KEYWORD override
            {
                return "yato::bad_variant_cast: failed conversion using yato::variant_cast";
            }
        };


        template <typename AltsDst_, typename AltsSrc_, typename StoredTy_, typename = void>
        struct variant_cast_create
        {
            basic_variant<AltsDst_> create(const basic_variant<AltsSrc_> &) {
                throw bad_variant_cast{};
            }

            basic_variant<AltsDst_> create(basic_variant<AltsSrc_> &&) {
                throw bad_variant_cast{};
            }
        };

        template <typename AltsDst_, typename AltsSrc_, typename StoredTy_>
        struct variant_cast_create <
            AltsDst_, AltsSrc_, StoredTy_,
            std::enable_if_t<(meta::list_find<AltsDst_, StoredTy_>::value != meta::list_npos) && (!std::is_same<void, StoredTy_>::value)>
        >
        {
            basic_variant<AltsDst_> create(const basic_variant<AltsSrc_> & var) {
                return basic_variant<AltsDst_>(var.template get_unsafe<StoredTy_>());
            }

            basic_variant<AltsDst_> create(basic_variant<AltsSrc_> && var) {
                return basic_variant<AltsDst_>(std::move(var.template get_unsafe<StoredTy_>()));
            }
        };

        template <typename AltsDst_, typename AltsSrc_, typename StoredTy_>
        struct variant_cast_create <
            AltsDst_, AltsSrc_, StoredTy_,
            std::enable_if_t<(meta::list_find<AltsDst_, StoredTy_>::value != meta::list_npos) && (std::is_same<void, StoredTy_>::value)>
        >
        {
            basic_variant<AltsDst_> create(const basic_variant<AltsSrc_> &) {
                return basic_variant<AltsDst_>(nullvar_t{});
            }

            basic_variant<AltsDst_> create(basic_variant<AltsSrc_> &&) {
                return basic_variant<AltsDst_>(nullvar_t{});
            }
        };



        template <typename AltsDst_, typename AltsSrc_, typename AltsTmp_, size_t IdxSrc_>
        struct variant_cast_impl
        {
            basic_variant<AltsDst_> cast(const basic_variant<AltsSrc_> & var) {
                return (var.type_index() == IdxSrc_)
                    ? variant_cast_create<AltsDst_, AltsSrc_, typename AltsTmp_::head>().create(var)
                    : variant_cast_impl<AltsDst_, AltsSrc_, typename AltsTmp_::tail, IdxSrc_ + 1>().cast(var);
            }

            basic_variant<AltsDst_> cast(basic_variant<AltsSrc_> && var) {
                return (var.type_index() == IdxSrc_)
                    ? variant_cast_create<AltsDst_, AltsSrc_, typename AltsTmp_::head>().create(std::move(var))
                    : variant_cast_impl<AltsDst_, AltsSrc_, typename AltsTmp_::tail, IdxSrc_ + 1>().cast(std::move(var));
            }
        };

        template <typename AltsDst_, typename AltsSrc_, size_t IdxSrc_>
        struct variant_cast_impl
            <AltsDst_, AltsSrc_, yato::meta::null_list, IdxSrc_>
        {
            template <typename Ty_>
            basic_variant<AltsDst_> cast(Ty_ &&) {
                throw bad_variant_cast{};
            }
        };

    }


    /**
     * Exception used to indicate variant access error
     */
    using details::bad_variant_access;

    /**
     * Exception used to indicate variant_cast error
     */
    using details::bad_variant_cast;

    /**
     * @see detials::basic_variant
     */
    template <typename... Alts_>
    using variant = details::basic_variant<yato::meta::list<Alts_...>>;

    /**
     * Construct empty variant, if void state is supported
     */
    using details::nullvar_t;

#if !YATO_MSVC || (YATO_MSVC >= YATO_MSVC_2013)
    YATO_INLINE_VARIABLE constexpr nullvar_t nullvar{};
#endif

    /**`
     * Cast variants with different alternatives lists
     */
    template <typename AltsListTo_, typename AltsListFrom_>
    yato::details::basic_variant<AltsListTo_> variant_cast(const yato::details::basic_variant<AltsListFrom_> & var) {
        return details::variant_cast_impl<AltsListTo_, AltsListFrom_, AltsListFrom_, 0>().cast(var);
    }

    /**
     * Cast variants with different alternatives lists
     */
    template <typename AltsListTo_, typename AltsListFrom_>
    yato::details::basic_variant<AltsListTo_> variant_cast(yato::details::basic_variant<AltsListFrom_> && var) {
        return details::variant_cast_impl<AltsListTo_, AltsListFrom_, AltsListFrom_, 0>().cast(std::move(var));
    }

}

#endif

