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

        template<typename Ty_, size_t Dims_> 
        using initializer_list_nd_t = typename initilizer_list_nd<Ty_, Dims_>::type;

        //-------------------------------------------------------
        //

        template <typename ValueTy_, typename Allocator_>
        class raw_vector
        {
            using this_type = raw_vector<ValueTy_, Allocator_>;
            using value_type = ValueTy_;
            using raw_pointer = ValueTy_*;
            using allocator_type = Allocator_;
            //-----------------------------------------------------

            yato::compressed_pair<Allocator_, ValueTy_*> m_storage;
            size_t m_allocated_size = 0;
            //-----------------------------------------------------

            void swap_allocator_(this_type & other, std::true_type)
            {
                using std::swap;
                swap(allocator(), other.allocator());
            }

            void swap_allocator_(this_type & other, std::false_type)
            {
                YATO_MAYBE_UNUSED(other);
            }

        public:
            constexpr
            raw_vector()
                : m_storage(yato::zero_arg_then_variadic_t{}, nullptr)
            { }

            constexpr
            raw_vector(allocator_type alloc)
                : m_storage(yato::one_arg_then_variadic_t{}, std::move(alloc), nullptr)
            { }

            raw_vector(const raw_vector&) = delete;

            raw_vector(raw_vector && other) noexcept
                : m_storage(yato::one_arg_then_variadic_t{}, std::move(other.allocator()), other.ptr())
                , m_allocated_size(other.m_allocated_size)
            {
                other.ptr() = nullptr;
                other.m_allocated_size = 0;
            }

            raw_vector& operator=(const raw_vector&) = delete;

            raw_vector& operator=(raw_vector && other) noexcept
            {
                YATO_REQUIRES(this != &other);
                
                if(ptr()) {
                    memory::deallocate(allocator(), ptr(), m_allocated_size);
                }

                allocator() = std::move(other.allocator());
                ptr() = other.ptr();
                m_allocated_size = other.m_allocated_size;

                other.ptr() = nullptr;
                other.m_allocated_size = 0;

                return *this;
            }

            /**
             * Only deallocate memory
             */
            ~raw_vector()
            {
                if(ptr()) {
                    memory::deallocate(allocator(), ptr(), m_allocated_size);
                }
            }

            void swap(this_type & other) noexcept
            {
                using std::swap;
                swap(ptr(), other.ptr());
                swap(m_allocated_size, other.m_allocated_size);
                swap_allocator_(other, typename std::allocator_traits<allocator_type>::propagate_on_container_swap{});
            }

            YATO_FORCED_INLINE
            raw_pointer & ptr()
            {
                return m_storage.second();
            }

            YATO_FORCED_INLINE
            const raw_pointer & ptr() const
            {
                return m_storage.second();
            }

            YATO_FORCED_INLINE
            allocator_type & allocator()
            {
                return m_storage.first();
            }

            YATO_FORCED_INLINE
            const allocator_type & allocator() const
            {
                return m_storage.first();
            }

            size_t capacity() const
            {
                return m_allocated_size;
            }

            void init_manually(value_type* pointer, size_t allocated_size)
            {
                YATO_REQUIRES(ptr() == nullptr);
                ptr() = pointer;
                m_allocated_size = allocated_size;
            }

            /**
             * Allocater and value-initialize.
             * Current state should be empty
             */
            template <typename ... InitArgs_>
            void init_from_value(size_t plain_size, InitArgs_ && ... init_args)
            {
                YATO_REQUIRES(ptr() == nullptr);
                YATO_REQUIRES(plain_size != 0);

                value_type* const new_data = memory::allocate(allocator(), plain_size);
                value_type* dst = new_data;
                try {
                    memory::fill_uninitialized(allocator(), dst, new_data + plain_size, std::forward<InitArgs_>(init_args)...);
                }
                catch(...) {
                    memory::destroy(allocator(), new_data, dst);
                    memory::deallocate(allocator(), new_data, plain_size);
                    throw;
                }
                ptr() = new_data;
                m_allocated_size = plain_size;
            }

            /**
             * Allocater and copy-initialize.
             * Current state should be empty
             */
            template <typename IterTy_>
            void init_from_range(size_t plain_size, IterTy_ src_first, IterTy_ src_last)
            {
                YATO_REQUIRES(ptr() == nullptr);
                YATO_REQUIRES(plain_size != 0);
                YATO_REQUIRES(yato::narrow_cast<std::ptrdiff_t>(plain_size) == std::distance(src_first, src_last));

                value_type* new_data = memory::allocate(allocator(), plain_size);
                value_type* dst = new_data;
                try {
                    memory::copy_to_uninitialized(allocator(), src_first, src_last, dst);
                }
                catch(...) {
                    memory::destroy(allocator(), new_data, dst);
                    memory::deallocate(allocator(), new_data, plain_size);
                    throw;
                }
                ptr() = new_data;
                m_allocated_size = plain_size;
            }

            /**
             *  Replaces the contents of the container
             *  In the case of exception the vector remains unchanged if data is copied into new storage, or becomes empty if assign was in-place.
             */
            template <typename ... InitArgs_>
            void assign(size_t old_size, size_t new_size, bool & valid, InitArgs_ && ... init_args)
            {
                value_type* old_data = ptr();
                allocator_type & alloc = allocator();
                if((new_size <= m_allocated_size) && (m_allocated_size != 0)) {
                    YATO_ASSERT(old_data != nullptr, "Invalid state");
                    // destoy old content
                    memory::destroy(alloc, old_data, old_data + old_size);
                    // fill new values
                    value_type* dst = old_data;
                    try {
                        memory::fill_uninitialized(alloc, dst, old_data + new_size, std::forward<InitArgs_>(init_args)...);
                    }
                    catch(...) {
                        memory::destroy(alloc, old_data, dst);
                        //memory::deallocate(alloc, old_data, m_allocated_size);
                        // cant recover, make empty
                        valid = false;
                        throw;
                    }
                } else {
                    value_type* const new_data = memory::allocate(alloc, new_size);
                    value_type* dst = new_data;
                    try {
                        memory::fill_uninitialized(alloc, dst, new_data + new_size, std::forward<InitArgs_>(init_args)...);
                    }
                    catch(...) {
                        memory::destroy(alloc, new_data, dst);
                        memory::deallocate(alloc, new_data, new_size);
                        valid = true;
                        throw;
                    }
                    // destoy old content
                    if(old_data != nullptr) {
                        memory::destroy(alloc, old_data, old_data + old_size);
                        memory::deallocate(alloc, old_data, m_allocated_size);
                    }
                    ptr() = new_data;
                    m_allocated_size = new_size;
                }
            }

            template <typename ... Args_>
            void resize(size_t old_size, size_t new_size, Args_ && ... init_args)
            {
                if(new_size < old_size) {
                    value_type* const old_data = ptr();
                    memory::destroy(allocator(), old_data + new_size, old_data + old_size);
                }
                else if(new_size > old_size) {
                    allocator_type& alloc = allocator();
                    if(new_size > m_allocated_size) {
                        value_type* const old_data = ptr();
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
                        ptr() = new_data;
                        m_allocated_size = new_size;
                    } else {
                        // enough space
                        value_type* const old_data = ptr();
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

            yato::range<value_type*> prepare_push_back(size_t old_size, size_t new_size)
            {
                if(new_size > m_allocated_size) {
                    allocator_type& alloc = allocator();
                    value_type* new_data = memory::allocate(alloc, new_size);
                    value_type* old_data = ptr();
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
                    ptr() = new_data;
                    m_allocated_size = new_size;
                }
                return yato::make_range(std::next(ptr(), old_size), std::next(ptr(), new_size));
            }

            /**
             * Set valid_count to number of valid elements after exception handling.
             */
            yato::range<value_type*> prepare_insert(size_t old_size, size_t insert_offset, size_t count, size_t element_size, size_t & valid_count)
            {
                const size_t insert_size = count * element_size;
                const size_t new_size = old_size + insert_size;
                YATO_ASSERT(new_size > old_size, "Invalid size");
                if(new_size > m_allocated_size) {
                    allocator_type& alloc = allocator();
                    value_type* new_data = memory::allocate(alloc, new_size);
                    value_type* old_data = ptr();
                    if(old_data != nullptr) {
                        // transfer left part
                        value_type* dst_left = new_data;
                        try {
                            memory::transfer_to_uninitialized(alloc, old_data, old_data + insert_offset, dst_left);
                        }
                        catch(...) {
                            valid_count = old_size / element_size;
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
                            valid_count = old_size / element_size;
                            memory::destroy(alloc, new_data, dst_left);
                            memory::destroy(alloc, new_data + insert_offset + insert_size, dst_right);
                            memory::deallocate(alloc, new_data, new_size);
                            throw;
                        }
                        // destroy old
                        memory::destroy(alloc, old_data, old_data + old_size);
                        memory::deallocate(alloc, old_data, m_allocated_size);
                    }
                    ptr() = new_data;
                    m_allocated_size = new_size;
                } else {
                    allocator_type& alloc = allocator();
                    // enough of allocated size
                    value_type* old_data = ptr();
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
                            valid_count = old_size / element_size;
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
                            valid_count = old_size / element_size;
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
                            valid_count = (dst - old_data) / element_size;
                            memory::destroy(alloc, old_data + valid_count * element_size, dst);
                            memory::destroy(alloc, dst + 1, dst_tail);
                            throw;
                        }
                        memory::destroy(alloc, old_data + insert_offset, old_data + insert_size);
                    }
                }
                return yato::make_range(std::next(ptr(), insert_offset), std::next(ptr(), insert_offset + insert_size));
            }

            /**
             * Set valid_count to number of valid elements after exception handling.
             */
            value_type* erase(size_t old_size, size_t erase_offset, size_t count, size_t element_size, size_t & valid_count)
            {
                YATO_REQUIRES(count != 0);

                const size_t erase_size = yato::narrow_cast<size_t>(count * element_size);

                YATO_ASSERT(old_size >= erase_size, "Invalid offsets");
                const size_t new_size = old_size - erase_size;

                allocator_type& alloc = allocator();
                value_type* old_data = ptr();

                // transfer right part
                value_type* dst = old_data + erase_offset;
                try {
                    memory::transfer_to_left(alloc, old_data + erase_offset + erase_size, old_data + old_size, dst);
                }
                catch(...) {
                    // the only uninitialized element is at dst position
                    valid_count = (dst - old_data) / element_size;
                    memory::destroy(alloc, old_data + valid_count * element_size, dst);
                    // dst + 1 is always valid because erase_size > 0
                    memory::destroy(alloc, dst + 1, old_data + old_size);
                    throw;
                }
                // destroy tail
                memory::destroy(alloc, old_data + new_size, old_data + old_size);
                // result position
                return old_data + erase_offset;
            }

            /**
             *  Increase the capacity of the container to a value that's greater or equal to new_capacity
             *  In the case of exception the vector remains unchanged.
             */
            void reserve(size_t old_size, size_t new_capacity)
            {
                if(new_capacity > m_allocated_size) {
                    allocator_type & alloc = allocator();
                    value_type* old_data = ptr();
                    value_type* new_data = memory::allocate(alloc, new_capacity);
                    if(old_data != nullptr) {
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
                    ptr() = new_data;
                }
            }

            /**
             *  Requests the removal of unused capacity.
             *  In case of exception the vector remains unchanged.
             */
            void shrink_to_fit(size_t plain_size)
            {
                value_type* old_data = ptr();
                if((old_data != nullptr) && (plain_size != m_allocated_size)) {
                    allocator_type & alloc = allocator();
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
                    ptr() = new_data;
                }
            }
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
            raw_vector<value_type, allocator_type> m_raw_vector;
            //-------------------------------------------------------

            template <typename InitTy_>
            void raw_init_from_ilist_(size_t plain_size, const initializer_list_nd_t<InitTy_, dimensions_number> & init_list)
            {
                YATO_REQUIRES(plain_size != 0);

                allocator_type& alloc = m_raw_vector.allocator();
                value_type* new_data = memory::allocate(alloc, plain_size);
                value_type* dst = new_data; // copy, that can be changed
                try {
                    // Recursively copy values from the initializser list
                    raw_copy_initializer_list_<InitTy_, dimensions_number>(init_list, dst);
                }
                catch(...) {
                    memory::destroy(alloc, new_data, dst);
                    memory::deallocate(alloc, new_data, plain_size);
                    throw;
                }
                m_raw_vector.init_manually(new_data, plain_size);
            }

            YATO_FORCED_INLINE
            size_t raw_offset_checked_(const const_iterator & position) const
            {
                const std::ptrdiff_t offset = std::distance<const_data_iterator>(m_raw_vector.ptr(), position.plain_cbegin());
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

            yato::range<value_type*> prepare_insert_(size_t insert_offset, size_t count, size_t element_size)
            {
                size_t valid_count_after_error = 0;
                try {
                    return m_raw_vector.prepare_insert(total_size(), insert_offset, count, element_size, valid_count_after_error);
                }
                catch(...) {
                    // adjust size
                    update_top_dimension_(valid_count_after_error);
                    throw;
                }
            }

            value_type* erase_(const const_iterator & position, size_t count, size_t element_size)
            {
                const size_t erase_offset = raw_offset_checked_(position);
                size_t valid_count_after_error = 0;
                value_type* pos = nullptr;
                try {
                    pos = m_raw_vector.erase(total_size(), erase_offset, count, element_size, valid_count_after_error);
                }
                catch(...) {
                    update_top_dimension_(valid_count_after_error);
                    throw;
                }
                update_top_dimension_(get_top_dimension_() - count);
                return pos;
            }

            template<typename InitTy_, size_t Dims_, typename Iter_>
            auto raw_copy_initializer_list_(const initializer_list_nd_t<InitTy_, Dims_> & init_list, Iter_ & dst)
                -> std::enable_if_t<(Dims_ > 1)>
            {
                const size_t dim_size = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - Dims_]);
                if(dim_size != init_list.size()) {
                    throw yato::argument_error("yato::vector_nd[ctor]: Invalid form of the initializer list.");
                }
                for (const auto & init_sub_list : init_list) {
                    raw_copy_initializer_list_<InitTy_, Dims_ - 1>(init_sub_list, dst);
                }
            }

            template<typename InitTy_, size_t Dims_, typename Iter_>
            auto raw_copy_initializer_list_(const initializer_list_nd_t<InitTy_, Dims_> & init_list, Iter_ & dst)
                -> std::enable_if_t<Dims_ == 1>
            {
                const size_t dim_size = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - Dims_]);
                YATO_ASSERT(dim_size >= init_list.size(), "Size was deduced incorrectly!");
                allocator_type & alloc = m_raw_vector.allocator();
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

            proxy create_proxy_(size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                return proxy(std::next(m_raw_vector.ptr(), offset * std::get<dim_descriptor::idx_total>(m_descriptors[1])), &m_descriptors[1]);
            }

            proxy create_proxy_(data_iterator plain_position) YATO_NOEXCEPT_KEYWORD
            {
                return proxy(plain_position, &m_descriptors[1]);
            }

            const_proxy create_const_proxy_(size_t offset) const YATO_NOEXCEPT_KEYWORD
            {
                return const_proxy(std::next(m_raw_vector.ptr(), offset * std::get<dim_descriptor::idx_total>(m_descriptors[1])), &m_descriptors[1]);
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

            template <typename InitTy_>
            void init_sizes_from_ilist_(const initializer_list_nd_t<InitTy_, dimensions_number> & init_list)
            {
                for(size_t i = 0; i < dimensions_number; ++i) {
                    std::get<dim_descriptor::idx_size>(m_descriptors[i]) = 0;
                }
                deduce_sizes_from_ilist_<InitTy_, dimensions_number>(init_list);
                init_subsizes_();
            }

            template<typename InitTy_, size_t Dims_>
            auto deduce_sizes_from_ilist_(const initializer_list_nd_t<InitTy_, Dims_> & init_list)
                -> std::enable_if_t<(Dims_ > 1)>
            {
                size_t & dim = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - Dims_]);
                dim = std::max(dim, init_list.size());
                for(const auto & sub_list : init_list) {
                    deduce_sizes_from_ilist_<InitTy_, Dims_ - 1>(sub_list);
                }
            }
            
            template<typename InitTy_, size_t Dims_>
            auto deduce_sizes_from_ilist_(const initializer_list_nd_t<InitTy_, Dims_> & init_list)
                -> std::enable_if_t<Dims_ == 1>
            {
                size_t & dim = std::get<dim_descriptor::idx_size>(m_descriptors[dimensions_number - Dims_]);
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
             * Clear after moved away
             */
            void tidy_()
            {
                YATO_REQUIRES(m_raw_vector.ptr() == nullptr);
                update_top_dimension_(0);
            }

        public:
            /**
             *  Create empty vector
             */
            YATO_CONSTEXPR_FUNC
            vector_nd_impl() = default;

            /**
             *  Create empty vector
             */
            explicit
            vector_nd_impl(const allocator_type & alloc)
                : m_raw_vector(alloc)
            { }

            /**
             *  Create without initialization
             */
            explicit
            vector_nd_impl(const dimensions_type & sizes, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc)
            {
                init_sizes_(sizes);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    m_raw_vector.init_from_value(plain_size);
                }
            }

            /**
             *  Create with initialization
             */
            vector_nd_impl(const dimensions_type & sizes, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc)
            {
                init_sizes_(sizes);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    m_raw_vector.init_from_value(plain_size, value);
                }
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes 
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const InputIt & first, const InputIt & last, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc)
            {
                init_sizes_(sizes);
                const size_t plain_size = total_size();
                if(yato::narrow_cast<std::ptrdiff_t>(plain_size) != std::distance(first, last)) {
                    throw yato::argument_error("yato::vector_nd[ctor]: Range size doesn't match dimensions.");
                }
                if(plain_size != 0) {
                    m_raw_vector.init_from_range(plain_size, first, last);
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
            vector_nd_impl(const initializer_list_nd_t<value_type, dimensions_number> & init_list)
                : m_raw_vector()
            {
                init_sizes_from_ilist_<value_type>(init_list);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    raw_init_from_ilist_<value_type>(plain_size, init_list);
                }
            }

            /**
             *  Create with sizes from a generic range of sizes without initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc)
            {
                YATO_REQUIRES(range.distance() == dimensions_number); // "Constructor takes the amount of arguments equal to dimensions number"
                init_sizes_(range);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    m_raw_vector.init_from_value(plain_size);
                }
            }

            /**
             *  Create with sizes from a generic range of sizes with initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc)
            {
                YATO_REQUIRES(range.distance() == dimensions_number); // "Constructor takes the amount of arguments equal to dimensions number"
                init_sizes_(range);
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    m_raw_vector.init_from_value(plain_size, value);
                }
            }

            /**
             *  Copy constructor
             */
            vector_nd_impl(const vector_nd_impl & other)
                : m_descriptors(other.m_descriptors)
                , m_raw_vector(alloc_traits::select_on_container_copy_construction(other.m_raw_vector.allocator()))
            {
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    m_raw_vector.init_from_range(plain_size, other.m_raw_vector.ptr(), other.m_raw_vector.ptr() + plain_size);
                }
            }

            /**
             *  Move-copy constructor
             */
            vector_nd_impl(vector_nd_impl && other) YATO_NOEXCEPT_KEYWORD
                : m_descriptors(std::move(other.m_descriptors))
                , m_raw_vector(std::move(other.m_raw_vector))
            {
                // Steal content
                other.tidy_();
            }

            /**
             *  Move-reshape
             */
            template <size_t NewDimsNum_>
            vector_nd_impl(const dimensions_type & sizes, vector_nd_impl<data_type, NewDimsNum_, allocator_type> && other) YATO_NOEXCEPT_KEYWORD
                : m_raw_vector()
            {
                if(sizes.total_size() != other.total_size()) {
                    throw yato::argument_error("yato::vector_nd[move-reshape]: Total size mismatch.");
                }
                init_sizes_(sizes);
                m_raw_vector = std::move(other.m_raw_vector);
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
                m_raw_vector = std::move(other.m_raw_vector);

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
                    m_raw_vector.init_from_range(plain_size, other.plain_cbegin(), other.plain_cend());
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
                const size_t size = total_size();
                if(size != 0) {
                    value_type* ptr = m_raw_vector.ptr();
                    YATO_ASSERT(ptr != nullptr, "Invalid state");
                    allocator_type & alloc = m_raw_vector.allocator();
                    memory::destroy(alloc, ptr, ptr + size);
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
                bool valid_after_error = true;
                try {
                    m_raw_vector.assign(old_size, new_size, valid_after_error, value);
                }
                catch(...) {
                    if(!valid_after_error) {
                        update_top_dimension_(0);
                    }
                    throw;
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
            allocator_type get_allocator() const
            {
                return m_raw_vector.allocator();
            }

            /**
             *  Save swap
             */
            void swap(this_type & other) noexcept
            {
                YATO_REQUIRES(this != &other);
                using std::swap;
                swap(m_descriptors, other.m_descriptors);
                m_raw_vector.swap(other.m_raw_vector);
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
                return m_raw_vector.ptr();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_raw_vector.ptr();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
            {
                return m_raw_vector.ptr() + total_size();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_end() YATO_NOEXCEPT_KEYWORD
            {
                return m_raw_vector.ptr() + total_size();
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
                return m_raw_vector.ptr();
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            const data_type* data() const YATO_NOEXCEPT_KEYWORD
            {
                return m_raw_vector.ptr();
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
                return m_raw_vector.capacity();
            }

            /**
             *  Increase the capacity of the container to a value that's greater or equal to new_capacity
             *  In the case of exception the vector remains unchanged.
             */
            void reserve(size_t new_capacity)
            {
                m_raw_vector.reserve(total_size(), new_capacity);
            }

            /**
             *  Requests the removal of unused capacity.
             *  In case of exception the vector remains unchanged.
             */
            void shrink_to_fit()
            {
                m_raw_vector.shrink_to_fit(total_size());
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
                m_raw_vector.resize(old_size, new_size);
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
                m_raw_vector.resize(old_size, new_size, value);
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
                m_raw_vector.resize(old_size, new_size);
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
                m_raw_vector.resize(old_size, new_size, value);
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
                const auto insert_range = m_raw_vector.prepare_push_back(old_size, new_size);
                value_type* dst = insert_range.begin();
                try {
                    memory::copy_to_uninitialized(m_raw_vector.allocator(), sub_vector.plain_cbegin(), sub_vector.plain_cend(), dst);
                }
                catch(...) {
                    // delete all, since sub-verctor was not inserted wholly.
                    memory::destroy(m_raw_vector.allocator(), insert_range.begin(), dst);
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
                const auto insert_range = m_raw_vector.prepare_push_back(old_size, new_size);
                value_type* dst = insert_range.begin();
                try {
                    memory::move_to_uninitialized(m_raw_vector.allocator(), sub_vector.plain_begin(), sub_vector.plain_end(), dst);
                }
                catch(...) {
                    // delete all, since sub-verctor was not inserted wholly.
                    memory::destroy(m_raw_vector.allocator(), insert_range.begin(), dst);
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
                value_type* old_data = m_raw_vector.ptr();
                memory::destroy(m_raw_vector.allocator(), old_data + new_size, old_data + old_size);
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

                auto insert_range = prepare_insert_(insert_offset, 1, element_size);

                allocator_type& alloc = m_raw_vector.allocator();
                value_type* dst = insert_range.begin();
                try {
                    memory::copy_to_uninitialized(alloc, sub_vector.plain_cbegin(), sub_vector.plain_cend(), dst);
                }
                catch(...) {
                    // destroy partially filled sub-vector
                    memory::destroy(alloc, insert_range.begin(), dst);
                    // destroy the detached right part
                    memory::destroy(alloc, insert_range.end(), m_raw_vector.ptr() + total_size() + element_size);
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

                auto insert_range = prepare_insert_(insert_offset, 1, element_size);

                allocator_type& alloc = m_raw_vector.allocator();
                value_type* dst = insert_range.begin();
                try {
                    memory::move_to_uninitialized(alloc, sub_vector.plain_begin(), sub_vector.plain_end(), dst);
                }
                catch(...) {
                    // destroy partially filled sub-vector
                    memory::destroy(alloc, insert_range.begin(), dst);
                    // destroy the detached right part
                    memory::destroy(alloc, insert_range.end(), m_raw_vector.ptr() + total_size() + element_size);
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

                    auto insert_range = prepare_insert_(insert_offset, count, element_size);
                    value_type* copy_first = insert_range.begin();
                    value_type* copy_last  = insert_range.begin();
                    size_t inserted_count = 0;

                    allocator_type& alloc = m_raw_vector.allocator();
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
                        memory::destroy(alloc, insert_range.end(), m_raw_vector.ptr() + total_size() + count * element_size);
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

                    auto insert_range = prepare_insert_(insert_offset, count, element_size);

                    allocator_type& alloc = m_raw_vector.allocator();
                    value_type* dst = insert_range.begin();
                    try {
                        memory::copy_to_uninitialized(alloc, (*first).plain_begin(), (*last).plain_begin(), dst);
                    }
                    catch(...) {
                        // number of whole elements
                        const size_t inserted_count = (dst - insert_range.begin()) / element_size;
                        // destroy partially filled sub-vector
                        memory::destroy(alloc, insert_range.begin() + inserted_count * element_size, dst);
                        // destroy the detached right part
                        memory::destroy(alloc, insert_range.end(), m_raw_vector.ptr() + total_size() + count * element_size);
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
                return create_proxy_(erase_(position, 1, get_element_size_()));
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
                    return create_proxy_(erase_(first, ucount, get_element_size_()));
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
