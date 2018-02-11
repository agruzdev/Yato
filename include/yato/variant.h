/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_VARIANT_H_
#define _YATO_VARIANT_H_

#include "meta.h"
#include "types.h"

namespace yato
{

    namespace details
    {
        template <typename Ty>
        struct sizeof_ext
            : public std::integral_constant<size_t, sizeof(Ty)>
        { };

        template <>
        struct sizeof_ext<void>
            : public std::integral_constant<size_t, 1>
        { };

        template <typename Ty>
        struct alignof_ext
            : public std::integral_constant<size_t, std::alignment_of<Ty>::value>
        { };

        template <>
        struct alignof_ext<void>
            : public std::integral_constant<size_t, 1>
        { };

        template <size_t X, size_t Y>
        struct max_helper
            : public std::integral_constant<size_t, (X > Y) ? X : Y>
        { };


        template <typename TypesList>
        struct max_types_size
            : public std::integral_constant<size_t, max_helper<
                sizeof_ext<typename TypesList::head>::value,
                max_types_size<typename TypesList::tail>::value
            >::value>
        { };

        template <>
        struct max_types_size <yato::meta::null_list>
            : public std::integral_constant<size_t, 1>
        { };


        template <typename TypesList>
        struct max_types_alignment
            : public std::integral_constant<size_t, max_helper<
                alignof_ext<typename TypesList::head>::value,
                max_types_alignment<typename TypesList::tail>::value
            >::value>
        { };

        template <>
        struct max_types_alignment <yato::meta::null_list>
            : public std::integral_constant<size_t, 1>
        { };

        /**
         *  Calls operator new for stored type
         */
        template <typename Ty, typename = void>
        struct variant_creator
        {
            template <typename ... Args>
            void placement_new(void* prt, Args && ... args) YATO_NOEXCEPT_OPERATOR((std::is_nothrow_constructible<Ty, Args...>::value))
            {
                new (yato::pointer_cast<Ty*>(prt)) Ty(std::forward<Args>(args)...);
            }
        };

        template <typename Ty>
        struct variant_creator<Ty, typename std::enable_if<
            std::is_same<Ty, void>::value
        >::type>
        {
            template <typename ... Args>
            void placement_new(void*, Args && ...) YATO_NOEXCEPT_KEYWORD
            { }
        };


        /**
         *  Calls destructor for stored type
         */
        template <typename Ty, typename = void>
        struct variant_destructor
        {
            void destroy(void* prt) YATO_NOEXCEPT_KEYWORD
            {
                yato::pointer_cast<Ty*>(prt)->~Ty();
            }
        };

        template <typename Ty>
        struct variant_destructor<Ty, typename std::enable_if<
            std::is_same<Ty, void>::value
        >::type>
        {
            void destroy(void*) YATO_NOEXCEPT_KEYWORD
            { }
        };

        /**
         *  Dereferences pointer to raw pointer corresponding to actual type 
         */
        template <typename Ty, typename = void>
        struct variant_dereference
        {
            Ty & operator()(Ty* ptr) YATO_NOEXCEPT_KEYWORD
            {
                return *ptr;
            }

            const Ty & operator()(const Ty* ptr) YATO_NOEXCEPT_KEYWORD
            {
                return *ptr;
            }
        };

        template <typename Ty>
        struct variant_dereference<Ty, typename std::enable_if<
            std::is_same<Ty, void>::value
        >::type>
        {
            void* operator()(void* ptr) YATO_NOEXCEPT_KEYWORD
            {
                return ptr;
            }

            const void* operator()(const void* ptr) YATO_NOEXCEPT_KEYWORD
            {
                return ptr;
            }
        };

        /**
         *  Performs actions for stored type
         */
        template <typename TypesList, size_t TypeIdx = 0, typename = void>
        struct variant_dispatcher
        {
            using stored_type = typename yato::meta::list_at<TypesList, TypeIdx>::type;

            /**
             *  Calls correct destructor according the stored type
             */
            void destroy(size_t idx, void* ptr) YATO_NOEXCEPT_KEYWORD
            {
                if (idx == TypeIdx) {
                    variant_destructor<stored_type>().destroy(ptr);
                }
                else {
                    variant_dispatcher<TypesList, TypeIdx + 1>().destroy(idx, ptr);
                }
            }

            /**
             *  Creates by copy constructor
             */
            void create_copy(size_t idx, void* dst, const void* src)
            {
                if (idx == TypeIdx) {
                    variant_creator<stored_type>().placement_new(dst, variant_dereference<stored_type>()(yato::pointer_cast<const stored_type*>(src)));
                }
                else {
                    variant_dispatcher<TypesList, TypeIdx + 1>().create_copy(idx, dst, src);
                }
            }

            /**
             *  Creates by copy constructor
             */
            void move_copy(size_t idx, void* dst, void* src)
            {
                if (idx == TypeIdx) {
                    variant_creator<stored_type>().placement_new(dst, std::move(variant_dereference<stored_type>()(yato::pointer_cast<stored_type*>(src))));
                }
                else {
                    variant_dispatcher<TypesList, TypeIdx + 1>().move_copy(idx, dst, src);
                }
            }

            template <typename Type = stored_type>
            auto copy_assign(size_t idx, void* dst, const void* src)
                -> typename std::enable_if<!std::is_same<Type, void>::value && std::is_copy_assignable<Type>::value, void>::type
            {
                if (idx == TypeIdx) {
                    *yato::pointer_cast<stored_type*>(dst) = *yato::pointer_cast<const stored_type*>(src);
                }
                else {
                    variant_dispatcher<TypesList, TypeIdx + 1>().copy_assign(idx, dst, src);
                }
            }

            /**
            *  If assignment operator is not available fall back to copy
            */
            template <typename Type = stored_type>
            auto copy_assign(size_t idx, void* dst, const void* src)
                -> typename std::enable_if<std::is_same<Type, void>::value || !std::is_copy_assignable<Type>::value, void>::type
            {
                if (idx == TypeIdx) {
                    destroy(idx, dst);
                    create_copy(idx, dst, src);
                }
                else {
                    variant_dispatcher<TypesList, TypeIdx + 1>().copy_assign(idx, dst, src);
                }
            }

            template <typename Type = stored_type>
            auto move_assign(size_t idx, void* dst, void* src)
                -> typename std::enable_if<std::is_same<Type, void>::value || !std::is_move_assignable<Type>::value, void>::type
            {
                if (idx == TypeIdx) {
                    destroy(idx, dst);
                    move_copy(idx, dst, src);
                }
                else {
                    variant_dispatcher<TypesList, TypeIdx + 1>().move_assign(idx, dst, src);
                }
            }

            template <typename Type = stored_type>
            auto move_assign(size_t idx, void* dst, void* src)
                -> typename std::enable_if<!std::is_same<Type, void>::value && std::is_move_assignable<Type>::value, void>::type
            {
                if (idx == TypeIdx) {
                    *yato::pointer_cast<stored_type*>(dst) = std::move(*yato::pointer_cast<stored_type*>(src));
                }
                else {
                    variant_dispatcher<TypesList, TypeIdx + 1>().move_assign(idx, dst, src);
                }
            }


            /**
             *  Gets typeid of the currently stored type
             *  Returns typeid(void) if no type is stored
             */
            const std::type_info & get_type(size_t idx) YATO_NOEXCEPT_KEYWORD
            {
                if (idx == TypeIdx) {
                    return typeid(stored_type);
                }
                else {
                    return variant_dispatcher<TypesList, TypeIdx + 1>().get_type(idx);
                }
            }
        };

        template <typename TypesList, size_t TypeIdx>
        struct variant_dispatcher < TypesList, TypeIdx, typename std::enable_if<
            (TypeIdx >= yato::meta::list_length<TypesList>::value)
        >::type>
        {
            void destroy(size_t, void*) YATO_NOEXCEPT_KEYWORD
            { }

            void create_copy(size_t, void*, const void*) YATO_NOEXCEPT_KEYWORD
            { }

            void move_copy(size_t, void*, const void*) YATO_NOEXCEPT_KEYWORD
            { }

            void copy_assign(size_t, void*, const void*) YATO_NOEXCEPT_KEYWORD
            { }

            void move_assign(size_t, void*, const void*) YATO_NOEXCEPT_KEYWORD
            { }

            const std::type_info & get_type(size_t) YATO_NOEXCEPT_KEYWORD
            {
                return typeid(void);
            }
        };


        struct construct_empty_t {};

        class bad_variant_access
            : public std::runtime_error
        {
        public:
            bad_variant_access(const char* what)
                : std::runtime_error(what)
            { }
        };

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
            using default_type = typename alternativies_list::head;
            using storage_type = typename std::aligned_storage<
                details::max_types_size<alternativies_list>::value,
                details::max_types_alignment<alternativies_list>::value >::type;
            static YATO_CONSTEXPR_VAR size_t alternativies_number = yato::meta::list_length<alternativies_list>::value;

            static_assert(alternativies_number > 0, "yato::variant: alternatives must be not empty");
            static_assert(std::is_same<typename meta::list_unique<alternativies_list>::type, alternativies_list>::value, "yato::variant: alternatives must have unique types");
            //--------------------------------------------------------------------
        private:
            storage_type m_storage;
            size_t m_type_idx;
            //--------------------------------------------------------------------

            template <typename Ty, typename ... Args> 
            void create_(Args && ... args)
            {
                details::variant_creator<Ty>().placement_new(&m_storage, std::forward<Args>(args)...);
                m_type_idx = yato::meta::list_find<alternativies_list, Ty>::value;
            }


            void destroy_() YATO_NOEXCEPT_KEYWORD
            {
                //if (m_type_idx != yato::meta::list_npos) {
                    details::variant_dispatcher<alternativies_list>().destroy(m_type_idx, yato::pointer_cast<void*>(&m_storage));
                //    m_type_idx = yato::meta::list_npos;
                //}
            }

            //--------------------------------------------------------------------

        public:
            /**
             *  Creates empty variant
             *  As is void is stored
             */
            template <typename Types_ = alternativies_list>
            explicit
            basic_variant(typename std::enable_if<
                    meta::list_find<Types_, void>::value != yato::meta::list_npos,
                details::construct_empty_t>::type = details::construct_empty_t()) YATO_NOEXCEPT_KEYWORD
                : m_type_idx(meta::list_find<Types_, void>::value)
            { }

            /**
             *  Create variant from value
             */
            template <typename Ty, typename = typename std::enable_if<
                (yato::meta::list_find<alternativies_list, std::decay_t<Ty>>::value != yato::meta::list_npos)
            >::type>
            explicit
            basic_variant(Ty && value)
            {
                create_<std::decay_t<Ty>>(std::forward<Ty>(value));
            }

            /**
             *  Create variant in place
             */
            template <typename Ty, typename = typename std::enable_if<(yato::meta::list_find<alternativies_list, Ty>::value != yato::meta::list_npos)>::type, typename ... Args>
            explicit
            basic_variant(yato::in_place_type_t<Ty>, Args && ... args)
            {
                create_<Ty>(std::forward<Args>(args)...);
            }

            /**
             *  Create variant in place
             */
            template <size_t Idx, typename = typename std::enable_if<(Idx < yato::meta::list_length<alternativies_list>::value)>::type, typename ... Args>
            explicit
            basic_variant(yato::in_place_index_t<Idx>, Args && ... args)
            {
                create_<meta::list_at_t<alternativies_list, Idx>>(std::forward<Args>(args)...);
            }

            ~basic_variant()
            {
                destroy_();
            }

            basic_variant(const this_type & other)
                : m_type_idx(other.m_type_idx)
            {
                details::variant_dispatcher<alternativies_list>().create_copy(other.m_type_idx, &m_storage, &other.m_storage);
            }

            basic_variant(this_type && other)
                : m_type_idx(other.m_type_idx)
            {
                details::variant_dispatcher<alternativies_list>().move_copy(other.m_type_idx, &m_storage, &other.m_storage);
            }

            /**
             *  If stored types of both variants are same, then calls copy assignment operator
             *  If the types are different or copy assignment is not available, then destroys current instance and creates copy
             */
            basic_variant& operator = (const this_type & other)
            {
                if (m_type_idx != other.m_type_idx) {
                    destroy_();
                    details::variant_dispatcher<alternativies_list>().create_copy(other.m_type_idx, &m_storage, &other.m_storage);
                    m_type_idx = other.m_type_idx;
                }
                else {
                    if (this != &other) {
                        details::variant_dispatcher<alternativies_list>().copy_assign(other.m_type_idx, &m_storage, &other.m_storage);
                    }
                }
                return *this;
            }

            /**
             *  If stored types of both variants are same, then calls copy assignment operator
             *  If the types are different or copy assignment is not available, then destroys current instance and creates copy
             */
            basic_variant& operator = (this_type && other)
            {
                if (m_type_idx != other.m_type_idx) {
                    destroy_();
                    details::variant_dispatcher<alternativies_list>().move_copy(other.m_type_idx, &m_storage, &other.m_storage);
                    m_type_idx = other.m_type_idx;
                }
                else {
                    if (this != &other) {
                        details::variant_dispatcher<alternativies_list>().move_assign(other.m_type_idx, &m_storage, &other.m_storage);
                    }
                }
                return *this;
            }

            /**
             *  Replaces content of the variant with a new type 
             *  Calling with Ty = void will clear the variant
             */
            template <typename Ty, typename ... Args>
            void emplace(Args && ... args)
            {
                destroy_();
                create_<Ty>(std::forward<Args>(args)...);
            }

            /**
             *  Get typeid of the currently stored type
             *  Returns typeid(void) if variant is empty
             */
            const std::type_info & type() const noexcept
            {
                return details::variant_dispatcher<alternativies_list>().get_type(m_type_idx);
            }

            /**
             * Get stored type index
             */
            const size_t & type_index() const noexcept
            {
                return m_type_idx;
            }

            /**
             *  Get value by type index
             *  On error throws bad_variant_access
             */
            template <size_t Idx>
            auto get()
                -> meta::list_at_t<alternativies_list, Idx>&
            {
                if (m_type_idx == Idx) {
                    return *yato::pointer_cast<meta::list_at_t<alternativies_list, Idx>*>(&m_storage);
                }
                else {
                    throw bad_variant_access("yato::variant_bad_access: Stored type differs from the type by given index");
                }
            }

            /**
             *  Get value by type index
             *  On error throws bad_variant_access 
             */
            template <size_t Idx>
            auto get() const
                -> const meta::list_at_t<alternativies_list, Idx>&
            {
                return const_cast<this_type*>(this)->get<Idx>();
            }

            /**
             *  Get value by type index
             *  On error returns default value
             */
            template <size_t Idx>
            decltype(auto) get(meta::list_at_t<alternativies_list, Idx> & default_value) noexcept
            {
                if (m_type_idx == Idx) {
                    return *yato::pointer_cast<typename yato::meta::list_at<alternativies_list, Idx>::type*>(&m_storage);
                }
                else {
                    return default_value;
                }
            }

            /**
             *  Get value by type index
             *  On error returns default value
             */
            template <size_t Idx>
            decltype(auto) get(const meta::list_at_t<alternativies_list, Idx> & default_value) const noexcept
            {
                if (m_type_idx == Idx) {
                    return *yato::pointer_cast<const meta::list_at_t<alternativies_list, Idx>*>(&m_storage);
                }
                else {
                    return default_value;
                }
            }

            /**
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty>
            Ty & get_as()
            {
                return get<meta::list_find<alternativies_list, Ty>::value>();
            }

            /**
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty>
            const Ty & get_as() const
            {
                return get<meta::list_find<alternativies_list, Ty>::value>();
            }

            /**
             *  Get value by type
             *  On error returns default error
             */
            template <typename Ty>
            Ty & get_as(Ty & default_value) YATO_NOEXCEPT_KEYWORD
            {
                return get<meta::list_find<alternativies_list, Ty>::value>(default_value);
            }

            /**
             *  Get value by type
             *  On error returns default error
             */
            template <typename Ty>
            const Ty & get_as(const Ty & default_value) const YATO_NOEXCEPT_KEYWORD
            {
                return get<meta::list_find<alternativies_list, Ty>::value>(default_value);
            }


            /**
             *  Get value without check
             *  Use only if you are sure what is stored type
             */
            template <typename Ty>
            Ty & get_as_unsafe()
            {
                return *yato::pointer_cast<Ty*>(&m_storage);
            }

            /**
             *  Get value without check
             *  Use only if you are sure what is stored type
             */
            template <typename Ty>
            const Ty & get_as_unsafe() const
            {
                return *yato::pointer_cast<const Ty*>(&m_storage);
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
                return basic_variant<AltsDst_>(var.template get_as_unsafe<StoredTy_>());
            }

            basic_variant<AltsDst_> create(basic_variant<AltsSrc_> && var) {
                return basic_variant<AltsDst_>(std::move(var.template get_as_unsafe<StoredTy_>()));
            }
        };

        template <typename AltsDst_, typename AltsSrc_, typename StoredTy_>
        struct variant_cast_create <
            AltsDst_, AltsSrc_, StoredTy_,
            std::enable_if_t<(meta::list_find<AltsDst_, StoredTy_>::value != meta::list_npos) && (std::is_same<void, StoredTy_>::value)>
        >
        {
            basic_variant<AltsDst_> create(const basic_variant<AltsSrc_> &) {
                return basic_variant<AltsDst_>(construct_empty_t{});
            }

            basic_variant<AltsDst_> create(basic_variant<AltsSrc_> &&) {
                return basic_variant<AltsDst_>(construct_empty_t{});
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
     *  Exception used to indicate variant access error
     */
    using details::bad_variant_access;

    /**
     *  Exception used to indicate variant_cast error
     */
    using details::bad_variant_cast;

    /**
     * @see detials::basic_variant
     */
    template <typename... Alts_>
    using variant = details::basic_variant<yato::meta::list<Alts_...>>;


    /**
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

