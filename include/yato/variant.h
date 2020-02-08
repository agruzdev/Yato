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


        template <size_t ListPos_>
        struct find_index_impl_
            : std::integral_constant<variant_index_t, static_cast<variant_index_t>(ListPos_)>
        { };

        template <>
        struct find_index_impl_<meta::list_npos>
            : std::integral_constant<variant_index_t, variant_no_index>
        { };

        template <typename TypesList_, typename Ty_>
        struct find_index
            : find_index_impl_<meta::list_find<TypesList_, Ty_>::value>
        { };


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
        struct max_types_size <yato::meta::null_list>
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
        struct max_types_alignment <yato::meta::null_list>
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
        struct all_copy_constructible <yato::meta::null_list>
            : public std::integral_constant<bool, true>
        { };


        template <typename TypesList_>
        struct all_nothrow_copy_constructible
            : public std::integral_constant<bool,
                (std::is_same<void, typename TypesList_::head>::value || std::is_nothrow_copy_constructible<typename TypesList_::head>::value) &&
                all_nothrow_copy_constructible<typename TypesList_::tail>::value
            >
        { };

        template <>
        struct all_nothrow_copy_constructible <yato::meta::null_list>
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
        struct all_move_constructible <yato::meta::null_list>
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
        struct all_nothrow_move_constructible <yato::meta::null_list>
            : public std::integral_constant<bool, true>
        { };




        /**
         *  Calls operator new for stored type
         */
        template <typename Ty_, typename = void>
        struct variant_creator
        {
            template <typename ... Args>
            auto placement_new(void* prt, Args && ... args) YATO_NOEXCEPT_OPERATOR((std::is_nothrow_constructible<Ty_, Args...>::value))
                -> std::enable_if_t<std::is_constructible<Ty_, Args...>::value>
            {
                new (yato::pointer_cast<Ty_*>(prt)) Ty_(std::forward<Args>(args)...);
            }
        };

        template <typename Ty_>
        struct variant_creator<Ty_, typename std::enable_if<
            std::is_same<Ty_, void>::value
        >::type>
        {
            template <typename ... Args>
            void placement_new(void*, Args && ...) YATO_NOEXCEPT_KEYWORD
            { }
        };


        /**
         *  Calls destructor for stored type
         */
        template <typename Ty_, typename = void>
        struct variant_destructor
        {
            void destroy(void* prt) YATO_NOEXCEPT_KEYWORD
            {
                yato::pointer_cast<Ty_*>(prt)->~Ty_();
            }
        };

        template <typename Ty_>
        struct variant_destructor<Ty_, typename std::enable_if<
            std::is_same<Ty_, void>::value
        >::type>
        {
            void destroy(void*) YATO_NOEXCEPT_KEYWORD
            { }
        };

        /**
         *  Dereferences pointer to raw pointer corresponding to actual type 
         */
        template <typename Ty_, typename = void>
        struct variant_dereference
        {
            Ty_ & operator()(Ty_* ptr) YATO_NOEXCEPT_KEYWORD
            {
                return *ptr;
            }

            const Ty_ & operator()(const Ty_* ptr) YATO_NOEXCEPT_KEYWORD
            {
                return *ptr;
            }
        };

        template <typename Ty_>
        struct variant_dereference<Ty_, typename std::enable_if<
            std::is_same<Ty_, void>::value
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

        //-----------------------------------------------------------------
        // Destroy

        /**
         * Calls correct destructor according the stored type
         */
        template <typename TypesList_, variant_index_t TypeIdx_ = 0, typename = void>
        struct variant_dispatcher_destroy
        {
            using stored_type = typename yato::meta::list_at<TypesList_, TypeIdx_>::type;

            void apply(variant_index_t idx, void* ptr) YATO_NOEXCEPT_KEYWORD
            {
                if (idx == TypeIdx_) {
                    variant_destructor<stored_type>().destroy(ptr);
                }
                else {
                    variant_dispatcher_destroy<TypesList_, TypeIdx_ + 1>().apply(idx, ptr);
                }
            }
        };

        template <typename TypesList_, variant_index_t TypeIdx_>
        struct variant_dispatcher_destroy < TypesList_, TypeIdx_, typename std::enable_if<
            (TypeIdx_ >= yato::meta::list_length<TypesList_>::value)
        >::type>
        {
            void apply(variant_index_t, void*) YATO_NOEXCEPT_KEYWORD
            { }
        };

        //-----------------------------------------------------------------
        // Copy

        /**
         *  Creates with copy constructor
         */
        template <typename TypesList_, variant_index_t TypeIdx_ = 0, typename = void>
        struct variant_dispatcher_copy
        {
            using stored_type = typename yato::meta::list_at<TypesList_, TypeIdx_>::type;

            void apply(variant_index_t idx, void* dst, const void* src)
            {
                if (idx == TypeIdx_) {
                    variant_creator<stored_type>().placement_new(dst, variant_dereference<stored_type>()(yato::pointer_cast<const stored_type*>(src)));
                }
                else {
                    variant_dispatcher_copy<TypesList_, TypeIdx_ + 1>().apply(idx, dst, src);
                }
            }
        };

        template <typename TypesList, variant_index_t TypeIdx>
        struct variant_dispatcher_copy < TypesList, TypeIdx, typename std::enable_if<
            (TypeIdx >= yato::meta::list_length<TypesList>::value)
        >::type>
        {
            void apply(variant_index_t, void*, const void*) YATO_NOEXCEPT_KEYWORD
            { }
        };

        //-----------------------------------------------------------------
        // Move

        /**
         *  Creates with move constructor
         */
        template <typename TypesList, variant_index_t TypeIdx = 0, typename = void>
        struct variant_dispatcher_move
        {
            using stored_type = typename yato::meta::list_at<TypesList, TypeIdx>::type;

            void apply(variant_index_t idx, void* dst, void* src)
            {
                if (idx == TypeIdx) {
                    variant_creator<stored_type>().placement_new(dst, std::move(variant_dereference<stored_type>()(yato::pointer_cast<stored_type*>(src))));
                }
                else {
                    variant_dispatcher_move<TypesList, TypeIdx + 1>().apply(idx, dst, src);
                }
            }
        };

        template <typename TypesList, variant_index_t TypeIdx>
        struct variant_dispatcher_move < TypesList, TypeIdx, typename std::enable_if<
            (TypeIdx >= yato::meta::list_length<TypesList>::value)
        >::type>
        {
            void apply(variant_index_t, void*, const void*) YATO_NOEXCEPT_KEYWORD
            { }
        };

        //-----------------------------------------------------------------
        // Copy assign

        template <typename TypesList, variant_index_t TypeIdx = 0, typename = void>
        struct variant_dispatcher_copy_assign
        {
            using stored_type = typename yato::meta::list_at<TypesList, TypeIdx>::type;

            template <typename Type = stored_type>
            auto apply(variant_index_t idx, void* dst, const void* src)
                -> typename std::enable_if<!std::is_same<Type, void>::value && std::is_copy_assignable<Type>::value, void>::type
            {
                if (idx == TypeIdx) {
                    *yato::pointer_cast<stored_type*>(dst) = *yato::pointer_cast<const stored_type*>(src);
                }
                else {
                    variant_dispatcher_copy_assign<TypesList, TypeIdx + 1>().apply(idx, dst, src);
                }
            }

            /**
             *  If assignment operator is not available fall back to copy
             */
            template <typename Type = stored_type>
            auto apply(variant_index_t idx, void* dst, const void* src)
                -> typename std::enable_if<std::is_same<Type, void>::value || !std::is_copy_assignable<Type>::value, void>::type
            {
                if (idx == TypeIdx) {
                    variant_dispatcher_destroy<TypesList, TypeIdx>().apply(idx, dst);
                    variant_dispatcher_copy<TypesList, TypeIdx>().apply(idx, dst, src);
                }
                else {
                    variant_dispatcher_copy_assign<TypesList, TypeIdx + 1>().apply(idx, dst, src);
                }
            }
        };

        template <typename TypesList, variant_index_t TypeIdx>
        struct variant_dispatcher_copy_assign < TypesList, TypeIdx, typename std::enable_if<
            (TypeIdx >= yato::meta::list_length<TypesList>::value)
        >::type>
        {
            void apply(variant_index_t, void*, const void*) YATO_NOEXCEPT_KEYWORD
            { }
        };

        //-----------------------------------------------------------------
        // Move assign

        template <typename TypesList, variant_index_t TypeIdx = 0, typename = void>
        struct variant_dispatcher_move_assign
        {
            using stored_type = typename yato::meta::list_at<TypesList, TypeIdx>::type;

            template <typename Type = stored_type>
            auto apply(variant_index_t idx, void* dst, void* src)
                -> typename std::enable_if<std::is_same<Type, void>::value || !std::is_move_assignable<Type>::value, void>::type
            {
                if (idx == TypeIdx) {
                    variant_dispatcher_destroy<TypesList, TypeIdx>().apply(idx, dst);
                    variant_dispatcher_move<TypesList, TypeIdx>().apply(idx, dst, src);
                }
                else {
                    variant_dispatcher_move_assign<TypesList, TypeIdx + 1>().apply(idx, dst, src);
                }
            }

            template <typename Type = stored_type>
            auto apply(variant_index_t idx, void* dst, void* src)
                -> typename std::enable_if<!std::is_same<Type, void>::value && std::is_move_assignable<Type>::value, void>::type
            {
                if (idx == TypeIdx) {
                    *yato::pointer_cast<stored_type*>(dst) = std::move(*yato::pointer_cast<stored_type*>(src));
                }
                else {
                    variant_dispatcher_move_assign<TypesList, TypeIdx + 1>().apply(idx, dst, src);
                }
            }

        };

        template <typename TypesList, variant_index_t TypeIdx>
        struct variant_dispatcher_move_assign < TypesList, TypeIdx, typename std::enable_if<
            (TypeIdx >= yato::meta::list_length<TypesList>::value)
        >::type>
        {
            void apply(variant_index_t, void*, const void*) YATO_NOEXCEPT_KEYWORD
            { }
        };

        //-----------------------------------------------------------------------------------------
        // Get type

        /**
         *  Gets typeid of the currently stored type
         *  Returns typeid(void) if no type is stored
         */
        template <typename TypesList, variant_index_t TypeIdx = 0, typename = void>
        struct variant_dispatcher_get_type
        {
            using stored_type = typename yato::meta::list_at<TypesList, TypeIdx>::type;

            const std::type_info & apply(variant_index_t idx) YATO_NOEXCEPT_KEYWORD
            {
                if (idx == TypeIdx) {
                    return typeid(stored_type);
                }
                else {
                    return variant_dispatcher_get_type<TypesList, TypeIdx + 1>().apply(idx);
                }
            }
        };

        template <typename TypesList, variant_index_t TypeIdx>
        struct variant_dispatcher_get_type < TypesList, TypeIdx, typename std::enable_if<
            (TypeIdx >= yato::meta::list_length<TypesList>::value)
        >::type>
        {
            const std::type_info & apply(variant_index_t) YATO_NOEXCEPT_KEYWORD
            {
                return typeid(void);
            }
        };

        //-----------------------------------------------------------------------------------------

        struct construct_empty_t {};

        class bad_variant_access
            : public std::runtime_error
        {
        public:
            bad_variant_access(const char* what)
                : std::runtime_error(what)
            { }
        };


        template <typename AltsList_, bool IsCopy_ = false, bool IsMove_ = false>
        class variant_storage
        {
        private:
            using alternativies_list = AltsList_;
            using storage_type = typename std::aligned_storage<
                details::max_types_size<alternativies_list>::value,
                details::max_types_alignment<alternativies_list>::value >::type;

            storage_type m_storage{};
            variant_index_t m_type_idx = variant_no_index;

        public:
            variant_storage() = default;

            variant_storage(variant_index_t idx) // for void idx
                : m_type_idx(idx)
            { }

            ~variant_storage() {
                if (m_type_idx != variant_no_index) {
                    destroy();
                }
            }

            variant_storage(const variant_storage&) = delete;
            variant_storage& operator = (const variant_storage&) = delete;

            variant_storage(variant_storage&&) = delete;
            variant_storage& operator = (variant_storage&&) = delete;

            template <typename Ty, typename ... Args> 
            void create(Args && ... args)
            {
                details::variant_creator<Ty>().placement_new(&m_storage, std::forward<Args>(args)...);
                m_type_idx = details::find_index<alternativies_list, Ty>::value;
            }

            void destroy() YATO_NOEXCEPT_KEYWORD
            {
                if (m_type_idx != variant_no_index) {
                    details::variant_dispatcher_destroy<alternativies_list>().apply(m_type_idx, yato::pointer_cast<void*>(&m_storage));
                    m_type_idx = variant_no_index;
                }
            }

            variant_index_t type_index() const {
                return m_type_idx;
            }

            const void* data() const {
                return static_cast<const void*>(&m_storage);
            }

            void* data() {
                return static_cast<void*>(&m_storage);
            }
        };

        template <typename AltsList_>
        class variant_storage <
            AltsList_,
            /*IsCopy_=*/true,
            /*IsMove_=*/false
        >
        {
        protected:
            using alternativies_list = AltsList_;
            using storage_type = typename std::aligned_storage<
                details::max_types_size<alternativies_list>::value,
                details::max_types_alignment<alternativies_list>::value >::type;

            storage_type m_storage{};
            variant_index_t m_type_idx = variant_no_index;

        public:
            variant_storage() = default;

            variant_storage(variant_index_t idx) // for void idx
                : m_type_idx(idx)
            { }

            ~variant_storage()
            {
                if (m_type_idx != variant_no_index) {
                    destroy();
                }
            }

            variant_storage(const variant_storage & other)
                : m_type_idx(other.m_type_idx)
            {
                if (other.m_type_idx != variant_no_index) {
                    details::variant_dispatcher_copy<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                }
            };

            variant_storage& operator = (const variant_storage & other)
            {
                if (this != &other) {
                    if (m_type_idx != other.m_type_idx) {
                        destroy();
                        details::variant_dispatcher_copy<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                        m_type_idx = other.m_type_idx;
                    } else {
                        details::variant_dispatcher_copy_assign<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                    }
                }
                return *this;
            }

            // Copy
            variant_storage(variant_storage && other) YATO_NOEXCEPT_OPERATOR(all_nothrow_copy_constructible<alternativies_list>::value)
                : m_type_idx(other.m_type_idx)
            {
                if (other.m_type_idx != variant_no_index) {
                    details::variant_dispatcher_copy<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                }
            };

            // Copy
            variant_storage& operator = (variant_storage && other) YATO_NOEXCEPT_OPERATOR(all_nothrow_copy_constructible<alternativies_list>::value)
            {
                return this->operator=(other);
            }

            template <typename Ty, typename ... Args> 
            void create(Args && ... args)
            {
                details::variant_creator<Ty>().placement_new(&m_storage, std::forward<Args>(args)...);
                m_type_idx = details::find_index<alternativies_list, Ty>::value;
            }

            void destroy() YATO_NOEXCEPT_KEYWORD
            {
                if (m_type_idx != variant_no_index) {
                    details::variant_dispatcher_destroy<alternativies_list>().apply(m_type_idx, yato::pointer_cast<void*>(&m_storage));
                    m_type_idx = variant_no_index;
                }
            }

            variant_index_t type_index() const {
                return m_type_idx;
            }

            const void* data() const {
                return static_cast<const void*>(&m_storage);
            }

            void* data() {
                return static_cast<void*>(&m_storage);
            }
        };



        template <typename AltsList_>
        class variant_storage <
            AltsList_,
            /*IsCopy_=*/true,
            /*IsMove_=*/true
        >
        {
        protected:
            using alternativies_list = AltsList_;
            using storage_type = typename std::aligned_storage<
                details::max_types_size<alternativies_list>::value,
                details::max_types_alignment<alternativies_list>::value >::type;

            storage_type m_storage{};
            variant_index_t m_type_idx = variant_no_index;

        public:
            variant_storage() = default;

            variant_storage(variant_index_t idx) // for void idx
                : m_type_idx(idx)
            { }

            ~variant_storage()
            {
                if (m_type_idx != variant_no_index) {
                    destroy();
                }
            }

            variant_storage(const variant_storage & other)
                : m_type_idx(other.m_type_idx)
            {
                if (other.m_type_idx != variant_no_index) {
                    details::variant_dispatcher_copy<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                }
            };

            variant_storage& operator = (const variant_storage & other)
            {
                if (this != &other) {
                    if (m_type_idx != other.m_type_idx) {
                        destroy();
                        details::variant_dispatcher_copy<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                        m_type_idx = other.m_type_idx;
                    } else {
                        details::variant_dispatcher_copy_assign<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                    }
                }
                return *this;
            }

            variant_storage(variant_storage && other) YATO_NOEXCEPT_OPERATOR(all_nothrow_move_constructible<alternativies_list>::value)
                : m_type_idx(other.m_type_idx)
            {
                if (other.m_type_idx != variant_no_index) {
                    details::variant_dispatcher_move<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                }
            };

            variant_storage& operator = (variant_storage && other) YATO_NOEXCEPT_OPERATOR(all_nothrow_move_constructible<alternativies_list>::value)
            {
                if (this != &other) {
                    if (m_type_idx != other.m_type_idx) {
                        destroy();
                        details::variant_dispatcher_move<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                        m_type_idx = other.m_type_idx;
                    } else {
                        details::variant_dispatcher_move_assign<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                    }
                }
                return *this;
            }

            template <typename Ty, typename ... Args> 
            void create(Args && ... args)
            {
                details::variant_creator<Ty>().placement_new(&m_storage, std::forward<Args>(args)...);
                m_type_idx = details::find_index<alternativies_list, Ty>::value;
            }

            void destroy() YATO_NOEXCEPT_KEYWORD
            {
                if(m_type_idx != variant_no_index) {
                    details::variant_dispatcher_destroy<alternativies_list>().apply(m_type_idx, yato::pointer_cast<void*>(&m_storage));
                    m_type_idx = variant_no_index;
                }
            }

            variant_index_t type_index() const {
                return m_type_idx;
            }

            const void* data() const {
                return static_cast<const void*>(&m_storage);
            }

            void* data() {
                return static_cast<void*>(&m_storage);
            }
        };


        template <typename AltsList_>
        class variant_storage <
            AltsList_,
            /*IsCopy_=*/false,
            /*IsMove_=*/true
        >
        {
        protected:
            using alternativies_list = AltsList_;
            using storage_type = typename std::aligned_storage<
                details::max_types_size<alternativies_list>::value,
                details::max_types_alignment<alternativies_list>::value >::type;

            storage_type m_storage{};
            variant_index_t m_type_idx = variant_no_index;

        public:
            variant_storage() = default;

            variant_storage(variant_index_t idx) // for void idx
                : m_type_idx(idx)
            { }

            ~variant_storage()
            {
                if (m_type_idx != variant_no_index) {
                    destroy();
                }
            }

            variant_storage(const variant_storage & other) = delete;

            variant_storage& operator = (const variant_storage & other) = delete;

            variant_storage(variant_storage && other) YATO_NOEXCEPT_OPERATOR(all_nothrow_move_constructible<alternativies_list>::value)
                : m_type_idx(other.m_type_idx)
            {
                if (other.m_type_idx != variant_no_index) {
                    details::variant_dispatcher_move<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                }
            };

            variant_storage& operator = (variant_storage && other) YATO_NOEXCEPT_OPERATOR(all_nothrow_move_constructible<alternativies_list>::value)
            {
                if (this != &other) {
                    if (m_type_idx != other.m_type_idx) {
                        destroy();
                        details::variant_dispatcher_move<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                        m_type_idx = other.m_type_idx;
                    } else {
                        details::variant_dispatcher_move_assign<alternativies_list>().apply(other.m_type_idx, &m_storage, &other.m_storage);
                    }
                }
                return *this;
            }

            template <typename Ty, typename ... Args> 
            void create(Args && ... args)
            {
                details::variant_creator<Ty>().placement_new(&m_storage, std::forward<Args>(args)...);
                m_type_idx = details::find_index<alternativies_list, Ty>::value;
            }

            void destroy() YATO_NOEXCEPT_KEYWORD
            {
                if (m_type_idx != variant_no_index) {
                    details::variant_dispatcher_destroy<alternativies_list>().apply(m_type_idx, yato::pointer_cast<void*>(&m_storage));
                    m_type_idx = variant_no_index;
                }
            }

            variant_index_t type_index() const {
                return m_type_idx;
            }

            const void* data() const {
                return static_cast<const void*>(&m_storage);
            }

            void* data() {
                return static_cast<void*>(&m_storage);
            }
        };

        template <typename AltsList_>
        using choose_variant_storage = variant_storage<
            AltsList_, 
            all_copy_constructible<AltsList_>::value, 
            all_move_constructible<AltsList_>::value
        >;



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
            using storage_type = choose_variant_storage<alternativies_list>;
            static YATO_CONSTEXPR_VAR size_t alternativies_number = yato::meta::list_length<alternativies_list>::value;

            static_assert(alternativies_number > 0, "yato::variant: alternatives must be not empty.");
            static_assert(alternativies_number <= details::variant_max_index, "yato::variant: too many alternatives.");
            static_assert(std::is_same<typename meta::list_unique<alternativies_list>::type, alternativies_list>::value, "yato::variant: alternatives must have unique types");
            //--------------------------------------------------------------------
        private:
            storage_type m_storage{};
            //--------------------------------------------------------------------

            template <typename Ty_>
            const Ty_* caddress_() const
            {
                return yato::launder(yato::pointer_cast<const Ty_*>(m_storage.data()));
            }

            template <typename Ty_>
            Ty_* address_()
            {
                return yato::launder(yato::pointer_cast<Ty_*>(m_storage.data()));
            }
            //--------------------------------------------------------------------

        public:
            /**
             *  Creates empty variant
             *  As is void is stored
             */
            template <typename Types_ = alternativies_list, typename = 
                std::enable_if_t<meta::list_find<Types_, void>::value != yato::meta::list_npos>
            >
            YATO_CONSTEXPR_FUNC
            basic_variant(details::construct_empty_t = details::construct_empty_t{}) YATO_NOEXCEPT_KEYWORD
                : m_storage(meta::list_find<Types_, void>::value)
            { }

            /**
             *  Create variant from value
             */
            template <typename Ty, typename = typename std::enable_if<
                (yato::meta::list_find<alternativies_list, yato::remove_cvref_t<Ty>>::value != yato::meta::list_npos)
            >::type>
            explicit
            basic_variant(Ty && value)
            {
                m_storage.template create<yato::remove_cvref_t<Ty>>(std::forward<Ty>(value));
            }

            /**
             *  Create variant in place
             */
            template <typename Ty, typename = typename std::enable_if<(yato::meta::list_find<alternativies_list, Ty>::value != yato::meta::list_npos)>::type, typename ... Args>
            explicit
            basic_variant(yato::in_place_type_t<Ty>, Args && ... args)
            {
                m_storage.template create<Ty>(std::forward<Args>(args)...);
            }

            /**
             *  Create variant in place
             */
            template <size_t Idx, typename = typename std::enable_if<(Idx < yato::meta::list_length<alternativies_list>::value)>::type, typename ... Args>
            explicit
            basic_variant(yato::in_place_index_t<Idx>, Args && ... args)
            {
                m_storage.template create<meta::list_at_t<alternativies_list, Idx>>(std::forward<Args>(args)...);
            }

            ~basic_variant() = default;

            basic_variant(const basic_variant & other) = default;

            basic_variant(basic_variant && other) = default;

            /**
             *  If stored types of both variants are same, then calls copy assignment operator
             *  If the types are different or copy assignment is not available, then destroys current instance and creates copy
             */
            basic_variant& operator = (const basic_variant & other) = default;

            /**
             *  If stored types of both variants are same, then calls move assignment operator
             *  If the types are different or copy assignment is not available, then destroys current instance and creates moved copy
             */
            basic_variant& operator = (basic_variant && other) = default;

            /**
             *  Replaces content of the variant with a new type 
             *  Calling with Ty = void will clear the variant
             */
            template <typename Ty, typename ... Args>
            void emplace(Args && ... args)
            {
                m_storage.destroy();
                m_storage.template create<Ty>(std::forward<Args>(args)...);
            }

            /**
             *  Get typeid of the currently stored type
             *  Returns typeid(void) if variant is empty
             */
            const std::type_info & type() const noexcept
            {
                return details::variant_dispatcher_get_type<alternativies_list>().apply(m_storage.type_index());
            }

            /**
             * Get stored type index
             */
            size_t type_index() const noexcept
            {
                return m_storage.type_index();
            }

            /**
             * Check stored type
             */
            template <typename Ty_>
            YATO_CONSTEXPR_FUNC
            bool is_type() const noexcept
            {
                return (meta::list_find<alternativies_list, Ty_>::value == m_storage.type_index());
            }

            /**
             *  Get value by type index
             *  On error throws bad_variant_access
             */
            template <size_t Idx>
            auto get()
                -> meta::list_at_t<alternativies_list, Idx>&
            {
                if (m_storage.type_index() == Idx) {
                    return *address_<meta::list_at_t<alternativies_list, Idx>>();
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
                if (m_storage.type_index() == Idx) {
                    return *caddress_<meta::list_at_t<alternativies_list, Idx>>();
                }
                else {
                    throw bad_variant_access("yato::variant_bad_access: Stored type differs from the type by given index");
                }
            }

            /**
             *  Get value by type index
             *  On error returns default value
             */
            template <size_t Idx>
            decltype(auto) get(meta::list_at_t<alternativies_list, Idx> & default_value) noexcept
            {
                if (m_storage.type_index() == Idx) {
                    return *address_<yato::meta::list_at_t<alternativies_list, Idx>>();
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
                if (m_storage.type_index() == Idx) {
                    return *caddress_<meta::list_at_t<alternativies_list, Idx>>();
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
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty>
            yato::optional<Ty> get_opt() &&
            {
                if (is_type<Ty>()) {
                    return yato::make_optional(std::move(*address_<Ty>()));
                } else {
                    return yato::nullopt_t{};
                }
            }

            /**
             *  Get value by type
             *  On error throws bad_variant_access
             */
            template <typename Ty>
            yato::optional<Ty> get_opt() const &
            {
                if(is_type<Ty>()) {
                    //return yato::make_optional(*yato::pointer_cast<const Ty*>(m_storage.data()));
                    return yato::make_optional(*caddress_<Ty>());
                } else {
                    return yato::nullopt_t{};
                }
            }


            /**
             *  Get value without check
             *  Use only if you are sure what is stored type
             */
            template <typename Ty>
            Ty & get_as_unsafe()
            {
                return *yato::pointer_cast<Ty*>(m_storage.data());
            }

            /**
             *  Get value without check
             *  Use only if you are sure what is stored type
             */
            template <typename Ty>
            const Ty & get_as_unsafe() const
            {
                return *yato::pointer_cast<const Ty*>(m_storage.data());
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
    using nullvar_t = details::construct_empty_t;

#ifndef YATO_MSVC_2013
    YATO_INLINE_VARIABLE constexpr nullvar_t nullvar{};
#endif

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

