/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_ND_H_
#define _YATO_ARRAY_ND_H_

#include <array>
#include <vector>

#include "array_view.h"
#include "assert.h"
#include "compressed_pair.h"
#include "memory_utility.h"
#include "range.h"

namespace yato
{
    /**
     * Tag type for allocating uninitialized vector
     */
    struct uninitialized_t {};


    namespace details
    {

        /*
         * Make type of multidimensional initializer list
         */
        template<typename _T, size_t _Dims>
        struct initilizer_list_nd
        {
            using type = std::initializer_list<typename initilizer_list_nd<_T, _Dims - 1>::type>;
        };

        template<typename _T>
        struct initilizer_list_nd<_T, 1>
        {
            using type = std::initializer_list<_T>;
        };


        //-------------------------------------------------------
        // Generic implementation

        template <typename _DataType, size_t _DimensionsNum, typename _Allocator>
        class vector_nd_impl
        {
        public:
            /*
             * Public traits of the multidimensional vector
             */
            using this_type = vector_nd_impl<_DataType, _DimensionsNum, _Allocator>;
            using dimensions_type = dimensionality<_DimensionsNum, size_t>;
            using data_type = _DataType;
            using value_type = _DataType;
            using allocator_type = _Allocator;
            using data_iterator = _DataType*;
            using const_data_iterator = const _DataType*;
            using reference = decltype(*std::declval<data_iterator>());
            using const_reference = decltype(*std::declval<const_data_iterator>());
            using size_type = size_t;

            using dim_descriptor = dimension_descriptor<size_type>;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = _DimensionsNum;
            static_assert(dimensions_number > 1, "Implementation for dimensions number larger than 1");
            //-------------------------------------------------------

        private:
            using raw_pointer = _DataType*;
            using compressed_storage_type = yato::compressed_pair<allocator_type, raw_pointer>;
            using alloc_traits = std::allocator_traits<allocator_type>;

            using size_iterator = typename dimensions_type::iterator;
            using size_const_iterator = typename dimensions_type::const_iterator;

            template<size_t _Dims>
            using initilizer_type = typename initilizer_list_nd<data_type, _Dims>::type;

            template<typename SomeDataIter, typename SomeDescriptor>
            using proxy_tmpl = details::sub_array_proxy<SomeDataIter, SomeDescriptor, dimensions_number - 1>;

            using proxy       = proxy_tmpl<data_iterator,       dim_descriptor>;
            using const_proxy = proxy_tmpl<const_data_iterator, dim_descriptor>;

        public:
            using iterator = proxy;
            using const_iterator = const_proxy;

            //-------------------------------------------------------

        private:
            std::array<dim_descriptor::type, dimensions_number> m_descriptors = {};
            size_t m_allocated_size = 0;
            compressed_storage_type m_data;
            //-------------------------------------------------------


            YATO_FORCED_INLINE
            raw_pointer & raw_ptr_()
            {
                return m_data.second();
            }

            YATO_FORCED_INLINE
            const raw_pointer & raw_ptr_() const
            {
                return m_data.second();
            }

            YATO_FORCED_INLINE
            allocator_type & allocator_()
            {
                return m_data.first();
            }

            YATO_FORCED_INLINE
            const allocator_type & allocator_() const
            {
                return m_data.first();
            }

            template <typename ... InitArgs_>
            void raw_init_(size_t plain_size, InitArgs_ && ... init_args)
            {
                YATO_REQUIRES(plain_size != 0);

                allocator_type& alloc = allocator_();
                value_type* const new_data = memory::allocate(alloc, plain_size);
                value_type* dst = new_data;
                try {
                    memory::fill_uninitialized(alloc, dst, new_data + plain_size, std::forward<InitArgs_>(init_args)...);
                }
                catch(...) {
                    memory::destroy(alloc, new_data, dst);
                    memory::deallocate(alloc, new_data, plain_size);
                    throw;
                }
                raw_ptr_() = new_data;
                m_allocated_size = plain_size;
            }

            template <typename IterTy_>
            void raw_init_from_range_(size_t plain_size, IterTy_ src_first, IterTy_ src_last)
            {
                YATO_REQUIRES(plain_size != 0);
                YATO_REQUIRES(yato::narrow_cast<std::ptrdiff_t>(plain_size) == std::distance(src_first, src_last));

                allocator_type& alloc = allocator_();
                value_type* new_data = memory::allocate(alloc, plain_size);
                value_type* dst = new_data;
                try {
                    memory::copy_to_uninitialized(alloc, src_first, src_last, dst);
                }
                catch(...) {
                    memory::destroy(alloc, new_data, dst);
                    memory::deallocate(alloc, new_data, plain_size);
                    throw;
                }
                raw_ptr_() = new_data;
                m_allocated_size = plain_size;
            }

            void raw_init_from_ilist_(size_t plain_size, const initilizer_type<dimensions_number> & init_list)
            {
                YATO_REQUIRES(plain_size != 0);

                allocator_type& alloc = allocator_();
                value_type* new_data = memory::allocate(alloc, plain_size);
                value_type* dst = new_data; // copy, that can be changed
                try {
                    // Recursively copy values from the initializser list
                    raw_copy_initializer_list_<dimensions_number>(init_list, dst);
                }
                catch(...) {
                    memory::destroy(alloc, new_data, dst);
                    memory::deallocate(alloc, new_data, plain_size);
                    throw;
                }
                raw_ptr_() = new_data;
                m_allocated_size = plain_size;
            }

            template <typename ... Args_>
            void raw_resize_(size_t old_size, size_t new_size, Args_ && ... init_args)
            {
                if(new_size < old_size) {
                    value_type* const old_data = raw_ptr_();
                    memory::destroy(allocator_(), old_data + new_size, old_data + old_size);
                }
                else if(new_size > old_size) {
                    allocator_type& alloc = allocator_();
                    if(new_size > m_allocated_size) {
                        value_type* const old_data = raw_ptr_();
                        value_type* const new_data = memory::allocate(alloc, new_size);
                        value_type* dst = new_data;
                        try {
                            // transfer old
                            if(old_data != nullptr) {
                                memory::transfer_to_uninitialized(alloc, old_data, old_data + old_size, dst);
                            }
                            // fill added space
                            memory::fill_uninitialized(alloc, dst, new_data + new_size, std::forward<Args_>(init_args)...);
                        }
                        catch(...) {
                            memory::destroy(alloc, new_data, dst);
                            memory::deallocate(alloc, new_data, new_size);
                            throw;
                        }
                        // delete old
                        if(old_data != nullptr) {
                            memory::destroy(alloc, old_data, old_data + old_size);
                            memory::deallocate(alloc, old_data, m_allocated_size);
                        }
                        raw_ptr_() = new_data;
                        m_allocated_size = new_size;
                    } else {
                        // enough space
                        value_type* const old_data = raw_ptr_();
                        value_type* dst = old_data + old_size;
                        try {
                            memory::fill_uninitialized(alloc, dst, old_data + new_size, std::forward<Args_>(init_args)...);
                        }
                        catch(...) {
                            // delete only new elements
                            memory::destroy(alloc, old_data + old_size, dst);
                            throw;
                        }
                    }
                }
            }

            YATO_FORCED_INLINE
            size_t raw_offset_checked_(const const_iterator & position) const
            {
                const std::ptrdiff_t offset = std::distance<const_data_iterator>(raw_ptr_(), position.plain_cbegin());
#if YATO_DEBUG
                if (offset < 0) {
                    throw yato::argument_error("yato::vector_nd: position iterator doesn't belong to this vector!");
                }
                if (static_cast<size_t>(offset) > total_size()) {
                    throw yato::argument_error("yato::vector_nd: position iterator doesn't belong to this vector!");
                }
#endif
                return yato::narrow_cast<size_t>(offset);
            }

            template <typename SizeIterator_>
            bool match_sub_dimensions_(const yato::range<SizeIterator_> & sub_dims)
            {
                auto current_sub_dims = dimensions_ref_range_().tail();
                if(sub_dims.distance() != current_sub_dims.distance()) {
                    return false;
                }
                if(total_size() != 0) {
                    if(!std::equal(sub_dims.begin(), sub_dims.end(), current_sub_dims.begin())) {
                        return false;
                    }
                }
                return true;
            }

            template <typename SizeIterator_>
            void init_dimensions_if_empty_(const yato::range<SizeIterator_> & sub_dims)
            {
                if(total_size() == 0) {
                    std::get<dim_descriptor::idx_size>(m_descriptors[0]) = 0;
                    auto dim = sub_dims.begin();
                    for(size_t i = 0; i < dimensions_number - 1; ++i) {
                        std::get<dim_descriptor::idx_size>(m_descriptors[i + 1]) = *dim++;
                    }
                    init_subsizes_();
                }
            }

            yato::range<value_type*> raw_prepare_push_back_(size_t old_size, size_t new_size)
            {
                if(new_size > m_allocated_size) {
                    allocator_type& alloc = allocator_();
                    value_type* new_data = memory::allocate(alloc, new_size);
                    value_type* old_data = raw_ptr_();
                    if(old_data != nullptr) {
                        value_type* dst = new_data;
                        try {
                            memory::transfer_to_uninitialized(alloc, old_data, old_data + old_size, dst);
                        }
                        catch(...) {
                            memory::destroy(alloc, new_data, dst);
                            memory::deallocate(alloc, new_data, new_size);
                            throw;
                        }
                        memory::destroy(alloc, old_data, old_data + old_size);
                        memory::deallocate(alloc, old_data, m_allocated_size);
                    }
                    raw_ptr_() = new_data;
                    m_allocated_size = new_size;
                }
                return yato::make_range(std::next(raw_ptr_(), old_size), std::next(raw_ptr_(), new_size));
            }

            yato::range<value_type*> raw_prepare_insert_(size_t insert_offset, size_t insert_size, size_t element_size)
            {
                const size_t old_size = total_size();
                const size_t new_size = old_size + insert_size;
                YATO_ASSERT(new_size > old_size, "Invalid size");
                if(new_size > m_allocated_size) {
                    allocator_type& alloc = allocator_();
                    value_type* new_data = memory::allocate(alloc, new_size);
                    value_type* old_data = raw_ptr_();
                    if(old_data != nullptr) {
                        // transfer left part
                        value_type* dst_left = new_data;
                        try {
                            memory::transfer_to_uninitialized(alloc, old_data, old_data + insert_offset, dst_left);
                        }
                        catch(...) {
                            memory::destroy(alloc, new_data, dst_left);
                            memory::deallocate(alloc, new_data, new_size);
                            throw;
                        }
                        // transfer right part
                        value_type* dst_right = new_data + insert_offset + insert_size;
                        try {
                            memory::transfer_to_uninitialized(alloc, old_data + insert_offset, old_data + old_size, dst_right);
                        }
                        catch(...) {
                            // destroy both ranges
                            memory::destroy(alloc, new_data, dst_left);
                            memory::destroy(alloc, new_data + insert_offset + insert_size, dst_right);
                            memory::deallocate(alloc, new_data, new_size);
                            throw;
                        }
                        // destroy old
                        memory::destroy(alloc, old_data, old_data + old_size);
                        memory::deallocate(alloc, old_data, m_allocated_size);
                    }
                    raw_ptr_() = new_data;
                    m_allocated_size = new_size;
                } else {
                    allocator_type& alloc = allocator_();
                    // enough of allocated size
                    value_type* old_data = raw_ptr_();
                    // uninitialized data size
                    const size_t right_size = old_size - insert_offset;
                    if(insert_size >= right_size) {
                        // no overlap
                        value_type* dst = old_data + insert_offset + insert_size;
                        try {
                            // [old_data + insert_offset, old_data + insert_offset + right_size)
                            memory::transfer_to_uninitialized(alloc, old_data + insert_offset, old_data + old_size, dst);
                        }
                        catch(...) {
                            memory::destroy(alloc, old_data + insert_offset + insert_size, dst);
                            throw;
                        }
                        memory::destroy(alloc, old_data + insert_offset, old_data + old_size);
                    } else {
                        // tail
                        const size_t tail_size = new_size - old_size;
                        value_type* dst_tail = old_data + new_size - tail_size;
                        try {
                            memory::transfer_to_uninitialized(alloc, old_data + old_size - tail_size, old_data + old_size, dst_tail);
                        }
                        catch(...) {
                            // delete only tail to keep old_size of constructed elements
                            memory::destroy(alloc, old_data + new_size - tail_size, dst_tail);
                            throw;
                        }
                        // overlapped part
                        YATO_ASSERT(old_size > 0, "Overlap case is not possible for empty vector");
                        value_type* dst = old_data + old_size - 1;
                        try {
                            // [old_data + insert_offset, old_data + insert_offset + right_size - tail_size)
                            memory::transfer_to_right(alloc, old_data + insert_offset, old_data + old_size - tail_size, dst);
                        }
                        catch(...) {
                            // the only uninitialized element is at dst position
                            const size_t valid_count = (dst - old_data) / element_size;
                            memory::destroy(alloc, old_data + valid_count * element_size, dst);
                            memory::destroy(alloc, dst + 1, dst_tail);
                            update_top_dimension_(valid_count);
                        }
                        memory::destroy(alloc, old_data + insert_offset, old_data + insert_size);
                    }
                }
                return yato::make_range(std::next(raw_ptr_(), insert_offset), std::next(raw_ptr_(), insert_offset + insert_size));
            }

            value_type* raw_erase_(const const_iterator & position, size_t count, size_t element_size)
            {
                YATO_REQUIRES(count != 0);
                YATO_REQUIRES(get_top_dimension_() >= count);

                const size_t erase_size = yato::narrow_cast<size_t>(count * element_size);
                const size_t erase_offset = raw_offset_checked_(position);

                const size_t old_size = total_size();
                YATO_ASSERT(old_size >= erase_size, "Invalid offsets");
                const size_t new_size = old_size - erase_size;

                allocator_type& alloc = allocator_();

                value_type* old_data = raw_ptr_();
                // transfer right part
                value_type* dst = old_data + erase_offset;
                try {
                    memory::transfer_to_left(alloc, old_data + erase_offset + erase_size, old_data + old_size, dst);
                }
                catch(...) {
                    // the only uninitialized element is at dst position
                    const size_t valid_count = (dst - old_data) / element_size;
                    memory::destroy(alloc, old_data + valid_count * element_size, dst);
                    // dst + 1 is always valid because erase_size > 0
                    memory::destroy(alloc, dst + 1, old_data + old_size);
                    update_top_dimension_(valid_count);
                    throw;
                }
                // destroy tail
                memory::destroy(alloc, old_data + new_size, old_data + old_size);
                // update size
                update_top_dimension_(get_top_dimension_() - count);
                // result position
                return old_data + erase_offset;
            }

            template<size_t _Depth, typename _Iter>
            auto raw_copy_initializer_list_(const initilizer_type<_Depth> & init_list, _Iter & dst)
                -> typename std::enable_if < (_Depth > 1), void > ::type
            {
                const size_t dim_size = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - _Depth]);
                if(dim_size != init_list.size()) {
                    throw yato::argument_error("yato::vector_nd[ctor]: Invalid form of the initializer list.");
                }
                for (const auto & init_sub_list : init_list) {
                    raw_copy_initializer_list_<_Depth - 1>(init_sub_list, dst);
                }
            }

            template<size_t _Depth, typename _Iter>
            auto raw_copy_initializer_list_(const initilizer_type<_Depth> & init_list, _Iter & dst)
                -> typename std::enable_if<(_Depth == 1), void>::type
            {
                const size_t dim_size = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - _Depth]);
                YATO_ASSERT(dim_size >= init_list.size(), "Size was deduced incorrectly!");
                allocator_type & alloc = allocator_();
                for (const auto & value : init_list) {
                    alloc_traits::construct(alloc, dst, value);
                    ++dst; // increment after successful copy
                }
                size_t tail = dim_size - init_list.size();
                if(tail != 0) {
                    if(0 == init_list.size()) {
                        throw yato::argument_error("yato::vector_nd[ctor]: Can't be constructed from empty initialaser sub-list.");
                    }
                    const value_type & init_val = *(std::next(init_list.begin(), init_list.size() - 1));
                    while(tail) {
                        alloc_traits::construct(alloc, dst, init_val); // copy the last one
                        ++dst; // increment after successful copy
                        --tail;
                    }
                }
            }

            void swap_allocator_(this_type & other, std::true_type)
            {
                using std::swap;
                swap(allocator_(), other.allocator_());
            }

            void swap_allocator_(this_type & other, std::false_type)
            {
                YATO_MAYBE_UNUSED(other);
            }

            proxy create_proxy_(size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                return proxy(std::next(raw_ptr_(), offset * std::get<dim_descriptor::idx_total>(m_descriptors[1])), &m_descriptors[1]);
            }

            proxy create_proxy_(data_iterator plain_position) YATO_NOEXCEPT_KEYWORD
            {
                return proxy(plain_position, &m_descriptors[1]);
            }

            const_proxy create_const_proxy_(size_t offset) const YATO_NOEXCEPT_KEYWORD
            {
                return const_proxy(std::next(raw_ptr_(), offset * std::get<dim_descriptor::idx_total>(m_descriptors[1])), &m_descriptors[1]);
            }

            const_proxy create_const_proxy_(const_data_iterator plain_position) const YATO_NOEXCEPT_KEYWORD
            {
                return const_proxy(plain_position, &m_descriptors[1]);
            }

            void init_subsizes_() YATO_NOEXCEPT_KEYWORD
            {
                std::get<dim_descriptor::idx_total>(m_descriptors[dimensions_number - 1]) = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - 1]);
                for (size_t i = dimensions_number - 1; i > 0; --i) {
                    std::get<dim_descriptor::idx_total>(m_descriptors[i - 1]) = std::get<dim_descriptor::idx_size>(m_descriptors[i - 1]) * std::get<dim_descriptor::idx_total>(m_descriptors[i]);
                }
            }

            void init_sizes_(const dimensions_type & extents) YATO_NOEXCEPT_KEYWORD
            {
                m_descriptors[dimensions_number - 1] = std::make_tuple(extents[dimensions_number - 1], extents[dimensions_number - 1]);
                for (size_t i = dimensions_number - 1; i > 0; --i) {
                    m_descriptors[i - 1] = std::make_tuple(extents[i - 1], extents[i - 1] * std::get<dim_descriptor::idx_total>(m_descriptors[i]));
                }
            }

            auto dimensions_ref_range_()
                -> decltype(yato::make_range(m_descriptors).map(tuple_getter<typename dim_descriptor::type, dim_descriptor::idx_size>()))
            {
                return yato::make_range(m_descriptors).map(tuple_getter<typename dim_descriptor::type, dim_descriptor::idx_size>());
            }

            template<typename RangeType>
            void init_sizes_(RangeType range)
            {
                YATO_REQUIRES(range.distance() == dimensions_number);
                std::copy(range.begin(), range.end(), dimensions_ref_range_().begin());
                init_subsizes_();
            }

            void init_sizes_(const initilizer_type<dimensions_number> & init_list)
            {
                for(size_t i = 0; i < dimensions_number; ++i) {
                    std::get<dim_descriptor::idx_size>(m_descriptors[i]) = 0;
                }
                deduce_sizes_from_ilist_<dimensions_number>(init_list);
                init_subsizes_();
            }

            template<size_t _Depth>
            auto deduce_sizes_from_ilist_(const initilizer_type<_Depth> & init_list)
                -> typename std::enable_if < (_Depth > 1), void > ::type
            {
                size_t & dim = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - _Depth]);
                dim = std::max(dim, init_list.size());
                for(const auto & sub_list : init_list) {
                    deduce_sizes_from_ilist_<_Depth - 1>(sub_list);
                }
            }
            
            template<size_t _Depth>
            auto deduce_sizes_from_ilist_(const initilizer_type<_Depth> & init_list)
                -> typename std::enable_if<(_Depth == 1), void>::type
            {
                size_t & dim = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - _Depth]);
                dim = std::max(dim, init_list.size());
            }

            YATO_FORCED_INLINE
            size_t get_element_size_() const
            {
                return std::get<dim_descriptor::idx_total>(m_descriptors[1]);
            }

            void update_top_dimension_(size_t new_size) YATO_NOEXCEPT_KEYWORD
            {
                std::get<dim_descriptor::idx_size>(m_descriptors[0])  = new_size;
                std::get<dim_descriptor::idx_total>(m_descriptors[0]) = std::get<dim_descriptor::idx_total>(m_descriptors[1]) * new_size;
            }

            size_type get_top_dimension_() const YATO_NOEXCEPT_KEYWORD
            {
                return std::get<dim_descriptor::idx_size>(m_descriptors[0]);
            }

            /**
             * Clear pointer and sizes data, memory should be already deallocated.
             */
            void tidy_()
            {
                raw_ptr_() = nullptr;
                m_allocated_size = 0;
                update_top_dimension_(0);
            }

        public:
            /**
             *  Create empty vector
             */
            YATO_CONSTEXPR_FUNC
            vector_nd_impl()
                : m_data(yato::zero_arg_then_variadic_t{}, nullptr)
            { }

            /**
             *  Create empty vector
             */
            explicit
            vector_nd_impl(const allocator_type & alloc)
                : m_data(yato::one_arg_then_variadic_t{}, alloc, nullptr)
            { }

            /**
             *  Create without initialization
             */
            explicit
            vector_nd_impl(const dimensions_type & sizes, const allocator_type & alloc = allocator_type())
                : m_data(yato::one_arg_then_variadic_t{}, alloc, nullptr)
            {
                init_sizes_(sizes);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    raw_init_(plain_size, value_type());
                }
            }

            /**
             *  Create with initialization
             */
            vector_nd_impl(const dimensions_type & sizes, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_data(yato::one_arg_then_variadic_t{}, alloc, nullptr)
            {
                init_sizes_(sizes);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    raw_init_(plain_size, value);
                }
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes 
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const InputIt & first, const InputIt & last, const allocator_type & alloc = allocator_type())
                : m_data(yato::one_arg_then_variadic_t{}, alloc, nullptr)
            {
                init_sizes_(sizes);
                const size_t plain_size = total_size();
                if(yato::narrow_cast<std::ptrdiff_t>(plain_size) != std::distance(first, last)) {
                    throw yato::argument_error("yato::vector_nd[ctor]: Range size doesn't match dimensions.");
                }
                if(plain_size != 0) {
                    raw_init_from_range_(plain_size, first, last);
                }
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const yato::range<InputIt> & range, const allocator_type & alloc = allocator_type())
                : vector_nd_impl(sizes, range.begin(), range.end(), alloc)
            { }

            /**
             *  Create from initializer list
             */
            vector_nd_impl(const initilizer_type<dimensions_number> & init_list)
            {
                init_sizes_(init_list);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    raw_init_from_ilist_(plain_size, init_list);
                }
            }

            /**
             *  Create with sizes from a generic range of sizes without initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const allocator_type & alloc = allocator_type())
                : m_data(yato::one_arg_then_variadic_t{}, alloc, nullptr)
            {
                YATO_REQUIRES(range.distance() == dimensions_number); // "Constructor takes the amount of arguments equal to dimensions number"
                init_sizes_(range);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    raw_init_(plain_size);
                }
            }

            /**
             *  Create with sizes from a generic range of sizes with initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_data(yato::one_arg_then_variadic_t{}, alloc, nullptr)
            {
                YATO_REQUIRES(range.distance() == dimensions_number); // "Constructor takes the amount of arguments equal to dimensions number"
                init_sizes_(range);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    raw_init_(plain_size, value);
                }
            }

            /**
             *  Copy constructor
             */
            vector_nd_impl(const vector_nd_impl & other)
                : m_descriptors(other.m_descriptors)
                , m_data(yato::one_arg_then_variadic_t{}, alloc_traits::select_on_container_copy_construction(other.allocator_()), nullptr)
            {
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    raw_init_from_range_(plain_size, other.raw_ptr_(), other.raw_ptr_() + plain_size);
                }
            }

            /**
             *  Move-copy constructor
             */
            vector_nd_impl(vector_nd_impl && other) YATO_NOEXCEPT_KEYWORD
                : m_descriptors(std::move(other.m_descriptors))
                , m_allocated_size(other.m_allocated_size)
                , m_data(std::move(other.m_data))
            {
                // Steal content
                other.tidy_();
            }

            /**
             *  Move-reshape
             */
            template <size_t NewDimsNum_>
            vector_nd_impl(const dimensions_type & sizes, vector_nd_impl<data_type, NewDimsNum_, allocator_type> && other) YATO_NOEXCEPT_KEYWORD
                : m_data(yato::one_arg_then_variadic_t{}, alloc_traits::select_on_container_copy_construction(other.allocator_()), nullptr)
            {
                if(sizes.total_size() != other.total_size()) {
                    throw yato::argument_error("yato::vector_nd[move-reshape]: Total size mismatch.");
                }
                init_sizes_(sizes);
                raw_ptr_() = other.raw_ptr_();
                m_allocated_size = other.m_allocated_size;
                // Steal content
                other.tidy_();
            }

            /**
             *  Copy assign
             */
            vector_nd_impl & operator= (const vector_nd_impl & other)
            {
                YATO_REQUIRES(this != &other);
                this_type{ other }.swap(*this);
                return *this;
            }

            /**
             *  Move assign
             */
            vector_nd_impl & operator= (vector_nd_impl && other) YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(this != &other);
                
                m_descriptors  = std::move(other.m_descriptors);
                m_data = std::move(other.m_data);
                m_allocated_size = other.m_allocated_size;
                
                // Steal content
                other.tidy_();

                return *this;
            }

            /**
             *  Copy from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            explicit
            vector_nd_impl(const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                init_sizes_(other.dimensions_range());
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    raw_init_from_range_(plain_size, other.plain_cbegin(), other.plain_cend());
                }
            }

            /**
             *  Assign from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            vector_nd_impl& operator = (const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                this_type{ other }.swap(*this);
                return *this;
            }

            /**
             *  Destructor
             */
            ~vector_nd_impl()
            {
                if(raw_ptr_()) {
                    YATO_ASSERT(m_allocated_size > 0, "Invalid allocated size");
                    allocator_type & alloc = allocator_();
                    memory::destroy(alloc, raw_ptr_(), raw_ptr_() + total_size());
                    memory::deallocate(alloc, raw_ptr_(), m_allocated_size);
                }
            }

            /**
             *  Replaces the contents of the container
             *  In the case of exception the vector remains unchanged if data is copied into new storage, or becomes empty if assign was in-place.
             */
            void assign(const dimensions_type & sizes, const data_type & value)
            {
                const size_t old_size = total_size();
                const size_t new_size = sizes.total_size();
                value_type* old_data = raw_ptr_();
                allocator_type & alloc = allocator_();
                if((new_size <= m_allocated_size) && (m_allocated_size != 0)) {
                    YATO_ASSERT(old_data != nullptr, "Invalid state");
                    // destoy old content
                    memory::destroy(alloc, old_data, old_data + old_size);
                    // fill new values
                    value_type* dst = old_data;
                    try {
                        memory::fill_uninitialized(alloc, dst, old_data + new_size, value);
                    }
                    catch(...) {
                        memory::destroy(alloc, old_data, dst);
                        memory::deallocate(alloc, old_data, m_allocated_size);
                        // cant recover, make empty
                        tidy_();
                        throw;
                    }
                } else {
                    value_type* const new_data = memory::allocate(alloc, new_size);
                    value_type* dst = new_data;
                    try {
                        memory::fill_uninitialized(alloc, dst, new_data + new_size, value);
                    }
                    catch(...) {
                        memory::destroy(alloc, new_data, dst);
                        memory::deallocate(alloc, new_data, new_size);
                        throw;
                    }
                    // destoy old content
                    if(old_data != nullptr) {
                        memory::destroy(alloc, old_data, old_data + old_size);
                        memory::deallocate(alloc, old_data, m_allocated_size);
                    }
                    raw_ptr_() = new_data;
                    m_allocated_size = new_size;
                }
                // update sizes
                init_sizes_(sizes);
            }

            /**
             * Create a new vector with another shape
             * All data will be copied to the new vector
             */
            template <size_t NewDimsNum_>
            vector_nd_impl<data_type, NewDimsNum_, allocator_type> reshape(const dimensionality<NewDimsNum_, size_t> & extents) const &
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, allocator_type>(extents, plain_cbegin(), plain_cend());
            }

            /**
             * Create a new vector with another shape
             * All data will be copied to the new vector
             */
            template <size_t NewDimsNum_, typename NewAllocatorType_>
            vector_nd_impl<data_type, NewDimsNum_, NewAllocatorType_> reshape(const dimensionality<NewDimsNum_, size_t> & extents, const NewAllocatorType_ & alloc) const &
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, NewAllocatorType_>(extents, plain_cbegin(), plain_cend(), alloc);
            }

            /**
             * Create a new vector with another shape
             * All data will be moved to the new vector
             */
            template <size_t NewDimsNum_>
            vector_nd_impl<data_type, NewDimsNum_, allocator_type> reshape(const dimensionality<NewDimsNum_, size_t> & extents) &&
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, allocator_type>(extents, std::make_move_iterator(plain_cbegin()), std::make_move_iterator(plain_cend()));
            }

            /**
             * Create a new vector with another shape
             * All data will be moved to the new vector
             */
            template <size_t NewDimsNum_, typename NewAllocatorType_>
            vector_nd_impl<data_type, NewDimsNum_, NewAllocatorType_> reshape(const dimensionality<NewDimsNum_, size_t> & extents, const NewAllocatorType_ & alloc) &&
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, NewAllocatorType_>(extents, std::make_move_iterator(plain_cbegin()), std::make_move_iterator(plain_cend()), alloc);
            }

            /**
             * Reshapes vector stealing content
             */
            template <size_t NewDimsNum_>
            vector_nd_impl<data_type, NewDimsNum_, allocator_type> move_reshape(const dimensionality<NewDimsNum_, size_t> & extents) &&
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, allocator_type>(extents, std::move(*this));
            }

            /**
             *  Returns the allocator associated with the container
             */
            allocator_type get_allocator() const YATO_NOEXCEPT_KEYWORD
            {
                return allocator_();
            }

            /**
             *  Save swap
             */
            void swap(this_type & other) YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(this != &other);
                using std::swap;
                swap(m_descriptors, other.m_descriptors);
                swap(raw_ptr_(), other.raw_ptr_());
                swap(m_allocated_size, other.m_allocated_size);
                swap_allocator_(other, typename alloc_traits::propagate_on_container_swap{});
            }
            /**
             *  Element access without bounds check in release
             */
            YATO_CONSTEXPR_FUNC_EX
            const_proxy operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < size(0));
                return create_const_proxy_(idx);
            }
            /**
             *  Element access without bounds check in release
             */
            YATO_CONSTEXPR_FUNC_EX
            proxy operator[](size_t idx) YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < size(0));
                return create_proxy_(idx);
            }
            /**
             *  Element access with bounds check
             */
            template<typename... _Tail>
            auto at(size_t idx, _Tail &&... tail) const
                -> typename std::enable_if<(yato::args_length<_Tail...>::value == dimensions_number - 1), const_reference>::type
            {
                if (idx >= size(0)) {
                    throw yato::out_of_range_error("yato::array_nd: out of range!");
                }
                return (*this)[idx].at(std::forward<_Tail>(tail)...);
            }

            /**
             *  Element access with bounds check
             */
            template<typename... _Tail>
            auto at(size_t idx, _Tail &&... tail)
                -> typename std::enable_if<(yato::args_length<_Tail...>::value == dimensions_number - 1), reference>::type
            {
                if (idx >= size(0)) {
                    throw yato::out_of_range_error("yato::array_nd: out of range!");
                }
                return (*this)[idx].at(std::forward<_Tail>(tail)...);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return create_const_proxy_(static_cast<size_t>(0));
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return create_proxy_(static_cast<size_t>(0));
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return create_const_proxy_(size(0));
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return create_proxy_(size(0));
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return raw_ptr_();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return raw_ptr_();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
            {
                return raw_ptr_() + total_size();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_end() YATO_NOEXCEPT_KEYWORD
            {
                return raw_ptr_() + total_size();
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<const_data_iterator> crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(cbegin(), cend());
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<data_iterator> range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(begin(), end());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<const_data_iterator> plain_crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(plain_cbegin(), plain_cend());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<data_iterator> plain_range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(plain_begin(), plain_end());
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            data_type* data() YATO_NOEXCEPT_KEYWORD
            {
                return raw_ptr_();
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            const data_type* data() const YATO_NOEXCEPT_KEYWORD
            {
                return raw_ptr_();
            }

            /**
             *  Checks whether the vector is empty
             */
            bool empty() const YATO_NOEXCEPT_KEYWORD
            {
                return (get_top_dimension_() == 0);
            }

            /**
             *  Get dimensions
             */
            dimensions_type dimensions() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_type(dimensions_range());
            }

            /**
             *  Get number of dimensions
             */
            YATO_CONSTEXPR_FUNC_EX
            size_t dimensions_num() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_number;
            }

            /**
             *  Get number of dimensions
             */
            auto dimensions_range() const
                -> decltype(yato::make_range(m_descriptors).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>()))
            {
                return make_range(m_descriptors).map(tuple_cgetter<typename dim_descriptor::type, dim_descriptor::idx_size>());
            }
            /**
             *  Get size of specified dimension
             *  If the vector is empty ( empty() returns true ) then calling for size(idx) returns 0 for idx = 0; Return value for any idx > 0 is undefined
             */
            YATO_CONSTEXPR_FUNC_EX
            size_type size(size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < dimensions_number);
                return std::get<dim_descriptor::idx_size>(m_descriptors[idx]);
            }
            /**
             *  Get the total size of the vector (number of all elements)
             */
            YATO_CONSTEXPR_FUNC_EX
            size_t total_size() const YATO_NOEXCEPT_KEYWORD
            {
                return std::get<dim_descriptor::idx_total>(m_descriptors[0]);
            }

            /**
             *  Returns the number of elements that the container has currently allocated space for
             */
            size_t capacity() const YATO_NOEXCEPT_KEYWORD
            {
                return m_allocated_size;
            }

            /**
             *  Increase the capacity of the container to a value that's greater or equal to new_capacity
             *  In the case of exception the vector remains unchanged.
             */
            void reserve(size_t new_capacity)
            {
                if(new_capacity > m_allocated_size) {
                    allocator_type & alloc = allocator_();
                    value_type* old_data = raw_ptr_();
                    value_type* new_data = alloc_traits::allocate(alloc, new_capacity);
                    if(old_data != nullptr) {
                        const size_t old_size = total_size();
                        value_type* dst = new_data;
                        try {
                            memory::transfer_to_uninitialized(alloc, old_data, old_data + old_size, dst);
                        }
                        catch(...) {
                            memory::destroy(alloc, new_data, dst);
                            memory::deallocate(alloc, new_data, new_capacity);
                            throw;
                        }
                        memory::destroy(alloc, old_data, old_data + old_size);
                        memory::deallocate(alloc, old_data, m_allocated_size);
                    }
                    m_allocated_size = new_capacity;
                    raw_ptr_() = new_data;
                }
            }

            /**
             *  Requests the removal of unused capacity.
             *  In case of exception the vector remains unchanged.
             */
            void shrink_to_fit()
            {
                value_type* old_data = raw_ptr_();
                const size_t plain_size = total_size();
                if((old_data != nullptr) && (plain_size != m_allocated_size)) {
                    allocator_type & alloc = allocator_();
                    value_type* new_data = nullptr;
                    if(plain_size > 0) {
                        new_data = memory::allocate(alloc, plain_size);
                        value_type* dst = new_data;
                        try {
                            memory::transfer_to_uninitialized(alloc, old_data, old_data + plain_size, dst);
                        }
                        catch(...) {
                            memory::destroy(alloc, new_data, dst);
                            memory::deallocate(alloc, new_data, plain_size);
                            throw;
                        }
                    }
                    memory::destroy(alloc, old_data, old_data + plain_size);
                    memory::deallocate(alloc, old_data, m_allocated_size);
                    m_allocated_size = plain_size;
                    raw_ptr_() = new_data;
                }
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than cirrent sze then all stored data will be preserved
             *  In the case of exception the vector remains unchanged.
             *  @param length desired length in number of sub-vectors
             */
            void resize(size_t length)
            {
                const size_t old_size = total_size();
                const size_t new_size = length * get_element_size_();
                raw_resize_(old_size, new_size);
                update_top_dimension_(length);
            }

            /**
             *  Resize vector length along the top dimension.
             *  If length is bigger than cirrent sze then all stored data will be preserved.
             *  In the case of exception the vector remains unchanged.
             *  @param length desired length in number of sub-vectors.
             *  @param value if length is bigger than current size new elements will be copy initialized from 'value'.
             */
            void resize(size_t length, const data_type & value)
            {
                const size_t old_size = total_size();
                const size_t new_size = length * get_element_size_();
                raw_resize_(old_size, new_size, value);
                update_top_dimension_(length);
            }

            /**
             * Resize all vector's extents.
             * All stored data will become invalid.
             * In the case of exception the vector remains unchanged.
             * @param extents desired size of the vector
             */
            void resize(const dimensions_type & extents)
            {
                const size_t old_size = total_size();
                const size_t new_size = extents.total_size();
                raw_resize_(old_size, new_size);
                init_sizes_(extents);
            }

            /**
             * Resize all vector's extents.
             * All stored data will become invalid.
             * In the case of exception the vector remains unchanged.
             * @param extents desired size of the vector
             * @param value if the new size is bigger than the current size new elements will be copy initialized from 'value'
             */
            void resize(const dimensions_type & extents, const data_type & value)
            {
                const size_t old_size = total_size();
                const size_t new_size = extents.total_size();
                raw_resize_(old_size, new_size, value);
                init_sizes_(extents);
            }

            /**
             *  Clear vector
             */
            void clear()
            {
                resize(0);
            }

            /**
             *  Get the first sub-vector proxy
             */
            const_proxy front() const
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
#endif
                return create_const_proxy_(0);
            }

            /**
             *  Get the first sub-vector proxy
             */
            proxy front()
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
#endif
                return create_proxy_(0);
            }

            /**
             *  Get the last sub-vector proxy
             */
            const_proxy back() const
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
#endif
                return create_const_proxy_(size(0) - 1);
            }

            /**
             *  Get the last sub-vector proxy
             */
            proxy back()
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
#endif
                return create_proxy_(size(0) - 1);
            }

            /**
             *  Add sub-vector element to the back.
             *  In the case of exception the vector remains unchanged.
             */
            template<typename _OtherDataType, typename _OtherAllocator>
            void push_back(const vector_nd_impl<_OtherDataType, dimensions_number - 1, _OtherAllocator> & sub_vector)
            {
                const auto sub_dims = sub_vector.dimensions_range();
                if(!match_sub_dimensions_(sub_dims)) {
                    throw yato::argument_error("yato::vector_nd[push_back]: Subvector dimensions mismatch!");
                }
                init_dimensions_if_empty_(sub_dims);

                const size_t old_size = total_size();
                const size_t new_size = old_size + sub_vector.total_size();
                const auto insert_range = raw_prepare_push_back_(old_size, new_size);
                value_type* dst = insert_range.begin();
                try {
                    memory::copy_to_uninitialized(allocator_(), sub_vector.plain_cbegin(), sub_vector.plain_cend(), dst);
                }
                catch(...) {
                    // delete all, since sub-verctor was not inserted wholly.
                    memory::destroy(allocator_(), insert_range.begin(), dst);
                    throw;
                }
                update_top_dimension_(get_top_dimension_() + 1);
            }

            /**
             *  Add sub-vector element to the back
             */
            template<typename _OtherAllocator>
            void push_back(vector_nd_impl<data_type, dimensions_number - 1, _OtherAllocator> && sub_vector)
            {
                const auto sub_dims = sub_vector.dimensions_range();
                if(!match_sub_dimensions_(sub_dims)) {
                    throw yato::argument_error("yato::vector_nd[push_back]: Subvector dimensions mismatch!");
                }
                init_dimensions_if_empty_(sub_dims);

                const size_t old_size = total_size();
                const size_t new_size = old_size + sub_vector.total_size();
                const auto insert_range = raw_prepare_push_back_(old_size, new_size);
                value_type* dst = insert_range.begin();
                try {
                    memory::move_to_uninitialized(allocator_(), sub_vector.plain_begin(), sub_vector.plain_end(), dst);
                }
                catch(...) {
                    // delete all, since sub-verctor was not inserted wholly.
                    memory::destroy(allocator_(), insert_range.begin(), dst);
                    throw;
                }
                update_top_dimension_(get_top_dimension_() + 1);
            }

            /**
             *  Removes the last element of the container.
             */
            void pop_back()
            {
#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[pop_back]: vector is already empty!");
                }
#endif
                const size_t old_size = total_size();
                update_top_dimension_(get_top_dimension_() - 1);
                const size_t new_size = total_size();
                value_type* old_data = raw_ptr_();
                memory::destroy(allocator_(), old_data + new_size, old_data + old_size);
            }

            /**
             *  Insert sub-vector element.
             *  In the case of error only part of vector [0, position) will be kept.
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename OtherTy_, typename OtherAllocator_>
            iterator insert(const const_iterator & position, const vector_nd_impl<OtherTy_, dimensions_number - 1, OtherAllocator_> & sub_vector)
            {
                const auto sub_dims = sub_vector.dimensions_range();
                if(!match_sub_dimensions_(sub_dims)) {
                    throw yato::argument_error("yato::vector_nd[insert]: Subvector dimensions number mismatch!");
                }
                init_dimensions_if_empty_(sub_dims);

                const size_t insert_offset = raw_offset_checked_(position);
                const size_t element_size  = sub_vector.total_size();
                if(element_size == 0) {
                    throw yato::argument_error("yato::vector_nd[insert]: Insert value is empty!");
                }

                auto insert_range = raw_prepare_insert_(insert_offset, element_size, element_size);

                value_type* dst = insert_range.begin();
                try {
                    memory::copy_to_uninitialized(allocator_(), sub_vector.plain_cbegin(), sub_vector.plain_cend(), dst);
                }
                catch(...) {
                    // destroy partially filled sub-vector
                    memory::destroy(allocator_(), insert_range.begin(), dst);
                    // destroy the detached right part
                    memory::destroy(allocator_(), insert_range.end(), raw_ptr_() + total_size() + element_size);
                    // adjust size
                    update_top_dimension_(insert_offset / element_size);
                    throw;
                }
                update_top_dimension_(get_top_dimension_() + 1);
                // return position to newly inserted element
                return create_proxy_(insert_range.begin());
            }

            /**
             *  Insert sub-vector element
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            iterator insert(const const_iterator & position, vector_nd_impl<data_type, dimensions_number - 1, allocator_type> && sub_vector)
            {
                const auto sub_dims = sub_vector.dimensions_range();
                if(!match_sub_dimensions_(sub_dims)) {
                    throw yato::argument_error("yato::vector_nd[insert]: Subvector dimensions number mismatch!");
                }
                init_dimensions_if_empty_(sub_dims);

                const size_t insert_offset = raw_offset_checked_(position);
                const size_t element_size  = sub_vector.total_size();
                if(element_size == 0) {
                    throw yato::argument_error("yato::vector_nd[insert]: Insert value is empty!");
                }

                auto insert_range = raw_prepare_insert_(insert_offset, element_size, element_size);

                value_type* dst = insert_range.begin();
                try {
                    memory::move_to_uninitialized(allocator_(), sub_vector.plain_begin(), sub_vector.plain_end(), dst);
                }
                catch(...) {
                    // destroy partially filled sub-vector
                    memory::destroy(allocator_(), insert_range.begin(), dst);
                    // destroy the detached right part
                    memory::destroy(allocator_(), insert_range.end(), raw_ptr_() + total_size() + element_size);
                    // adjust size
                    update_top_dimension_(insert_offset / element_size);
                    throw;
                }
                update_top_dimension_(get_top_dimension_() + 1);
                // return position to newly inserted element
                return create_proxy_(insert_range.begin());
            }

            /**
             *  Insert count copies of sub-vector element
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename OtherTy_, typename OtherAllocator_>
            iterator insert(const const_iterator & position, size_t count, const vector_nd_impl<OtherTy_, dimensions_number - 1, OtherAllocator_> & sub_vector)
            {
                if(count > 0) {
                    const auto sub_dims = sub_vector.dimensions_range();
                    if(!match_sub_dimensions_(sub_dims)) {
                        throw yato::argument_error("yato::vector_nd[insert]: Subvector dimensions number mismatch!");
                    }
                    init_dimensions_if_empty_(sub_dims);

                    const size_t insert_offset = raw_offset_checked_(position);
                    const size_t element_size  = sub_vector.total_size();
                    if(element_size == 0) {
                        throw yato::argument_error("yato::vector_nd[insert]: Insert value is empty!");
                    }

                    auto insert_range = raw_prepare_insert_(insert_offset, count * element_size, element_size);
                    value_type* copy_first = insert_range.begin();
                    value_type* copy_last  = insert_range.begin();
                    size_t inserted_count = 0;

                    allocator_type& alloc = allocator_();
                    try {
                        while(inserted_count != count) {
                            memory::copy_to_uninitialized(alloc, sub_vector.plain_cbegin(), sub_vector.plain_cend(), copy_last);
                            copy_first = copy_last;
                            ++inserted_count;
                        }
                    }
                    catch(...) {
                        // destroy partially filled sub-vector
                        memory::destroy(alloc, copy_first, copy_last);
                        // destroy the detached right part
                        memory::destroy(alloc, insert_range.end(), raw_ptr_() + total_size() + count * element_size);
                        // adjust size
                        update_top_dimension_(insert_offset / element_size + inserted_count);
                        throw;
                    }
                    update_top_dimension_(get_top_dimension_() + count);
                    // return position to newly inserted element
                    return create_proxy_(insert_range.begin());
                } else {
                    const size_t insert_offset = raw_offset_checked_(position);
                    return create_proxy_(insert_offset);
                }
            }

            /**
             *  Inserts sub-vector elements from range [first, last) before 'position'
             *  @param position Iterator to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename IteratorNd_, typename = 
                // Check that iterator poits to something of lower dimensionality
                std::enable_if_t<std::iterator_traits<IteratorNd_>::value_type::dimensions_number == dimensions_number - 1>
            >
            iterator insert(const const_iterator & position, const IteratorNd_ & first, const IteratorNd_ & last)
            {
                const auto icount = std::distance(first, last);
                if(icount > 0) {
                    const size_t count = yato::narrow_cast<size_t>(icount);
                    const auto sub_dims = (*first).dimensions_range();
                    if(!match_sub_dimensions_(sub_dims)) {
                        throw yato::argument_error("yato::vector_nd[insert]: Subvector dimensions number mismatch!");
                    }
                    init_dimensions_if_empty_(sub_dims);

                    const size_t insert_offset = raw_offset_checked_(position);
                    const size_t element_size  = (*first).total_size();
                    if(element_size == 0) {
                        throw yato::argument_error("yato::vector_nd[insert]: Insert value is empty!");
                    }

                    auto insert_range = raw_prepare_insert_(insert_offset, count * element_size, element_size);
                    value_type* dst = insert_range.begin();
                    try {
                        memory::copy_to_uninitialized(allocator_(), (*first).plain_begin(), (*last).plain_begin(), dst);
                    }
                    catch(...) {
                        // number of whole elements
                        const size_t inserted_count = (dst - insert_range.begin()) / element_size;
                        // destroy partially filled sub-vector
                        memory::destroy(allocator_(), insert_range.begin() + inserted_count * element_size, dst);
                        // destroy the detached right part
                        memory::destroy(allocator_(), insert_range.end(), raw_ptr_() + total_size() + count * element_size);
                        // adjust size
                        update_top_dimension_(insert_offset / element_size + inserted_count);
                        throw;
                    }
                    update_top_dimension_(get_top_dimension_() + count);
                    // return position to newly inserted element
                    return create_proxy_(insert_range.begin());
                } else {
                    const size_t insert_offset = raw_offset_checked_(position);
                    return create_proxy_(insert_offset);
                }
            }

            /**
             *  Inserts sub-vector elements from range [first, last) before 'position'
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template<typename _OtherDataIterator, typename _SizeIterator>
            iterator insert(const const_iterator & position, const yato::range< proxy_tmpl<_OtherDataIterator, _SizeIterator> > & range)
            {
                return insert(position, range.begin(), range.end());
            }

            /**
             *  Removes the sub-vector element at 'position'
             */
            iterator erase(const const_iterator & position)
            {
                if(empty()) {
                    throw yato::argument_error("yato::vector_nd[erase]: Vector is empty!");
                }
                return create_proxy_(raw_erase_(position, 1, get_element_size_()));
            }

            /**
             *  Removes the sub-vector elements in the range [first, last)
             */
            iterator erase(const const_iterator & first, const const_iterator & last)
            {
                const std::ptrdiff_t count = std::distance(first, last);
                if(count < 0) {
                    throw yato::argument_error("yato::vector_nd[erase]: Invalid range iterators!");
                }
                if(count != 0) {
                    const size_t ucount = yato::narrow_cast<size_t>(count);
                    if(ucount > size(0)) {
                        throw yato::argument_error("yato::vector_nd[erase]: Invalid range iterators!");
                    }
                    return create_proxy_(raw_erase_(first, ucount, get_element_size_()));
                } else {
                    const size_t insert_offset = raw_offset_checked_(first);
                    return create_proxy_(insert_offset);
                }
            }

            //------------------------------------------------------------

            template <typename, size_t, typename>
            friend class vector_nd_impl;
        };







        //-------------------------------------------------------
        // Implementation of 1D case
        // Delegates most methods to std::vector

        template <typename _DataType, typename _Allocator>
        class vector_nd_impl<_DataType, 1, _Allocator>
        {
        public:
            /*
            * Public traits of the multidimensional vector
            */
            using my_type = vector_nd_impl<_DataType, 1, _Allocator>;
            using dimensions_type = dimensionality<1, size_t>;
            using data_type = _DataType;
            using allocator_type = _Allocator;
            using container_type = std::vector<data_type, allocator_type>;
            using data_iterator = typename container_type::iterator;
            using const_data_iterator = typename container_type::const_iterator;
            using reference = decltype(*std::declval<data_iterator>());
            using const_reference = decltype(*std::declval<const_data_iterator>());

            static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;

            using iterator = data_iterator;
            using const_iterator = const_data_iterator;
            //-------------------------------------------------------

        private:
            container_type m_plain_vector;
            //-------------------------------------------------------

        public:
            /**
             *  Create empty vector
             */
            vector_nd_impl() YATO_NOEXCEPT_KEYWORD
                : m_plain_vector()
            {}

            /**
             *  Create empty vector
             */
            explicit
            vector_nd_impl(const allocator_type & alloc) YATO_NOEXCEPT_KEYWORD
                : m_plain_vector(alloc)
            {}

            /**
             *  Create without initialization
             */
            explicit
            vector_nd_impl(const dimensions_type & size, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                m_plain_vector.resize(size[0]);
            }

            /**
             *  Create with initialization
             */
            vector_nd_impl(const dimensions_type & size, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                assign(size, value);
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const InputIt & first, const InputIt & last, const allocator_type & alloc = allocator_type())
                : m_plain_vector(first, last, alloc)
            {
                YATO_MAYBE_UNUSED(sizes);
                YATO_REQUIRES(sizes.total_size() == narrow_cast<size_t>(std::distance(first, last)));
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const yato::range<InputIt> & range, const allocator_type & alloc = allocator_type())
                : vector_nd_impl(sizes, range.begin(), range.end(), alloc)
            { }

            /**
             *  Create from initializer list
             */
            vector_nd_impl(const std::initializer_list<data_type> & init_list)
                : m_plain_vector(init_list)
            { }

            /**
             *  Create with sizes from a generic range of sizes without initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (range.distance() != dimensions_number) {
                    throw yato::out_of_range_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                m_plain_vector.resize(*range.begin());
            }

            /**
             *  Create with sizes from a generic range of sizes with initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_plain_vector(alloc)
            {
                if (range.distance() != dimensions_number) {
                    throw yato::out_of_range_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                m_plain_vector.resize(*range.begin(), value);
            }

            /**
             *  Create from std::vector
             */
            template<typename _VecDataType, typename _VecAllocator>
            vector_nd_impl(const std::vector<_VecDataType, _VecAllocator> & vector)
                : m_plain_vector(vector)
            { }

            /**
             *  Create from std::vector
             */
            vector_nd_impl(std::vector<data_type, allocator_type> && vector)
                : m_plain_vector(std::move(vector))
            { }

            /**
             *  Copy constructor
             */
            vector_nd_impl(const my_type & other)
                : m_plain_vector(other.m_plain_vector)
            {}

            /**
             * Move-copy constructor
             */
            vector_nd_impl(my_type && other)
                : m_plain_vector(std::move(other.m_plain_vector))
            {}

            /**
             *  Copy assign
             */
            my_type & operator= (const my_type & other)
            {
                if (this != &other) {
                    my_type tmp{ other };
                    tmp.swap(*this);
                }
                return *this;
            }

            /**
             *  Move assign
             */
            my_type & operator= (my_type && other) YATO_NOEXCEPT_KEYWORD
            {
                if (this != &other) {
                    m_plain_vector = std::move(other.m_plain_vector);
                }
                return *this;
            }

            /**
             *  Assign from std::vector
             */
            template<typename _VecDataType, typename _VecAllocator>
            my_type & operator= (const std::vector<_VecDataType, _VecAllocator> & vector)
            {
                m_plain_vector = vector;
                return *this;
            }

            /**
             *  Assign from std::vector
             */
            my_type & operator= (std::vector<data_type, allocator_type> && vector)
            {
                m_plain_vector = std::move(vector);
                return *this;
            }

            /**
             *  Copy from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            explicit
            vector_nd_impl(const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                m_plain_vector.resize(other.total_size());
                std::copy(other.plain_cbegin(), other.plain_cend(), plain_begin());
            }

            /**
             *  Assign from proxy
             */
            template<typename _DataIterator, typename _SizeIterator>
            my_type & operator= (const details::sub_array_proxy<_DataIterator, _SizeIterator, dimensions_number> & other)
            {
                my_type tmp{ other };
                tmp.swap(*this);
                return *this;
            }

            /**
             *  Destructor
             */
            ~vector_nd_impl()
            {}

#ifdef YATO_MSVC_2013
            /**
             *  Convert to std::vector
             */
            operator std::vector<data_type, allocator_type> & ()
            {
                return m_plain_vector;
            }
#else
            /**
             *  Convert to std::vector
             */
            operator std::vector<data_type, allocator_type> & () &
            {
                return m_plain_vector;
            }

            /**
             *  Convert to std::vector
             */
            operator std::vector<data_type, allocator_type> && () &&
            {
                return std::move(m_plain_vector);
            }
#endif
            /**
             *  Replaces the contents of the container
             */
            void assign(const dimensions_type & size, const data_type & value)
            {
                m_plain_vector.assign(size[0], value);
            }

            /**
             * Create a new vector with another shape
             * All data will be copied to the new vector
             */
            template <size_t NewDimsNum_>
            vector_nd_impl<data_type, NewDimsNum_, allocator_type> reshape(const dimensionality<NewDimsNum_, size_t> & extents) const &
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, allocator_type>(extents, plain_cbegin(), plain_cend());
            }

            /**
             * Create a new vector with another shape
             * All data will be copied to the new vector
             */
            template <size_t NewDimsNum_, typename NewAllocatorType_>
            vector_nd_impl<data_type, NewDimsNum_, NewAllocatorType_> reshape(const dimensionality<NewDimsNum_, size_t> & extents, const NewAllocatorType_ & alloc) const &
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, NewAllocatorType_>(extents, plain_cbegin(), plain_cend(), alloc);
            }

            /**
             * Create a new vector with another shape
             * All data will be copied to the new vector
             */
            template <size_t NewDimsNum_>
            vector_nd_impl<data_type, NewDimsNum_, allocator_type> reshape(const dimensionality<NewDimsNum_, size_t> & extents) &&
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, allocator_type>(extents, std::make_move_iterator(plain_cbegin()), std::make_move_iterator(plain_cend()));
            }

            /**
             * Create a new vector with another shape
             * All data will be copied to the new vector
             */
            template <size_t NewDimsNum_, typename NewAllocatorType_>
            vector_nd_impl<data_type, NewDimsNum_, NewAllocatorType_> reshape(const dimensionality<NewDimsNum_, size_t> & extents, const NewAllocatorType_ & alloc) &&
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, NewAllocatorType_>(extents, std::make_move_iterator(plain_cbegin()), std::make_move_iterator(plain_cend()), alloc);
            }

            /**
             * Reshapes vector stealing content
             */
            template <size_t NewDimsNum_>
            vector_nd_impl<data_type, NewDimsNum_, allocator_type> move_reshape(const dimensionality<NewDimsNum_, size_t> & extents) &&
            {
                if(extents.total_size() != total_size()) {
                    yato::argument_error("yato::vector_nd[reshape]: Total size mismatch.");
                }
                return vector_nd_impl<data_type, NewDimsNum_, allocator_type>(extents, std::move(*this));
            }

            /**
             *  Returns the allocator associated with the container
             */
            allocator_type get_allocator() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.get_allocator();
            }

            /**
             *  Save swap
             */
            void swap(my_type & other) YATO_NOEXCEPT_KEYWORD
            {
                m_plain_vector.swap(other.m_plain_vector);
            }
            /**
             *  Element access without bounds check in release
             */
            YATO_CONSTEXPR_FUNC_EX
            const_reference operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < m_plain_vector.size());
                return m_plain_vector[idx];
            }
            /**
             *  Element access without bounds check in release
             */
            YATO_CONSTEXPR_FUNC_EX
            reference operator[](size_t idx) YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < m_plain_vector.size());
                return m_plain_vector[idx];
            }
            /**
             *  Element access with bounds check
             */
            const_reference at(size_t idx) const
            {
                return m_plain_vector.at(idx);
            }

            /**
             *  Element access with bounds check
             */
            reference at(size_t idx)
            {
                return m_plain_vector.at(idx);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return plain_cbegin();
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return plain_begin();
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return plain_cend();
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return plain_end();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cbegin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.begin();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.cend();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_end() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.end();
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<const_data_iterator> crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(cbegin(), cend());
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<data_iterator> range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(begin(), end());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<const_data_iterator> plain_crange() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(plain_cbegin(), plain_cend());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<data_iterator> plain_range() YATO_NOEXCEPT_KEYWORD
            {
                return yato::make_range(plain_begin(), plain_end());
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            data_type* data() YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.data();
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            const data_type* data() const YATO_NOEXCEPT_KEYWORD
            {
                return const_cast<my_type*>(this)->data();
            }

            /**
             *  Checks whether the vector is empty
             */
            bool empty() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.empty();
            }

            /**
             *  Get dimensions
             */
            dimensions_type dimensions() const YATO_NOEXCEPT_KEYWORD
            {
                return yato::dims(m_plain_vector.size());
            }

            /**
             *  Get number of dimensions
             */
            size_t dimensions_num() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_number;
            }

            /**
             *  Get number of dimensions
             */
            auto dimensions_range() const
                -> yato::range<yato::numeric_iterator<size_t>>
            {
                return yato::make_range(m_plain_vector.size(), m_plain_vector.size() + 1);
            }

            /**
             *  Get size of specified dimension
             */
            size_t size(size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < dimensions_number);
                YATO_MAYBE_UNUSED(idx);
                return m_plain_vector.size();
            }

            /**
             *  Get the total size of the vector (number of all elements)
             */
            size_t total_size() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.size();
            }

            /**
             *  Returns the number of elements that the container has currently allocated space for
             */
            size_t capacity() const YATO_NOEXCEPT_KEYWORD
            {
                return m_plain_vector.capacity();
            }

            /**
             *  Increase the capacity of the container to a value that's greater or equal to new_capacity
             */
            void reserve(size_t new_capacity)
            {
                m_plain_vector.reserve(new_capacity);
            }

            /**
             *  Clear vector
             */
            void clear()
            {
                m_plain_vector.clear();
            }

            /**
             *  Requests the removal of unused capacity
             */
            void shrink_to_fit()
            {
                m_plain_vector.shrink_to_fit();
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than the current size, then all containing data will be preserved
             */
            void resize(size_t length)
            {
                m_plain_vector.resize(length);
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than the current size, then all containing data will be preserved
             */
            void resize(size_t length, const data_type & value)
            {
                m_plain_vector.resize(length, value);
            }

            /**
             *  Resize vector extents. 
             *  All stored data becomes invalid
             */
            void resize(const dimensions_type & extents)
            {
                m_plain_vector.resize(extents[0]);
            }

            /**
             *  Resize vector extents.
             *  All stored data becomes invalid
             */
            void resize(const dimensions_type & extents, const data_type & value)
            {
                m_plain_vector.resize(extents[0], value);
            }

            /**
             *  Get the first sub-vector proxy
             */
            const_reference front() const
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
                return m_plain_vector.front();
            }

            /**
             *  Get the first sub-vector proxy
             */
            reference front()
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
                return m_plain_vector.front();
            }

            /**
             *  Get the last sub-vector proxy
             */
            const_reference back() const
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[back]: vector is empty");
                }
                return m_plain_vector.back();
            }

            /**
             *  Get the last sub-vector proxy
             */
            reference back()
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[back]: vector is empty");
                }
                return m_plain_vector.back();
            }

            /**
             *  Add sub-vector element to the back
             */
            void push_back(const data_type & value)
            {
                m_plain_vector.push_back(value);
            }

            /**
             *  Add sub-vector element to the back
             */
            void push_back(data_type && value)
            {
                m_plain_vector.push_back(std::move(value));
            }

            /**
             *  Removes the last element of the container.
             */
            void pop_back()
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[pop_back]: vector is already empty!");
                }
                m_plain_vector.pop_back();
            }

            /**
             *  Inserts elements at the specified location before 'position'
             */
            iterator insert(const const_iterator & position, const data_type & value)
            {
                return m_plain_vector.insert(position, value);
            }

            /**
             *  Inserts elements at the specified location before 'position'
             */
            iterator insert(const const_iterator & position, data_type && value)
            {
                return m_plain_vector.insert(position, std::move(value));
            }

            /**
             *  Inserts elements at the specified location before 'position'
             */
            iterator insert(const const_iterator & position, size_t count, const data_type & value)
            {
                return m_plain_vector.insert(position, count, value);
            }

            /**
             *  Inserts elements from range [first, last) before 'position'
             */
            template<class _InputIt>
            iterator insert(const const_iterator & position, _InputIt && first, _InputIt && last)
            {
                return m_plain_vector.insert(position, std::forward<_InputIt>(first), std::forward<_InputIt>(last));
            }

            /**
             *  Inserts elements from range before 'position'
             */
            template<class _InputIt>
            iterator insert(const const_iterator & position, const yato::range<_InputIt> & range)
            {
                return m_plain_vector.insert(position, range.begin(), range.end());
            }

            /**
             *  Removes the element at 'position'
             */
            iterator erase(const const_iterator & position)
            {
                return m_plain_vector.erase(position);
            }

            /**
             *  Removes the elements in the range [first; last)
             */
            iterator erase(const const_iterator & first, const const_iterator & last)
            {
                return m_plain_vector.erase(first, last);
            }

            /**
             *  Removes the elements in the range 
             */
            template<class _InputIt>
            iterator erase(const yato::range<_InputIt> & range)
            {
                return m_plain_vector.erase(range.begin(), range.last());
            }

            //------------------------------------------------------------

            template <typename, size_t, typename>
            friend class vector_nd_impl;
        };

    }

    template <typename _DataType, size_t _DimensionsNum, typename _Allocator = std::allocator<_DataType> >
    using vector_nd = details::vector_nd_impl<_DataType, _DimensionsNum, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector_1d = vector_nd<_DataType, 1, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector = vector_1d<_DataType, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector_2d = vector_nd<_DataType, 2, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector_3d = vector_nd<_DataType, 3, _Allocator>;

    template <typename _DataType, typename _Allocator = std::allocator<_DataType> >
    using vector_4d = vector_nd<_DataType, 4, _Allocator>;
}

#endif
