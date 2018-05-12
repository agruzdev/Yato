/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_MEMORY_UTILITY_H_
#define _YATO_MEMORY_UTILITY_H_

#include "type_traits.h"
#include "assert.h"

namespace yato
{

    /**
     * Fucntions for working with raw memory
     */
    namespace memory
    {

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

            using alloc_traits = std::allocator_traits<Alloc_>;
            while(src_first != src_last) {
                alloc_traits::construct(alloc, dst_first, *src_first++);
                ++dst_first;
            }
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

            using alloc_traits = std::allocator_traits<Alloc_>;
            while(src_first != src_last) {
                alloc_traits::construct(alloc, dst_first, std::move(*src_first++));
                ++dst_first;
            }
        }

        /**
         * Destroy a range of values, leaving memory uninitialized
         */
        template <typename Alloc_, typename Ty_>
        void destroy(Alloc_ & alloc, Ty_* first, Ty_* last)
        {
            YATO_REQUIRES(first != nullptr);
            YATO_REQUIRES(last  != nullptr);
            YATO_REQUIRES(std::distance(first, last) >= 0);

            using alloc_traits = std::allocator_traits<Alloc_>;
            while(first != last) {
                alloc_traits::destroy(alloc, first++);
            }
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
            
            using alloc_traits = std::allocator_traits<Alloc_>;
            while(src_first != src_last) {
                alloc_traits::construct(alloc, dst_first, std::move_if_noexcept(*src_first++));
                ++dst_first;
            }
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
            
            using alloc_traits = std::allocator_traits<Alloc_>;
            while(src_first != src_last) {
                alloc_traits::destroy(alloc, dst_first);
                alloc_traits::construct(alloc, dst_first, std::move_if_noexcept(*src_first));
                ++dst_first;
                ++src_first;
            }
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

    }
}

#endif

