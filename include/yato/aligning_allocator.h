/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#ifndef _YATO_ALIGNING_ALLOCATOR_H_
#define _YATO_ALIGNING_ALLOCATOR_H_

#include <cstddef>
#include "types.h"

namespace yato
{

    namespace details
    {
        template <size_t Align_, typename OffsetType_>
        struct get_extra_length
            : public std::integral_constant<size_t, Align_ - 1 + sizeof(OffsetType_) + std::alignment_of<OffsetType_>::value - 1>
        { };

        template <size_t Align_, size_t TypeSize_ = 8, typename = void>
        struct select_offset_type
        {
            using offset_type = make_type_t<unsigned_type_tag, TypeSize_>;
            using type = typename std::conditional<
#ifndef YATO_MSVC_2013
                (get_extra_length<Align_, offset_type>::value <= std::numeric_limits<offset_type>::max()),
#else
                (get_extra_length<Align_, offset_type>::value <= (((static_cast<uint64_t>(1) << (TypeSize - 1)) - 1) << 1) + 1),
#endif
                offset_type,
                typename select_offset_type<Align_, 2 * TypeSize_>::type
            >::type;
        };

        template <size_t Align_, size_t TypeSize_>
        struct select_offset_type <Align_, TypeSize_, typename std::enable_if<
            (TypeSize_ > 64)
        >::type>
        {
            using type = void;
        };

        template <typename Ty_, size_t Align_, typename = void>
        struct aligning_allocator_base
        {
            using byte_type = uint8_t;
            using offset_type = typename select_offset_type<Align_>::type;
            static YATO_CONSTEXPR_VAR size_t extra_bytes = get_extra_length<Align_, offset_type>::value;

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

            Ty_* allocate_aligned(size_t n)
            {
                byte_type* const unaligned_ptr = new byte_type[n * sizeof(Ty_) + extra_bytes];
                byte_type* const aligned_ptr = ceil_address(unaligned_ptr + extra_bytes, Align_);
                YATO_ASSERT((unaligned_ptr <= aligned_ptr) && (aligned_ptr <= unaligned_ptr + extra_bytes), "aligned ptr is inside extra memory");
                *pointer_cast<offset_type*>(get_offset_ptr(aligned_ptr)) = narrow_cast<offset_type>(aligned_ptr - unaligned_ptr);
                return pointer_cast<Ty_*>(aligned_ptr);
            }

            void deallocate_aligned(Ty_* p, size_t)
            {
                byte_type* aligned_ptr = pointer_cast<byte_type*>(p);
                const offset_type offset = *pointer_cast<offset_type*>(get_offset_ptr(aligned_ptr));
                delete[] (aligned_ptr - offset);
            }
        };


        template <typename Ty_, size_t Align_>
        struct aligning_allocator_base <Ty_, Align_, typename std::enable_if <
            ((Align_ != 0) && !(Align_ & (Align_ - 1))) && // is power of 2
            (Align_ <= std::alignment_of<std::max_align_t>::value) // Extended alignment is not guaranteed 
        >::type>
        {
            static YATO_CONSTEXPR_VAR size_t extra_bytes = 0;
            using aligned_storage = typename std::aligned_storage<sizeof(Ty_), Align_>::type;

            Ty_* allocate_aligned(size_t n)
            {
                return pointer_cast<Ty_*>(new aligned_storage[n]);
            }

            void deallocate_aligned(Ty_* p, size_t)
            {
                delete[] pointer_cast<aligned_storage*>(p);
            }
        };


        template <typename Ty_, size_t Align_>
        struct aligning_allocator_base <Ty_, Align_, typename std::enable_if<(Align_ == 0)>::type>
        { };
    }


    template <typename Ty_, size_t Align_>
    class aligning_allocator
        : private details::aligning_allocator_base<Ty_, Align_>
    {
    private:
        using my_type = aligning_allocator<Ty_, Align_>;
        using super_type = details::aligning_allocator_base<Ty_, Align_>;

    public:
        static YATO_CONSTEXPR_VAR size_t alignment = Align_;
        using super_type::extra_bytes;

        using value_type = Ty_;
        using is_always_equal = std::true_type;
        using propagate_on_container_copy_assignment = std::false_type;
        using propagate_on_container_move_assignment = std::false_type;
        using propagate_on_container_swap = std::false_type;

#ifndef YATO_CXX17
        using pointer = Ty_*;
        using const_pointer = const Ty_*;
        using reference = Ty_&;
        using const_reference = const Ty_&;
        using void_pointer = void*;
        using const_void_pointer = const void*;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        //---------------------------------------------------

#endif
        // Standard rebind works only for Alloc<typename, typename...> and is not sauitable for Alloc<typename, size_t>.
        template <typename Uy_>
        struct rebind
        {
            using other = aligning_allocator<Uy_, Align_>;
        };
        //---------------------------------------------------

        YATO_CONSTEXPR_FUNC
        aligning_allocator() = default;

        YATO_CONSTEXPR_FUNC
        aligning_allocator(const my_type &) = default;

        template <typename U, size_t A>
        YATO_CONSTEXPR_FUNC
        aligning_allocator(const aligning_allocator<U, A> &) YATO_NOEXCEPT_KEYWORD
        { }

        YATO_CONSTEXPR_FUNC
        aligning_allocator(my_type &&) noexcept = default;

        ~aligning_allocator() = default;

        aligning_allocator & operator = (const my_type &) = default;

        aligning_allocator & operator = (my_type &&) noexcept = default;

        Ty_* allocate(std::size_t n)
        {
            YATO_REQUIRES(n > 0);
            return super_type::allocate_aligned(n);
        }

        void deallocate(Ty_* p, std::size_t n)
        {
            super_type::deallocate_aligned(p, n);
        }

#ifndef YATO_CXX17
        YATO_CONSTEXPR_FUNC
        std::size_t max_size() const
        {
            return std::numeric_limits<std::size_t>::max() / sizeof(value_type);
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

    template <typename Ty1_, size_t Align1_, typename Ty2_, size_t Align2_>
    YATO_CONSTEXPR_FUNC
    bool operator == (const aligning_allocator<Ty1_, Align1_> &, const aligning_allocator<Ty2_, Align2_> &)
    {
        return true;
    }

    template <typename Ty1_, size_t Align1_, typename Ty2_, size_t Align2_>
    YATO_CONSTEXPR_FUNC
    bool operator != (const aligning_allocator<Ty1_, Align1_> &, const aligning_allocator<Ty2_, Align2_> &)
    {
        return false;
    }
}

#endif
