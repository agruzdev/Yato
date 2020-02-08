/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_MEMORY_UTILITY_H_
#define _YATO_MEMORY_UTILITY_H_

#include <cstring>
#include <new>

#include "assertion.h"
#include "container_base.h"
#include "prerequisites.h"
#include "type_traits.h"

namespace yato
{

    /**
     * Fucntions for working with raw memory
     */
    namespace memory
    {
        /**
         * copy for trivial types
         */
        template <typename Ty_>
        void trivial_copy(const Ty_* src_first, const Ty_* src_last, Ty_* dst_first)
        {
            YATO_REQUIRES(src_first != nullptr);
            YATO_REQUIRES(src_last  != nullptr);
            YATO_REQUIRES(dst_first != nullptr);
            YATO_REQUIRES(std::distance(src_first, src_last) >= 0);
            YATO_REQUIRES((dst_first - src_last >= 0) || (src_first - (dst_first + std::distance(src_first, src_last)) >= 0));

            std::memcpy(dst_first, src_first, std::distance(src_first, src_last) * sizeof(Ty_));
        }

        /**
         * copy_n for trivial types
         */
        template <typename Ty_>
        void trivial_copy_n(const Ty_* src_first, size_t count, Ty_* dst_first)
        {
            YATO_REQUIRES(src_first != nullptr);
            YATO_REQUIRES(dst_first != nullptr);
            YATO_REQUIRES((dst_first - (src_first + count) >= 0) || (src_first - (dst_first + count) >= 0));

            std::memcpy(dst_first, src_first, count * sizeof(Ty_));
        }


        namespace details
        {
            template <typename Alloc_, typename Iter_, typename Ty_, typename = void>
            struct copy_operation
            {
                static
                void apply(Alloc_ & alloc, Iter_ src_first, Iter_ src_last, Ty_* & dst_first)
                {
                    using alloc_traits = std::allocator_traits<Alloc_>;
                    while(src_first != src_last) {
                        alloc_traits::construct(alloc, dst_first, *src_first++);
                        ++dst_first;
                    }
                }
            };

            template <typename Alloc_, typename Iter_, typename Ty_>
            struct copy_operation<Alloc_, Iter_, Ty_,
                    std::enable_if_t<
                        std::is_trivial<Ty_>::value && 
                        std::is_pointer<Iter_>::value &&
                        std::is_same<Ty_, std::remove_cv_t<typename std::iterator_traits<Iter_>::value_type>>::value
                    >
                >
            {
                static
                void apply(Alloc_ &, Iter_ src_first, Iter_ src_last, Ty_* & dst_first)
                {
                    const size_t count = std::distance(src_first, src_last);
                    trivial_copy_n(src_first, count, dst_first);
                    dst_first += count;
                }
            };



            template <typename Alloc_, typename Iter_, typename Ty_, typename = void>
            struct move_operation
            {
                static
                void apply(Alloc_ & alloc, Iter_ src_first, Iter_ src_last, Ty_* & dst_first)
                {
                    using alloc_traits = std::allocator_traits<Alloc_>;
                    while(src_first != src_last) {
                        alloc_traits::construct(alloc, dst_first, std::move(*src_first++));
                        ++dst_first;
                    }
                }
            };

            template <typename Alloc_, typename Iter_, typename Ty_>
            struct move_operation<Alloc_, Iter_, Ty_,
                    std::enable_if_t<
                        std::is_trivial<Ty_>::value && 
                        std::is_pointer<Iter_>::value &&
                        std::is_same<Ty_, std::remove_cv_t<typename std::iterator_traits<Iter_>::value_type>>::value
                    >
                >
            {
                static
                void apply(Alloc_ &, Iter_ src_first, Iter_ src_last, Ty_* & dst_first)
                {
                    const size_t count = std::distance(src_first, src_last);
                    trivial_copy_n(src_first, count, dst_first);
                    dst_first += count;
                }
            };



            template <typename Alloc_, typename Ty_, typename = void>
            struct destroy_operation
            {
                static
                void apply(Alloc_ & alloc, Ty_* first, Ty_* last)
                {
                    using alloc_traits = std::allocator_traits<Alloc_>;
                    while(first != last) {
                        alloc_traits::destroy(alloc, first++);
                    }
                }
            };

            template <typename Alloc_, typename Ty_>
            struct destroy_operation<Alloc_, Ty_,
                    std::enable_if_t<std::is_trivial<Ty_>::value>
                >
            {
                static
                void apply(Alloc_ &, Ty_*, Ty_*)
                { }
            };



            template <typename Alloc_, typename Ty_, typename = void>
            struct transfer_operation
            {
                static
                void apply(Alloc_ & alloc, const Ty_* src_first, const Ty_* src_last, Ty_* & dst_first)
                {
                    using alloc_traits = std::allocator_traits<Alloc_>;
                    while(src_first != src_last) {
                        alloc_traits::construct(alloc, dst_first, std::move_if_noexcept(*src_first++));
                        ++dst_first;
                    }
                }
            };

            template <typename Alloc_, typename Ty_>
            struct transfer_operation<Alloc_, Ty_,
                    std::enable_if_t<std::is_trivial<Ty_>::value>
                >
            {
                static
                void apply(Alloc_ &, const Ty_* src_first, const Ty_* src_last, Ty_* & dst_first)
                {
                    const size_t count = std::distance(src_first, src_last);
                    trivial_copy_n(src_first, count, dst_first);
                    dst_first += count;
                }
            };



            template <typename Alloc_, typename Ty_, typename = void>
            struct transfer_left_operation
            {
                static
                void apply(Alloc_ & alloc, const Ty_* src_first, const Ty_* src_last, Ty_* & dst_first)
                {
                    using alloc_traits = std::allocator_traits<Alloc_>;
                    while(src_first != src_last) {
                        alloc_traits::destroy(alloc, dst_first);
                        alloc_traits::construct(alloc, dst_first, std::move_if_noexcept(*src_first));
                        ++dst_first;
                        ++src_first;
                    }
                }
            };

            template <typename Alloc_, typename Ty_>
            struct transfer_left_operation<Alloc_, Ty_,
                    std::enable_if_t<std::is_trivial<Ty_>::value>
                >
            {
                static
                void apply(Alloc_ &, const Ty_* src_first, const Ty_* src_last, Ty_* & dst_first)
                {
                    const size_t count = std::distance(src_first, src_last);
                    std::memmove(dst_first, src_first, count * sizeof(Ty_));
                    dst_first += count;
                }
            };



            template <typename Alloc_, typename Ty_, typename = void>
            struct transfer_right_operation
            {
                static
                void apply(Alloc_ & alloc, const Ty_* src_first, const Ty_* src_last, Ty_* & dst_last)
                {
                    using alloc_traits = std::allocator_traits<Alloc_>;
                    const Ty_* src_rfirst = src_first - 1;
                    const Ty_* src_rlast  = src_last  - 1;
                    while(src_rfirst != src_rlast) {
                        alloc_traits::destroy(alloc, dst_last);
                        alloc_traits::construct(alloc, dst_last, std::move_if_noexcept(*src_rlast));
                        --dst_last;
                        --src_rlast;
                    }
                }
            };

            template <typename Alloc_, typename Ty_>
            struct transfer_right_operation<Alloc_, Ty_,
                    std::enable_if_t<std::is_trivial<Ty_>::value>
                >
            {
                static
                void apply(Alloc_ &, const Ty_* src_first, const Ty_* src_last, Ty_* & dst_last)
                {
                    const size_t count = std::distance(src_first, src_last);
                    std::memmove(dst_last - count + 1, src_first, count * sizeof(Ty_));
                    dst_last -= count;
                }
            };
        }


        /**
         * Allocate raw memory
         */
        template <typename Alloc_>
        auto allocate(Alloc_ & alloc, size_t count)
            -> typename std::allocator_traits<Alloc_>::pointer
        {
            using alloc_traits = std::allocator_traits<Alloc_>;
            YATO_REQUIRES(count > 0);
            YATO_REQUIRES(count <= alloc_traits::max_size(alloc));
            return alloc_traits::allocate(alloc, count);
        }

        /**
         * Deallocate raw memory
         */
        template <typename Alloc_>
        void deallocate(Alloc_ & alloc, typename std::allocator_traits<Alloc_>::pointer ptr, size_t count)
        {
            using alloc_traits = std::allocator_traits<Alloc_>;
            alloc_traits::deallocate(alloc, ptr, count);
        }

        /**
         * Fill raw memory with a value
         * 'first' iterator is passed by reference for handling exceptions.
         */
        template <typename Alloc_, typename Ty_, typename ... InitArgs_>
        void fill_uninitialized(Alloc_ & alloc, Ty_* & first, Ty_* last, InitArgs_ && ... init_args)
        {
            YATO_REQUIRES(first != nullptr);
            YATO_REQUIRES(last  != nullptr);
            YATO_REQUIRES(std::distance(first, last) >= 0);

            using alloc_traits = std::allocator_traits<Alloc_>;
            while(first != last) {
                alloc_traits::construct(alloc, first, std::forward<InitArgs_>(init_args)...);
                ++first;
            }
        }


        /**
         * Init raw memory with copied values.
         * dst_first must not belong [src_first, src_last)
         */
        template <typename Alloc_, typename Iter_, typename Ty_>
        void copy_to_uninitialized(Alloc_ & alloc, Iter_ src_first, Iter_ src_last, Ty_* & dst_first)
        {
            YATO_REQUIRES(dst_first != nullptr);
            YATO_REQUIRES(std::distance(src_first, src_last) >= 0);

            details::copy_operation<Alloc_, Iter_, Ty_>::apply(alloc, src_first, src_last, dst_first);
        }

        /**
         * Init raw memory with copied values.
         * dst_first must not belong [src_first, src_last)
         */
        template <typename Alloc_, typename Iter_, typename Ty_>
        auto copy_to_uninitialized_multidim(Alloc_ & alloc, Iter_ src_first, const Iter_ & src_last, Ty_* & dst_first)
            -> std::enable_if_t<(Iter_::dimensions_number > 1)>
        {
            YATO_REQUIRES(dst_first != nullptr);
            YATO_REQUIRES(std::distance(src_first, src_last) >= 0);
            if (src_first.makes_plain_range()) {
                copy_to_uninitialized(alloc, (*src_first).plain_cbegin(), (*src_last).plain_cbegin(), dst_first);
            }
            else {
                while(src_first != src_last) {
                    copy_to_uninitialized_multidim(alloc, (*src_first).cbegin(), (*src_first).cend(), dst_first);
                    ++src_first;
                }
            }
        }

        /**
         * Init raw memory with copied values.
         * dst_first must not belong [src_first, src_last)
         */
        template <typename Alloc_, typename Iter_, typename Ty_>
        auto copy_to_uninitialized_multidim(Alloc_ & alloc, Iter_ src_first, const Iter_ & src_last, Ty_* & dst_first)
            -> std::enable_if_t<(Iter_::dimensions_number == 1)>
        {
            YATO_REQUIRES(dst_first != nullptr);
            YATO_REQUIRES(std::distance(src_first, src_last) >= 0);
            if (src_first.makes_plain_range()) {
                copy_to_uninitialized(alloc, (*src_first).plain_cbegin(), (*src_last).plain_cbegin(), dst_first);
            }
            else {
                while(src_first != src_last) {
                    copy_to_uninitialized(alloc, (*src_first).cbegin(), (*src_first).cend(), dst_first);
                    ++src_first;
                }
            }
        }

        /**
         * Fallback for not multidimensional iterators
         */
        template <typename Alloc_, typename Iter_, typename Ty_>
        auto copy_to_uninitialized_multidim(Alloc_ & alloc, const Iter_ & src_first, const Iter_ & src_last, Ty_* & dst_first)
            -> std::enable_if_t<!yato::is_multidimensional<Iter_>::value>
        {
            copy_to_uninitialized(alloc, src_first, src_last, dst_first);
        }


        /**
         * Init raw memory with moved values
         * dst_first must not belong [src_first, src_last)
         */
        template <typename Alloc_, typename Iter_, typename Ty_>
        void move_to_uninitialized(Alloc_ & alloc, Iter_ src_first, Iter_ src_last, Ty_* & dst_first)
        {
            YATO_REQUIRES(dst_first != nullptr);
            YATO_REQUIRES(std::distance(src_first, src_last) >= 0);

            details::move_operation<Alloc_, Iter_, Ty_>::apply(alloc, src_first, src_last, dst_first);
        }


        /**
         * Destroy a range of values, leaving memory uninitialized
         * Does nothing for trivial types
         */
        template <typename Alloc_, typename Ty_>
        void destroy(Alloc_ & alloc, Ty_* first, Ty_* last)
        {
            YATO_REQUIRES(first != nullptr);
            YATO_REQUIRES(last  != nullptr);
            YATO_REQUIRES(std::distance(first, last) >= 0);

            details::destroy_operation<Alloc_, Ty_>::apply(alloc, first, last);
        }


        /**
         * Init raw memory with copied or moved values
         * dst_first must not belong [src_first, src_last)
         */
        template <typename Alloc_, typename Ty_>
        void transfer_to_uninitialized(Alloc_ & alloc, const Ty_* src_first, const Ty_* src_last, Ty_* & dst_first)
        {
            YATO_REQUIRES(src_first != nullptr);
            YATO_REQUIRES(src_last  != nullptr);
            YATO_REQUIRES(dst_first != nullptr);
            YATO_REQUIRES(std::distance(src_first, src_last) >= 0);
            YATO_REQUIRES((dst_first - src_last >= 0) || (src_first - dst_first - std::distance(src_first, src_last) >= 0))
            
            details::transfer_operation<Alloc_, Ty_>::apply(alloc, src_first, src_last, dst_first);
        }


        /**
         * Transfer a block of values to the left. Destroys values vefore rewriting.
         * 
         * [XXXX][YYYY][ZZZZ]
         * [XXXX][ZZZZ][____]
         *       dst   src   
         */
        template <typename Alloc_, typename Ty_>
        void transfer_to_left(Alloc_ & alloc, const Ty_* src_first, const Ty_* src_last, Ty_* & dst_first)
        {
            YATO_REQUIRES(src_first != nullptr);
            YATO_REQUIRES(src_last  != nullptr);
            YATO_REQUIRES(dst_first != nullptr);
            YATO_REQUIRES(std::distance(src_first, src_last) >= 0);
            YATO_REQUIRES(src_first - dst_first > 0);

            details::transfer_left_operation<Alloc_, Ty_>::apply(alloc, src_first, src_last, dst_first);
        }


        /**
         * Transfer a block of values to the right in backward order.
         * Copies the elements from the range, defined by [first, first + size), to another range ending at dst_last
         * Destroys values on rewriting.
         * 
         * [XXXX][YYYY][ZZZZ]
         * [XXXX][____][YYYY]
         *       src   dst   
         */
        template <typename Alloc_, typename Ty_>
        void transfer_to_right(Alloc_ & alloc, const Ty_* src_first, const Ty_* src_last, Ty_* & dst_last)
        {
            YATO_REQUIRES(src_first != nullptr);
            YATO_REQUIRES(src_last  != nullptr);
            YATO_REQUIRES(dst_last != nullptr);
            YATO_REQUIRES(std::distance(src_first, src_last) >= 0);
            YATO_REQUIRES(dst_last - src_first > 0);
            
            details::transfer_right_operation<Alloc_, Ty_>::apply(alloc, src_first, src_last, dst_last);
        }


    } // namespace memory

#ifdef YATO_HAS_LAUNDER
    using std::launder;
#else
    template <typename Ty_>
    YATO_CONSTEXPR_FUNC
    Ty_* launder(Ty_* p) YATO_NOEXCEPT_KEYWORD
    {
        return p;
    }
#endif


} // namespace yato

#endif

