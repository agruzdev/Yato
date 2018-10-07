/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_VECTOR_ND_H_
#define _YATO_VECTOR_ND_H_

#include <array>
#include <vector>

#include "array_proxy.h"
#include "array_view.h"
#include "assert.h"
#include "compressed_pair.h"
#include "iterator_nd.h"
#include "memory_utility.h"
#include "range.h"
#include "stl_utility.h"

namespace yato
{

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

        struct default_capacity_policy
        {
            /**
             * Calculate new capacity size after reallocation.
             * @param old_size Current capacity.
             * @param new_size Required capacity.
             * @return Capacity to be allocated, probably bigger than required.
             */
            static YATO_CONSTEXPR_FUNC_CXX14
            size_t increase(size_t old_size, size_t new_size)
            {
                YATO_REQUIRES(new_size > old_size);
                if(old_size < std::numeric_limits<size_t>::max() / 2) {
                    return std::max(new_size, 2 * old_size);
                }
                else {
                    return std::numeric_limits<size_t>::max();
                }
            }
        };

        //-------------------------------------------------------
        //

        template <typename ValueTy_, typename Allocator_, typename CapacityPolicy_>
        class raw_vector
        {
            using this_type = raw_vector<ValueTy_, Allocator_, CapacityPolicy_>;
            using value_type = ValueTy_;
            using raw_pointer = ValueTy_*;
            using allocator_type = Allocator_;
            using capacity_policy = CapacityPolicy_;
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
                    const size_t new_capacity = capacity_policy::increase(m_allocated_size, new_size);
                    value_type* const new_data = memory::allocate(alloc, new_capacity);
                    value_type* dst = new_data;
                    try {
                        memory::fill_uninitialized(alloc, dst, new_data + new_size, std::forward<InitArgs_>(init_args)...);
                    }
                    catch(...) {
                        memory::destroy(alloc, new_data, dst);
                        memory::deallocate(alloc, new_data, new_capacity);
                        valid = true;
                        throw;
                    }
                    // destoy old content
                    if(old_data != nullptr) {
                        memory::destroy(alloc, old_data, old_data + old_size);
                        memory::deallocate(alloc, old_data, m_allocated_size);
                    }
                    ptr() = new_data;
                    m_allocated_size = new_capacity;
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
                        const size_t new_capacity = capacity_policy::increase(m_allocated_size, new_size);
                        value_type* const old_data = ptr();
                        value_type* const new_data = memory::allocate(alloc, new_capacity);
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
                            memory::deallocate(alloc, new_data, new_capacity);
                            throw;
                        }
                        // delete old
                        if(old_data != nullptr) {
                            memory::destroy(alloc, old_data, old_data + old_size);
                            memory::deallocate(alloc, old_data, m_allocated_size);
                        }
                        ptr() = new_data;
                        m_allocated_size = new_capacity;
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
                    const size_t new_capacity = capacity_policy::increase(m_allocated_size, new_size);
                    value_type* new_data = memory::allocate(alloc, new_capacity);
                    value_type* old_data = ptr();
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
                    ptr() = new_data;
                    m_allocated_size = new_capacity;
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
                    const size_t new_capacity = capacity_policy::increase(m_allocated_size, new_size);
                    value_type* new_data = memory::allocate(alloc, new_capacity);
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
                            memory::deallocate(alloc, new_data, new_capacity);
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
                            memory::deallocate(alloc, new_data, new_capacity);
                            throw;
                        }
                        // destroy old
                        memory::destroy(alloc, old_data, old_data + old_size);
                        memory::deallocate(alloc, old_data, m_allocated_size);
                    }
                    ptr() = new_data;
                    m_allocated_size = new_capacity;
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
                        memory::destroy(alloc, old_data + insert_offset, old_data + insert_offset + insert_size);
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

        template <typename _DataType, size_t _DimensionsNum, typename _Allocator, typename CapacityPolicy_ = details::default_capacity_policy>
        class vector_nd_impl
            : public container_nd<_DataType, _DimensionsNum, vector_nd_impl<_DataType, _DimensionsNum, _Allocator, CapacityPolicy_>>
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
            using alloc_traits = std::allocator_traits<allocator_type>;

            using size_iterator = typename dimensions_type::iterator;
            using size_const_iterator = typename dimensions_type::const_iterator;

            using proxy       = proxy_nd<value_type,                   dim_descriptor, dimensions_number - 1>;
            using const_proxy = proxy_nd<std::add_const_t<value_type>, dim_descriptor, dimensions_number - 1>;

        public:
            using iterator       = iterator_nd<value_type,                   dim_descriptor, dimensions_number - 1>;
            using const_iterator = iterator_nd<std::add_const_t<value_type>, dim_descriptor, dimensions_number - 1>;

            //-------------------------------------------------------

        private:
            std::array<dim_descriptor::type, dimensions_number> m_descriptors = {};
            raw_vector<value_type, allocator_type, CapacityPolicy_> m_raw_vector;
            //-------------------------------------------------------

            template <typename InitTy_>
            void raw_init_from_ilist_(size_t plain_size, const initializer_list_nd_t<InitTy_, dimensions_number> & init_list)
            {
                YATO_REQUIRES(plain_size != 0);

                allocator_type& alloc = m_raw_vector.allocator();
                value_type* new_data  = memory::allocate(alloc, plain_size);
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

            template <typename MultidimTy_>
            void raw_init_from_multidim_(size_t plain_size, const MultidimTy_ & v)
            {
                YATO_REQUIRES(plain_size != 0);

                allocator_type& alloc = m_raw_vector.allocator();
                value_type* new_data  = memory::allocate(alloc, plain_size);
                value_type* dst = new_data; // copy, that can be changed
                try {
                    // Recursively copy values
                    fill_from_miltidim_(dst, v);
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
                const std::ptrdiff_t offset = std::distance<const_data_iterator>(m_raw_vector.ptr(), (*position).plain_cbegin());
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
                return proxy(m_raw_vector.ptr() + offset * std::get<dim_descriptor::idx_total>(m_descriptors[1]), &m_descriptors[1]);
            }

            proxy create_proxy_(data_iterator plain_position) YATO_NOEXCEPT_KEYWORD
            {
                return proxy(plain_position, &m_descriptors[1]);
            }

            const_proxy create_const_proxy_(size_t offset) const YATO_NOEXCEPT_KEYWORD
            {
                return const_proxy(m_raw_vector.ptr() + offset * std::get<dim_descriptor::idx_total>(m_descriptors[1]), &m_descriptors[1]);
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
                -> decltype(yato::make_range(m_descriptors).map(tuple_getter<dim_descriptor::idx_size>()))
            {
                return yato::make_range(m_descriptors).map(tuple_getter<dim_descriptor::idx_size>());
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

            template <typename MultidimType_>
            auto fill_from_miltidim_(value_type* & dst, const MultidimType_ & v)
                -> std::enable_if_t<(MultidimType_::dimensions_number > 1)>
            {
                for (auto it = v.cbegin(); it != v.cend(); ++it) {
                    fill_from_miltidim_(dst, *it);
                }
            }

            template <typename MultidimType_>
            auto fill_from_miltidim_(value_type* & dst, const MultidimType_ & v)
                -> std::enable_if_t<(MultidimType_::dimensions_number == 1)>
            {
                memory::copy_to_uninitialized(m_raw_vector.allocator(), v.cbegin(), v.cend(), dst);
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

            void destroy_()
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
            vector_nd_impl(const dimensions_type & sizes, vector_nd_impl<data_type, NewDimsNum_, allocator_type> && other)
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
                destroy_(); // destroy current content

                m_descriptors  = std::move(other.m_descriptors);
                m_raw_vector = std::move(other.m_raw_vector);

                // Steal content
                other.tidy_();

                return *this;
            }

            /**
             *  Copy from proxy
             */
            template<typename ProxyValue_, typename ProxyDescriptor_, proxy_access_policy ProxyAccess_>
            explicit
            vector_nd_impl(const proxy_nd<ProxyValue_, ProxyDescriptor_, dimensions_number, ProxyAccess_> & proxy)
            {
                init_sizes_(proxy.dimensions_range());
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    if (proxy.continuous()) {
                        m_raw_vector.init_from_range(plain_size, proxy.plain_cbegin(), proxy.plain_cend());
                    }
                    else {
                        raw_init_from_multidim_(plain_size, proxy);
                    }
                }
            }

            /**
             *  Assign from proxy
             */
            template<typename _DataIterator, typename _SizeIterator, proxy_access_policy ProxyAccess_>
            vector_nd_impl& operator = (const proxy_nd<_DataIterator, _SizeIterator, dimensions_number, ProxyAccess_> & proxy)
            {
                this_type{ proxy }.swap(*this);
                return *this;
            }

            /**
             *  Copy from view
             */
            template <typename OtherTy_>
            explicit
            vector_nd_impl(const yato::array_view_nd<OtherTy_, dimensions_number> & view)
            {
                init_sizes_(view.dimensions_range());
                const size_t plain_size = total_size();
                if(plain_size != 0) {
                    if (view.continuous()) {
                        m_raw_vector.init_from_range(plain_size, view.plain_cbegin(), view.plain_cend());
                    }
                    else {
                        raw_init_from_multidim_(plain_size, view);
                    }
                }
            }

            /**
             *  Assign from view
             */
            template <typename OtherTy_>
            vector_nd_impl& operator = (const yato::array_view_nd<OtherTy_, dimensions_number> & view)
            {
                this_type{ view }.swap(*this);
                return *this;
            }

            /**
             *  Destructor
             */
            ~vector_nd_impl()
            {
                destroy_();
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
             * Reshapes vector stealing content
             */
            template <size_t NewDimsNum_>
            vector_nd_impl<data_type, NewDimsNum_, allocator_type> reshape(const dimensionality<NewDimsNum_, size_t> & extents) &&
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
            YATO_CONSTEXPR_FUNC_CXX14
            const_proxy operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < size(0));
                return create_const_proxy_(idx);
            }
            /**
             *  Element access without bounds check in release
             */
            YATO_CONSTEXPR_FUNC_CXX14
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
                return static_cast<const_iterator>(create_const_proxy_(static_cast<size_t>(0)));
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator begin() YATO_NOEXCEPT_KEYWORD
            {
                return static_cast<iterator>(create_proxy_(static_cast<size_t>(0)));
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return static_cast<const_iterator>(create_const_proxy_(size(0)));
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator end() YATO_NOEXCEPT_KEYWORD
            {
                return static_cast<iterator>(create_proxy_(size(0)));
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
            std::add_pointer_t<value_type> data() YATO_NOEXCEPT_KEYWORD
            {
                return m_raw_vector.ptr();
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            std::add_pointer_t<std::add_const_t<value_type>> cdata() const YATO_NOEXCEPT_KEYWORD
            {
                return m_raw_vector.ptr();
            }

            /**
             * Construct view for the full vector
             */
            auto cview() const
            {
                return yato::array_view_nd<std::add_const_t<value_type>, dimensions_number>(cdata(), dimensions());
            }

            /**
             * Construct view for the full vector
             */
            auto view()
            {
                return yato::array_view_nd<value_type, dimensions_number>(data(), dimensions());
            }

            /**
             * Convert to view for the full vector
             */
            operator yato::array_view_nd<std::add_const_t<value_type>, dimensions_number>() const
            {
                return cview();
            }

            /**
             * Convert to view for the full vector
             */
            operator yato::array_view_nd<value_type, dimensions_number>()
            {
                return view();
            }

            /**
             * Convert to view for the full vector
             */
            operator proxy_nd<std::add_const_t<value_type>, dim_descriptor, dimensions_number>() const
            {
                return proxy_nd<std::add_const_t<value_type>, dim_descriptor, dimensions_number>(m_raw_vector.ptr(), &m_descriptors[0]);
            }

            /**
             * Convert to view for the full vector
             */
            operator proxy_nd<value_type, dim_descriptor, dimensions_number>()
            {
                return proxy_nd<value_type, dim_descriptor, dimensions_number>(m_raw_vector.ptr(), &m_descriptors[0]);
            }

            /**
             *  Checks whether the vector is empty
             */
            bool empty() const YATO_NOEXCEPT_KEYWORD
            {
                return (get_top_dimension_() == 0);
            }

            /**
             *  Data is always continuous in vector
             */
            bool continuous() const
            {
                return true;
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
            YATO_CONSTEXPR_FUNC_CXX14
            size_t dimensions_num() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_number;
            }

            /**
             *  Get number of dimensions
             */
            auto dimensions_range() const
                -> decltype(yato::make_range(m_descriptors).map(tuple_cgetter<dim_descriptor::idx_size>()))
            {
                return make_range(m_descriptors).map(tuple_cgetter<dim_descriptor::idx_size>());
            }
            /**
             *  Get size of specified dimension
             *  If the vector is empty ( empty() returns true ) then calling for size(idx) returns 0 for idx = 0; Return value for any idx > 0 is undefined
             */
            YATO_CONSTEXPR_FUNC_CXX14
            size_type size(size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < dimensions_number);
                return std::get<dim_descriptor::idx_size>(m_descriptors[idx]);
            }
            /**
             * Return stride in bytes
             */
            YATO_CONSTEXPR_FUNC_CXX14
            size_type stride(size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < dimensions_number - 1);
                return std::get<dim_descriptor::idx_total>(m_descriptors[idx + 1]) * sizeof(value_type);
            }
            /**
             *  Get the total size of the vector (number of all elements)
             */
            YATO_CONSTEXPR_FUNC_CXX14
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
            template <typename OtherValue_, typename Impl_>
            void push_back(const container_nd<OtherValue_, dimensions_number - 1, Impl_> & sub_element)
            {
                const auto sub_dims = sub_element.dimensions_range();
                if(!match_sub_dimensions_(sub_dims)) {
                    throw yato::argument_error("yato::vector_nd[push_back]: Subvector dimensions mismatch!");
                }
                init_dimensions_if_empty_(sub_dims);

                const size_t old_size = total_size();
                const size_t new_size = old_size + sub_element.total_size();
                const auto insert_range = m_raw_vector.prepare_push_back(old_size, new_size);
                value_type* dst = insert_range.begin();
                try {
                    memory::copy_to_uninitialized_multidim(m_raw_vector.allocator(), sub_element.cbegin(), sub_element.cend(), dst);
                }
                catch(...) {
                    // delete all, since sub-vector was not inserted wholly.
                    memory::destroy(m_raw_vector.allocator(), insert_range.begin(), dst);
                    throw;
                }
                update_top_dimension_(get_top_dimension_() + 1);
            }

            /**
             *  Add sub-vector element to the back
             */
            template <typename OtherAllocator_>
            void push_back(vector_nd_impl<data_type, dimensions_number - 1, OtherAllocator_> && sub_vector)
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
            template <typename OtherValue_, typename Impl_>
            iterator insert(const const_iterator & position, const container_nd<OtherValue_, dimensions_number - 1, Impl_> & sub_element)
            {
                return insert(position, 1, sub_element);
            }

            /**
             *  Insert sub-vector element
             *  @param position iterator(proxy) to the position to insert element before; If iterator doens't belong to this vector, the behavior is undefined
             */
            template <typename OtherAllocator_>
            iterator insert(const const_iterator & position, vector_nd_impl<data_type, dimensions_number - 1, OtherAllocator_> && sub_vector)
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
            template <typename OtherValue_, typename Impl_>
            iterator insert(const const_iterator & position, size_t count, const container_nd<OtherValue_, dimensions_number - 1, Impl_> & sub_element)
            {
                if(count > 0) {
                    const auto sub_dims = sub_element.dimensions_range();
                    if(!match_sub_dimensions_(sub_dims)) {
                        throw yato::argument_error("yato::vector_nd[insert]: Subvector dimensions number mismatch!");
                    }
                    init_dimensions_if_empty_(sub_dims);

                    const size_t insert_offset = raw_offset_checked_(position);
                    const size_t element_size  = sub_element.total_size();
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
                            memory::copy_to_uninitialized_multidim(m_raw_vector.allocator(), sub_element.cbegin(), sub_element.cend(), copy_last);
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
                // Check that iterator points to something of lower dimensionality
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
                        memory::copy_to_uninitialized_multidim(alloc, first, last, dst);
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
            template<typename IterValue_, typename IterDescriptor_, proxy_access_policy IterAccess_>
            iterator insert(const const_iterator & position, const yato::range<iterator_nd<IterValue_, IterDescriptor_, dimensions_number - 1, IterAccess_>> & range)
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
                    const size_t erase_offset = raw_offset_checked_(first);
                    return create_proxy_(erase_offset);
                }
            }

            /**
             *  Removes the elements in the range 
             */
            iterator erase(const yato::range<const_iterator> & range)
            {
                return erase(range.begin(), range.last());
            }

            //------------------------------------------------------------

            template <typename, size_t, typename, typename>
            friend class vector_nd_impl;
        };







        //-------------------------------------------------------
        // Implementation of 1D case
        // More specific implementation with full std::vector interface

        template <typename _DataType, typename _Allocator, typename CapacityPolicy_>
        class vector_nd_impl<_DataType, 1, _Allocator, CapacityPolicy_>
            : public container_nd<_DataType, 1, vector_nd_impl<_DataType, 1, _Allocator, CapacityPolicy_>>
        {
        public:
            /*
             * Public traits of the multidimensional vector
             */
            using this_type = vector_nd_impl<_DataType, 1, _Allocator>;
            using dimensions_type = dimensionality<1, size_t>;
            using size_type = size_t;
            using value_type = _DataType;
            using data_type  = _DataType;
            using allocator_type = _Allocator;
            using data_iterator       = std::add_pointer_t<_DataType>;
            using const_data_iterator = std::add_pointer_t<std::add_const_t<_DataType>>;
            using reference           = std::add_lvalue_reference_t<_DataType>;
            using const_reference     = std::add_lvalue_reference_t<std::add_const_t<_DataType>>;

            using dim_descriptor = dimension_descriptor<size_type>;

            static YATO_CONSTEXPR_VAR size_t dimensions_number = 1;

            using iterator = data_iterator;
            using const_iterator = const_data_iterator;
            //-------------------------------------------------------

        private:
            using alloc_traits = std::allocator_traits<allocator_type>;

            raw_vector<value_type, allocator_type, CapacityPolicy_> m_raw_vector;
            size_t m_size = 0;
            //-------------------------------------------------------

            YATO_FORCED_INLINE
            size_t raw_offset_checked_(const const_iterator & position) const
            {
                const std::ptrdiff_t offset = std::distance<const_data_iterator>(m_raw_vector.ptr(), position);
#if YATO_DEBUG
                if (offset < 0) {
                    throw yato::argument_error("yato::vector_nd: position iterator doesn't belong to this vector!");
                }
                if (static_cast<size_t>(offset) > m_size) {
                    throw yato::argument_error("yato::vector_nd: position iterator doesn't belong to this vector!");
                }
#endif
                return yato::narrow_cast<size_t>(offset);
            }

            yato::range<value_type*> prepare_insert_(size_t insert_offset, size_t count)
            {
                size_t valid_count_after_error = 0;
                try {
                    return m_raw_vector.prepare_insert(m_size, insert_offset, count, 1, valid_count_after_error);
                }
                catch(...) {
                    // adjust size
                    m_size = valid_count_after_error;
                    throw;
                }
            }

            value_type* erase_(const const_iterator & position, size_t count)
            {
                YATO_REQUIRES(m_size >= count);
                const size_t erase_offset = raw_offset_checked_(position);
                size_t valid_count_after_error = 0;
                value_type* pos = nullptr;
                try {
                    pos = m_raw_vector.erase(m_size, erase_offset, count, 1, valid_count_after_error);
                }
                catch(...) {
                    m_size = valid_count_after_error;
                    throw;
                }
                m_size -= count;
                return pos;
            }

            void destroy_()
            {
                if(m_size != 0) {
                    value_type* ptr = m_raw_vector.ptr();
                    memory::destroy(m_raw_vector.allocator(), ptr, ptr + m_size);
                }
            }

            void tidy_()
            {
                m_size = 0;
            }
            //-------------------------------------------------------

        public:
            /**
             *  Create empty vector
             */
            YATO_CONSTEXPR_FUNC
            vector_nd_impl()
                : m_raw_vector()
            {}

            /**
             *  Create empty vector
             */
            explicit
            vector_nd_impl(const allocator_type & alloc)
                : m_raw_vector(alloc)
            {}

            /**
             *  Create without initialization
             */
            explicit
            vector_nd_impl(const dimensions_type & sizes, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc), m_size(sizes[0])
            {
                m_raw_vector.init_from_value(m_size);
            }

            /**
             *  Create without initialization
             */
            explicit
            vector_nd_impl(size_t size, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc), m_size(size)
            {
                m_raw_vector.init_from_value(m_size);
            }

            /**
             *  Create with initialization
             */
            vector_nd_impl(const dimensions_type & sizes, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc), m_size(sizes[0])
            {
                m_raw_vector.init_from_value(m_size, value);
            }

            /**
             *  Create with initialization
             */
            vector_nd_impl(size_t size, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc), m_size(size)
            {
                m_raw_vector.init_from_value(m_size, value);
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, InputIt first, InputIt last, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc), m_size(sizes[0])
            {
                m_raw_vector.init_from_range(m_size, first, last);
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(size_t size, InputIt first, InputIt last, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc), m_size(size)
            {
                m_raw_vector.init_from_range(m_size, first, last);
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(const dimensions_type & sizes, const yato::range<InputIt> & range, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc), m_size(sizes[0])
            {
                m_raw_vector.init_from_range(m_size, range.begin(), range.end());
            }

            /**
             *  Create from a range of elements
             *  Amount of elements in the range [first, last) should exactly match the given sizes
             */
            template <typename InputIt>
            vector_nd_impl(size_t size, const yato::range<InputIt> & range, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc), m_size(size)
            {
                m_raw_vector.init_from_range(m_size, range.begin(), range.end());
            }

            /**
             *  Create from initializer list
             */
            vector_nd_impl(const std::initializer_list<data_type> & init_list)
                : m_raw_vector(), m_size(init_list.size())
            {
                m_raw_vector.init_from_range(m_size, init_list.begin(), init_list.end());
            }

            /**
             *  Create with sizes from a generic range of sizes without initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc)
            {
                if (range.distance() != dimensions_number) {
                    throw yato::out_of_range_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                m_size = *range.begin();
                m_raw_vector.init_from_value(m_size);
            }

            /**
             *  Create with sizes from a generic range of sizes with initialization
             */
            template<typename _IteratorType>
            vector_nd_impl(const yato::range<_IteratorType> & range, const data_type & value, const allocator_type & alloc = allocator_type())
                : m_raw_vector(alloc)
            {
                if (range.distance() != dimensions_number) {
                    throw yato::out_of_range_error("Constructor takes the amount of arguments equal to dimensions number");
                }
                m_size = *range.begin();
                m_raw_vector.init_from_value(m_size, value);
            }

            /**
             *  Create from std::vector
             */
            template<typename _VecDataType, typename _VecAllocator>
            vector_nd_impl(const std::vector<_VecDataType, _VecAllocator> & vector)
                : m_raw_vector(vector.get_allocator()), m_size(vector.size())
            {
                m_raw_vector.init_from_range(m_size, vector.cbegin(), vector.cend());
            }

            /**
             *  Create from std::vector.
             *  std::vector's data cant be stolen, so just move stored elements.
             */
            template<typename _VecAllocator>
            vector_nd_impl(std::vector<data_type, _VecAllocator> && vector)
                : m_raw_vector(vector.get_allocator()), m_size(vector.size())
            {
                m_raw_vector.init_from_range(m_size, std::make_move_iterator(vector.begin()), std::make_move_iterator(vector.end()));
            }

            /**
             *  Copy constructor
             */
            vector_nd_impl(const vector_nd_impl & other)
                : m_raw_vector(alloc_traits::select_on_container_copy_construction(other.m_raw_vector.allocator()))
                , m_size(other.m_size)
            {
                m_raw_vector.init_from_range(m_size, other.plain_cbegin(), other.plain_cend());
            }

            /**
             * Move-copy constructor
             */
            vector_nd_impl(vector_nd_impl && other) noexcept
                : m_raw_vector(std::move(other.m_raw_vector))
                , m_size(other.m_size)
            {
                // steal
                other.tidy_();
            }

            /**
             *  Copy assign
             */
            vector_nd_impl & operator= (const vector_nd_impl & other)
            {
                YATO_REQUIRES(this != &other);
                this_type{other}.swap(*this);
                return *this;
            }

            /**
             *  Move assign
             */
            vector_nd_impl & operator= (vector_nd_impl && other) noexcept
            {
                YATO_REQUIRES(this != &other);
                destroy_();
                m_raw_vector = std::move(other.m_raw_vector);
                m_size = other.m_size;
                // steal
                other.tidy_();
                return *this;
            }

            /**
             *  Assign from std::vector
             */
            template<typename _VecDataType, typename _VecAllocator>
            vector_nd_impl & operator= (const std::vector<_VecDataType, _VecAllocator> & vector)
            {
                this_type{vector}.swap(*this);
                return *this;
            }

            /**
             *  Assign from std::vector
             */
            template<typename _VecAllocator>
            vector_nd_impl & operator= (std::vector<value_type, _VecAllocator> && vector)
            {
                this_type{std::move(vector)}.swap(*this);
                return *this;
            }

            /**
             *  Copy from proxy
             */
            template<typename ProxyValue_, typename ProxyDescriptor_, proxy_access_policy ProxyAccess_>
            explicit
            vector_nd_impl(const proxy_nd<ProxyValue_, ProxyDescriptor_, dimensions_number, ProxyAccess_> & proxy)
                : m_raw_vector(), m_size(proxy.total_size())
            {
                m_raw_vector.init_from_range(m_size, proxy.plain_cbegin(), proxy.plain_cend());
            }

            /**
             *  Assign from proxy
             */
            template<typename ProxyValue_, typename ProxyDescriptor_, proxy_access_policy ProxyAccess_>
            vector_nd_impl& operator= (const proxy_nd<ProxyValue_, ProxyDescriptor_, dimensions_number, ProxyAccess_> & proxy)
            {
                this_type{ proxy }.swap(*this);
                return *this;
            }

            /**
             *  Copy from view
             */
            template <typename OtherTy_>
            explicit
            vector_nd_impl(const yato::array_view_nd<OtherTy_, dimensions_number> & view)
                : m_raw_vector(), m_size(view.total_size())
            {
                m_raw_vector.init_from_range(m_size, view.plain_cbegin(), view.plain_cend());
            }

            /**
             *  Assign from view
             */
            template <typename OtherTy_>
            vector_nd_impl& operator= (const yato::array_view_nd<OtherTy_, dimensions_number> & view)
            {
                this_type{ view }.swap(*this);
                return *this;
            }

            /**
             *  Move-reshape
             */
            template <size_t NewDimsNum_>
            vector_nd_impl(const dimensions_type & sizes, vector_nd_impl<data_type, NewDimsNum_, allocator_type> && other)
                : m_raw_vector()
            {
                if(sizes.total_size() != other.total_size()) {
                    throw yato::argument_error("yato::vector_nd[move-reshape]: Total size mismatch.");
                }
                m_size = sizes.total_size();
                m_raw_vector = std::move(other.m_raw_vector);
                // Steal content
                other.tidy_();
            }

            /**
             *  Destructor
             */
            ~vector_nd_impl()
            {
                destroy_();
            }

            /**
             *  Convert to std::vector
             */
            operator std::vector<data_type, allocator_type>() const &
            {
                return std::vector<data_type, allocator_type>(plain_cbegin(), plain_cend(), m_raw_vector.allocator());
            }

            /**
             *  Convert to std::vector
             */
            operator std::vector<data_type, allocator_type>() &&
            {
                return std::vector<data_type, allocator_type>(std::make_move_iterator(plain_cbegin()), std::make_move_iterator(plain_cend()), m_raw_vector.allocator());
            }

            /**
             *  Replaces the contents of the container
             */
            void assign(size_t size, const data_type & value)
            {
                bool valid_after_error = true;
                try {
                    m_raw_vector.assign(m_size, size, valid_after_error, value);
                }
                catch(...) {
                    if(!valid_after_error) {
                        m_size = 0;
                    }
                    throw;
                }
                m_size = size;
            }

            /**
             *  Replaces the contents of the container
             */
            void assign(const dimensions_type & sizes, const data_type & value)
            {
                assign(sizes[0], value);
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
             * Reshapes vector stealing content
             */
            template <size_t NewDimsNum_>
            vector_nd_impl<data_type, NewDimsNum_, allocator_type> reshape(const dimensionality<NewDimsNum_, size_t> & extents) &&
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
                return m_raw_vector.get_allocator();
            }

            /**
             *  Safe swap
             */
            void swap(this_type & other) noexcept
            {
                m_raw_vector.swap(other.m_raw_vector);
                std::swap(m_size, other.m_size);
            }

            /**
             *  Element access without bounds check in release
             */
            YATO_CONSTEXPR_FUNC_CXX14
            const_reference operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < m_size);
                return *(m_raw_vector.ptr() + idx);
            }

            /**
             *  Element access without bounds check in release
             */
            YATO_CONSTEXPR_FUNC_CXX14
            reference operator[](size_t idx) YATO_NOEXCEPT_KEYWORD
            {
                YATO_REQUIRES(idx < m_size);
                return *(m_raw_vector.ptr() + idx);
            }

            /**
             *  Element access with bounds check
             */
            const_reference at(size_t idx) const
            {
                if(idx >= m_size) {
                    throw yato::out_of_range_error("yato::vector_1d: Index " + yato::stl::to_string(idx) + " is out of range!");
                }
                return operator[](idx);
            }

            /**
             *  Element access with bounds check
             */
            reference at(size_t idx)
            {
                if(idx >= m_size) {
                    throw yato::out_of_range_error("yato::vector_1d: Index " + yato::stl::to_string(idx) + " is out of range!");
                }
                return operator[](idx);
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cbegin() const
            {
                return plain_cbegin();
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator begin()
            {
                return plain_begin();
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            const_iterator cend() const
            {
                return plain_cend();
            }

            /**
             *  Iterator for accessing sub-array elements along the top dimension
             */
            iterator end()
            {
                return plain_end();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cbegin() const
            {
                return m_raw_vector.ptr();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_begin()
            {
                return m_raw_vector.ptr();
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            const_data_iterator plain_cend() const
            {
                return m_raw_vector.ptr() + m_size;
            }

            /**
             *  Iterator for accessing elements trough all dimensions
             */
            data_iterator plain_end()
            {
                return m_raw_vector.ptr() + m_size;
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<const_data_iterator> crange() const
            {
                return yato::make_range(cbegin(), cend());
            }

            /**
             *  Range for accessing sub-array elements trough the top dimension
             */
            yato::range<data_iterator> range()
            {
                return yato::make_range(begin(), end());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<const_data_iterator> plain_crange() const
            {
                return yato::make_range(plain_cbegin(), plain_cend());
            }

            /**
             *  Range for accessing elements trough all dimensions
             */
            yato::range<data_iterator> plain_range()
            {
                return yato::make_range(plain_begin(), plain_end());
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            std::add_pointer_t<value_type> data()
            {
                return m_raw_vector.ptr();
            }

            /**
             * Get a raw pointer to stored data beginning
             */
            std::add_pointer_t<std::add_const_t<value_type>> cdata() const
            {
                return m_raw_vector.ptr();
            }

            /**
             * Construct view for the full vector
             */
            auto cview() const
            {
                return yato::array_view_1d<std::add_const_t<value_type>>(cdata(), yato::dims(m_size));
            }

            /**
             * Construct view for the full vector
             */
            auto view()
            {
                return yato::array_view_1d<value_type>(data(), yato::dims(m_size));
            }

            /**
             * Convert to view for the full vector
             */
            operator yato::array_view_1d<std::add_const_t<value_type>>() const
            {
                return cview();
            }

            /**
             * Convert to view for the full vector
             */
            operator yato::array_view_1d<value_type>()
            {
                return view();
            }

            /**
             * Convert to view for the full vector
             */
            operator yato::proxy_nd<std::add_const_t<value_type>, dim_descriptor, 1>() const
            {
                return yato::proxy_nd<std::add_const_t<value_type>, dim_descriptor, 1>(data(), &m_size, &m_size);
            }

            /**
             * Convert to view for the full vector
             */
            operator yato::proxy_nd<value_type, dim_descriptor, 1>()
            {
                return yato::proxy_nd<value_type, dim_descriptor, 1>(data(), &m_size, &m_size);
            }

            /**
             *  Checks whether the vector is empty
             */
            bool empty() const
            {
                return m_size == 0;
            }

            /**
             *  Data is always continuous in vector
             */
            bool continuous() const
            {
                return true;
            }

            /**
             *  Get dimensions
             */
            dimensions_type dimensions() const
            {
                return yato::dims(m_size);
            }

            /**
             *  Get number of dimensions
             */
            size_t dimensions_num() const
            {
                return dimensions_number;
            }

            /**
             *  Get number of dimensions
             */
            auto dimensions_range() const
                -> yato::range<yato::numeric_iterator<size_t>>
            {
                return yato::make_range(yato::numeric_iterator<size_t>(m_size), yato::numeric_iterator<size_t>(m_size + 1));
            }

            /**
             *  Get size of specified dimension
             */
            size_t size(size_t idx) const
            {
                YATO_REQUIRES(idx < dimensions_number);
                YATO_MAYBE_UNUSED(idx);
                return m_size;
            }

            /**
             *  Get size of specified dimension
             */
            size_t size() const
            {
                return m_size;
            }

            /**
             *  1D vector has no stride
             */
            size_t stride(size_t idx) const
            {
                YATO_REQUIRES(idx < dimensions_number - 1);
                YATO_MAYBE_UNUSED(idx);
                return 0;
            }

            /**
             *  Get the total size of the vector (number of all elements)
             */
            size_t total_size() const
            {
                return m_size;
            }

            /**
             *  Returns the number of elements that the container has currently allocated space for
             */
            size_t capacity() const
            {
                return m_raw_vector.capacity();
            }

            /**
             *  Increase the capacity of the container to a value that's greater or equal to new_capacity
             */
            void reserve(size_t new_capacity)
            {
                m_raw_vector.reserve(m_size, new_capacity);
            }

            /**
             *  Clear vector
             */
            void clear()
            {
                destroy_();
                m_size = 0;
            }

            /**
             *  Requests the removal of unused capacity
             */
            void shrink_to_fit()
            {
                m_raw_vector.shrink_to_fit(m_size);
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than the current size, then all containing data will be preserved
             */
            void resize(size_t length)
            {
                m_raw_vector.resize(m_size, length);
                m_size = length;
            }

            /**
             *  Resize vector length along the top dimension
             *  If length is bigger than the current size, then all containing data will be preserved
             */
            void resize(size_t length, const data_type & value)
            {
                m_raw_vector.resize(m_size, length, value);
                m_size = length;
            }

            /**
             *  Resize vector extents. 
             *  All stored data becomes invalid
             */
            void resize(const dimensions_type & extents)
            {
                m_raw_vector.resize(m_size, extents[0]);
                m_size = extents[0];
            }

            /**
             *  Resize vector extents.
             *  All stored data becomes invalid
             */
            void resize(const dimensions_type & extents, const data_type & value)
            {
                m_raw_vector.resize(m_size, extents[0], value);
                m_size = extents[0];
            }

            /**
             *  Get the first sub-vector proxy
             */
            const_reference front() const
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
                return *(m_raw_vector.ptr());
            }

            /**
             *  Get the first sub-vector proxy
             */
            reference front()
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[front]: vector is empty");
                }
                return *(m_raw_vector.ptr());
            }

            /**
             *  Get the last sub-vector proxy
             */
            const_reference back() const
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[back]: vector is empty");
                }
                return *(m_raw_vector.ptr() + m_size - 1);
            }

            /**
             *  Get the last sub-vector proxy
             */
            reference back()
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[back]: vector is empty");
                }
                return *(m_raw_vector.ptr() + m_size - 1);
            }

            /**
             *  Add sub-vector element to the back
             */
            void push_back(const data_type & value)
            {
                const auto insert_range = m_raw_vector.prepare_push_back(m_size, m_size + 1);
                // may throw
                // no need to handle error since copying only one element to the end
                alloc_traits::construct(m_raw_vector.allocator(), insert_range.begin(), value);
                ++m_size;
            }

            /**
             *  Add sub-vector element to the back
             */
            void push_back(data_type && value)
            {
                const auto insert_range = m_raw_vector.prepare_push_back(m_size, m_size + 1);
                // may throw
                // no need to handle error since copying only one element to the end
                alloc_traits::construct(m_raw_vector.allocator(), insert_range.begin(), std::move(value));
                ++m_size;
            }

            /**
             * Create element at the end
             */
            template <typename... Args_>
            reference emplace_back(Args_ && ... args)
            {
                const auto insert_range = m_raw_vector.prepare_push_back(m_size, m_size + 1);
                value_type* dst = insert_range.begin();
                // may throw
                // no need to handle error since copying only one element to the end
                alloc_traits::construct(m_raw_vector.allocator(), dst, std::forward<Args_>(args)...);
                ++m_size;
                return *dst;
            }

            /**
             *  Removes the last element of the container.
             */
            void pop_back()
            {
                if (empty()) {
                    throw yato::out_of_range_error("yato::vector_nd[pop_back]: vector is already empty!");
                }
                --m_size;
                alloc_traits::destroy(m_raw_vector.allocator(), m_raw_vector.ptr() + m_size);
            }

            /**
             *  Inserts elements at the specified location before 'position'
             */
            iterator insert(const const_iterator & position, const data_type & value)
            {
                const size_t insert_offset = raw_offset_checked_(position);
                const auto insert_range = prepare_insert_(insert_offset, 1);
                try {
                    alloc_traits::construct(m_raw_vector.allocator(), insert_range.begin(), value);
                }
                catch(...) {
                    allocator_type& alloc = m_raw_vector.allocator();
                    const size_t valid_count = insert_offset;
                    // destroy detached right part
                    memory::destroy(alloc, insert_range.end(), m_raw_vector.ptr() + m_size + 1);
                    // adjust size
                    m_size = valid_count;
                    throw;
                }
                ++m_size;
                return insert_range.begin();
            }

            /**
             *  Inserts elements at the specified location before 'position'
             */
            iterator insert(const const_iterator & position, data_type && value)
            {
                const size_t insert_offset = raw_offset_checked_(position);
                const auto insert_range = prepare_insert_(insert_offset, 1);
                try {
                    alloc_traits::construct(m_raw_vector.allocator(), insert_range.begin(), std::move(value));
                }
                catch(...) {
                    allocator_type& alloc = m_raw_vector.allocator();
                    const size_t valid_count = insert_offset;
                    // destroy detached right part
                    memory::destroy(alloc, insert_range.end(), m_raw_vector.ptr() + m_size + 1);
                    // adjust size
                    m_size = valid_count;
                    throw;
                }
                ++m_size;
                return insert_range.begin();
            }

            /**
             *  Create element at the specified location before 'position'
             */
            template <typename... Args_>
            iterator emplace(const const_iterator & position, Args_ && ... args)
            {
                const size_t insert_offset = raw_offset_checked_(position);
                const auto insert_range = prepare_insert_(insert_offset, 1);
                try {
                    alloc_traits::construct(m_raw_vector.allocator(), insert_range.begin(), std::forward<Args_>(args)...);
                }
                catch(...) {
                    allocator_type& alloc = m_raw_vector.allocator();
                    const size_t valid_count = insert_offset;
                    // destroy detached right part
                    memory::destroy(alloc, insert_range.end(), m_raw_vector.ptr() + m_size + 1);
                    // adjust size
                    m_size = valid_count;
                    throw;
                }
                ++m_size;
                return insert_range.begin();
            }

            /**
             *  Inserts elements at the specified location before 'position'
             */
            iterator insert(const const_iterator & position, size_t count, const data_type & value)
            {
                const size_t insert_offset = raw_offset_checked_(position);
                if(count == 0) {
                    return plain_begin() + insert_offset;
                }
                const auto insert_range = prepare_insert_(insert_offset, count);
                allocator_type& alloc = m_raw_vector.allocator();
                
                value_type* dst = insert_range.begin();
                size_t valid_count = insert_offset;
                try {
                    size_t insert_count = count;
                    while(insert_count--) {
                        alloc_traits::construct(alloc, dst, value);
                        ++dst;
                        ++valid_count;
                    }
                }
                catch(...) {
                    // destroy detached right part
                    memory::destroy(alloc, insert_range.end(), m_raw_vector.ptr() + m_size + count);
                    // adjust size
                    m_size = valid_count;
                    throw;
                }
                m_size += count;
                return insert_range.begin();
            }

            /**
             *  Inserts elements from range [first, last) before 'position'
             */
            template<class InputIt_, typename = 
                std::enable_if_t<yato::is_iterator<InputIt_>::value>
            >
            iterator insert(const const_iterator & position, InputIt_ first, InputIt_ last)
            {
                const size_t insert_offset = raw_offset_checked_(position);
                const auto icount = std::distance(first, last);
                if (icount < 0) {
                    throw yato::argument_error("yato::vector_1d[insert]: Invalid iterators.");
                }
                if (icount == 0) {
                    return plain_begin() + insert_offset;
                }
                const auto insert_count = static_cast<size_t>(icount);
                const auto insert_range = prepare_insert_(insert_offset, insert_count);
                allocator_type& alloc = m_raw_vector.allocator();
                value_type* dst = insert_range.begin();
                try {
                    memory::copy_to_uninitialized(alloc, first, last, dst);
                }
                catch(...) {
                    const size_t valid_count = dst - m_raw_vector.ptr();
                    // destroy detached right part
                    memory::destroy(alloc, insert_range.end(), m_raw_vector.ptr() + m_size + insert_count);
                    // adjust size
                    m_size = valid_count;
                    throw;
                }
                m_size += insert_count;
                return insert_range.begin();
            }

            /**
             *  Inserts elements from range before 'position'
             */
            template<class _InputIt>
            iterator insert(const const_iterator & position, const yato::range<_InputIt> & range)
            {
                return insert(position, range.begin(), range.end());
            }

            /**
             *  Removes the element at 'position'
             */
            iterator erase(const const_iterator & position)
            {
                //return m_plain_vector.erase(position);
                if(empty()) {
                    throw yato::argument_error("yato::vector_1d[erase]: Vector is empty!");
                }
                return erase_(position, 1);
            }

            /**
             *  Removes the elements in the range [first; last)
             */
            iterator erase(const const_iterator & first, const const_iterator & last)
            {
                const std::ptrdiff_t count = std::distance(first, last);
                if(count < 0) {
                    throw yato::argument_error("yato::vector_1d[erase]: Invalid range iterators!");
                }
                if(count != 0) {
                    const size_t ucount = yato::narrow_cast<size_t>(count);
                    if(ucount > m_size) {
                        throw yato::argument_error("yato::vector_1d[erase]: Invalid range iterators!");
                    }
                    return erase_(first, ucount);
                } else {
                    const size_t erase_offset = raw_offset_checked_(first);
                    return plain_begin() + erase_offset;
                }
            }

            /**
             *  Removes the elements in the range 
             */
            iterator erase(const yato::range<const_iterator> & range)
            {
                return erase(range.begin(), range.last());
            }

            //------------------------------------------------------------

            template <typename, size_t, typename, typename>
            friend class vector_nd_impl;
        };

    }

    template <typename DataType_, size_t DimensionsNum_,
        typename Allocator_ = std::allocator<DataType_>,
        typename CapacityPolicy_ = details::default_capacity_policy
    >
    using vector_nd = details::vector_nd_impl<DataType_, DimensionsNum_, Allocator_, CapacityPolicy_>;

    template <typename DataType_,
        typename Allocator_ = std::allocator<DataType_>,
        typename CapacityPolicy_ = details::default_capacity_policy
    >
    using vector_1d = vector_nd<DataType_, 1, Allocator_, CapacityPolicy_>;

    template <typename DataType_,
        typename Allocator_ = std::allocator<DataType_>,
        typename CapacityPolicy_ = details::default_capacity_policy
    >
    using vector = vector_nd<DataType_, 1, Allocator_, CapacityPolicy_>;

    template <typename DataType_,
        typename Allocator_ = std::allocator<DataType_>,
        typename CapacityPolicy_ = details::default_capacity_policy
    >
    using vector_2d = vector_nd<DataType_, 2, Allocator_, CapacityPolicy_>;

    template <typename DataType_, 
        typename Allocator_ = std::allocator<DataType_>,
        typename CapacityPolicy_ = details::default_capacity_policy
    >
    using vector_3d = vector_nd<DataType_, 3, Allocator_, CapacityPolicy_>;

    template <typename DataType_,
        typename Allocator_ = std::allocator<DataType_>,
        typename CapacityPolicy_ = details::default_capacity_policy
    >
    using vector_4d = vector_nd<DataType_, 4, Allocator_, CapacityPolicy_>;
}

#endif //_YATO_VECTOR_ND_H_
