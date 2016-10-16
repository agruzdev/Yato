/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016 Alexey Gruzdev
 */

#ifndef _YATO_ALIGNING_ALLOCATOR_H_
#define _YATO_ALIGNING_ALLOCATOR_H_

#include <cstddef>
#include "types.h"

namespace yato
{

    namespace details
    {
        template <size_t Align, typename OffsetType>
        struct get_extra_length
            : public std::integral_constant<size_t, Align - 1 + sizeof(OffsetType) + std::alignment_of<OffsetType>::value - 1>
        { };

        template <size_t Align, size_t TypeSize = 8, typename = void>
        struct select_offset_type
        {
            using offset_type = make_type_t<unsigned_type_tag, TypeSize>;
            using type = typename std::conditional<
#ifndef YATO_MSVC_2013
                (get_extra_length<Align, offset_type>::value <= std::numeric_limits<offset_type>::max()),
#else
# pragma warning(push)
# pragma warning(disable: 4293)
                (get_extra_length<Align, offset_type>::value <= (static_cast<uint64_t>(1) << TypeSize) - static_cast<uint64_t>(1)),
# pragma warning(pop)
#endif
                offset_type,
                typename select_offset_type<Align, 2 * TypeSize>::type
            >::type;
        };

        template <size_t Align, size_t TypeSize>
        struct select_offset_type <Align, TypeSize, typename std::enable_if<
            (TypeSize > 64)
        >::type>
        {
            using type = void;
        };

        template <typename T, size_t Align, typename = void>
        struct aligning_allocator_base
        {
            using byte_type = uint8_t;
            using offset_type = typename select_offset_type<Align>::type;
            static YATO_CONSTEXPR_VAR size_t extra_bytes = get_extra_length<Align, offset_type>::value;

            static
            byte_type* ceil_address(byte_type* p, size_t align) 
            {
                return reinterpret_cast<byte_type*>((reinterpret_cast<size_t>(p) / align) * align);
            }

            static 
            byte_type* get_offset_ptr(byte_type* aligned_ptr)
            {
                return ceil_address(aligned_ptr - sizeof(offset_type), std::alignment_of<offset_type>::value);
            }

            T* allocate_aligned(size_t n)
            {
                byte_type* const unaligned_ptr = new byte_type[n * sizeof(T) + extra_bytes];
                byte_type* const aligned_ptr = ceil_address(unaligned_ptr + extra_bytes, Align);
                YATO_ASSERT((unaligned_ptr <= aligned_ptr) && (aligned_ptr <= unaligned_ptr + extra_bytes), "aligned ptr is inside extra memory");
                *pointer_cast<offset_type*>(get_offset_ptr(aligned_ptr)) = narrow_cast<offset_type>(aligned_ptr - unaligned_ptr);
                return pointer_cast<T*>(aligned_ptr);
            }

            void deallocate_aligned(T* p, size_t)
            {
                byte_type* aligned_ptr = pointer_cast<byte_type*>(p);
                const offset_type offset = *pointer_cast<offset_type*>(get_offset_ptr(aligned_ptr));
                delete[] (aligned_ptr - offset);
            }
        };


        template <typename T, size_t Align>
        struct aligning_allocator_base <T, Align, typename std::enable_if <
            ((Align != 0) && !(Align & (Align - 1))) && // is power of 2
            (Align <= std::alignment_of<std::max_align_t>::value) // Extended alignment is not guaranteed 
        >::type>
        {
            static YATO_CONSTEXPR_VAR size_t extra_bytes = 0;
            using aligned_storage = typename std::aligned_storage<sizeof(T), Align>::type;

            T* allocate_aligned(size_t n)
            {
                return pointer_cast<T*>(new aligned_storage[n]);
            }

            void deallocate_aligned(T* p, size_t)
            {
                delete[] pointer_cast<aligned_storage*>(p);
            }
        };


        template <typename T, size_t Align>
        struct aligning_allocator_base <T, Align, typename std::enable_if<(Align == 0)>::type>
        { };
    }

    // ToDo (a.gruzdev): Use aligning new from C++17 
    template <typename T, size_t Align>
    class aligning_allocator
        : private details::aligning_allocator_base<T, Align>
    {
    private:
        using my_type = aligning_allocator<T, Align>;
        using super_type = details::aligning_allocator_base<T, Align>;

    public:
        static YATO_CONSTEXPR_VAR size_t alignment = Align;
        using super_type::extra_bytes;

        using value_type = T;
        using is_always_equal = std::true_type;
        using propagate_on_container_copy_assignment = std::false_type;
        using propagate_on_container_move_assignment = std::false_type;
        using propagate_on_container_swap = std::false_type;
#ifndef YATO_CXX17
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using void_pointer = void*;
        using const_void_pointer = const void*;
        using size_type = size_t;
        using difference_type = std::ptrdiff_t;
        //---------------------------------------------------
        template <typename U>
        struct rebind
        {
            using other = aligning_allocator<U, Align>;
        };
#endif
        //---------------------------------------------------

    private:
        using aligned_storage = typename std::aligned_storage<sizeof(value_type), alignment>::type;
        //---------------------------------------------------

    public:
        YATO_CONSTEXPR_FUNC
        aligning_allocator() YATO_NOEXCEPT_KEYWORD
        { }

        YATO_CONSTEXPR_FUNC
        aligning_allocator(const my_type &) YATO_NOEXCEPT_KEYWORD
        { }

        template <typename U, size_t A>
        YATO_CONSTEXPR_FUNC
        aligning_allocator(const aligning_allocator<U, A> &) YATO_NOEXCEPT_KEYWORD
        { }

        YATO_CONSTEXPR_FUNC
        aligning_allocator(my_type &&) YATO_NOEXCEPT_KEYWORD
        { }

        ~aligning_allocator() = default;

        aligning_allocator & operator = (const my_type &) YATO_NOEXCEPT_KEYWORD
        { }

        aligning_allocator & operator = (my_type &&) YATO_NOEXCEPT_KEYWORD
        { }

        pointer allocate(size_type n)
        {
            YATO_REQUIRES(n > 0);
            return super_type::allocate_aligned(n);
        }

        void deallocate(pointer p, size_t n)
        {
            super_type::deallocate_aligned(p, n);
        }

#ifndef YATO_CXX17
        YATO_CONSTEXPR_FUNC
        size_type max_size() const
        {
            return std::numeric_limits<size_type>::max() / sizeof(value_type);
        }

        pointer address(reference x) const
        {
            return std::addressof(x);
        }

        const_pointer address(const_reference x) const
        {
            return std::addressof(x);
        }

        template <typename U, typename... Args>
        void construct(U* p, Args &&... args)
        {
            new(pointer_cast<pointer>(p)) U(std::forward<Args>(args)...);
        }

        template <typename U>
        void destroy(U* p)
        {
            // C4100 can also be issued when code calls a destructor on a otherwise unreferenced parameter of primitive type. 
            // This is a limitation of the Visual C++ compiler
            YATO_MAYBE_UNUSED(p); 
            p->~U();
        }
#endif
    };

    template <typename T1, size_t Align1, typename T2, size_t Align2>
    YATO_CONSTEXPR_FUNC
    bool operator == (const aligning_allocator<T1, Align1> & one, const aligning_allocator<T2, Align2> & another)
    {
        return true;
    }

    template <typename T1, size_t Align1, typename T2, size_t Align2>
    YATO_CONSTEXPR_FUNC
    bool operator != (const aligning_allocator<T1, Align1> & one, const aligning_allocator<T2, Align2> & another)
    {
        return false;
    }
}

#endif
