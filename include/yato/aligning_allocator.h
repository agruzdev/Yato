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

    template <typename T, size_t Align, typename = void>
    class aligning_allocator
    { };


    // ToDo (a.gruzdev): Use aligning new from C++17 
    template <typename T, size_t Align>
    class aligning_allocator <T, Align, typename std::enable_if<
        ((Align != 0) && !(Align & (Align - 1))) && // is power of 2
        (Align > 0) &&  // Alignment 0 is undefined behaviour
        (Align <= std::alignment_of<std::max_align_t>::value) // Extended alignment is not guaranteed 
    >::type>
    {
    private:
        using my_type = aligning_allocator<T, Align>;
    public:
        static YATO_CONSTEXPR_VAR size_t alignment = Align;
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
            return pointer_cast<pointer>(new aligned_storage[n]);
        }

        void deallocate(pointer p, size_t)
        {
            delete[] pointer_cast<aligned_storage*>(p);
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
