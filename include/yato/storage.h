/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_STORAGE_H_
#define _YATO_STORAGE_H_

#include <memory>
#include <array>

#include "types.h"

namespace yato
{
    namespace details
    {
        template <typename _T, size_t _SizeLimit, bool _AllocatesMemory = true>
        class storage_impl final
        {
        public:
            using my_type = storage_impl<_T, _SizeLimit, true>;
            using stored_type = _T;
            using pointer_type = stored_type*;
            using pointer_to_const_type = const stored_type*;
            using reference_type = stored_type&;
            using reference_to_const_type = const stored_type&;
            static YATO_CONSTEXPR_VAR size_t size = sizeof(_T*);
            static YATO_CONSTEXPR_VAR size_t max_size = _SizeLimit;
            static YATO_CONSTEXPR_VAR bool allocates_memory = true;

        private:
            std::unique_ptr<stored_type> m_ptr;

        public:
            storage_impl(const stored_type & obj)
            {
                m_ptr = std::make_unique<stored_type>(obj);
            }

            storage_impl(const my_type & other)
            {
                m_ptr = std::make_unique<stored_type>(*other.get());
            }

            storage_impl(my_type && other)
            {
                m_ptr = std::move(other.m_ptr);
            }

            ~storage_impl()
            {
            }

            void swap(my_type & other) YATO_NOEXCEPT_KEYWORD
            {
                if (this != &other) {
                    m_ptr.swap(other.m_ptr);
                }
            }

            my_type & operator = (const my_type & other)
            {
                if (this != &other) {
                    my_type tmp{ other };
                    tmp.swap(*this);
                }
                return *this;
            }

            my_type & operator = (my_type && other)
            {
                if (this != &other) {
                    m_ptr = std::move(other.m_ptr);
                }
                return *this;
            }

            pointer_type get()
            {
                return m_ptr.get();
            }

            pointer_to_const_type get() const
            {
                return m_ptr.get();
            }

            pointer_type operator->()
            {
                return m_ptr.operator->();
            }

            pointer_to_const_type operator->() const
            {
                return m_ptr.operator->();
            }

            reference_type operator*()
            {
                return *m_ptr;
            }

            reference_to_const_type operator*() const
            {
                return *m_ptr;
            }
        };

        template <typename _T, size_t _SizeLimit>
        class storage_impl <_T, _SizeLimit, false> final
        {
            static_assert(sizeof(_T) <= _SizeLimit, "yato::storage: Data type size should be less than the size limit");
        public:
            using my_type = storage_impl<_T, _SizeLimit, false>;
            using stored_type = _T;
            using pointer_type = stored_type*;
            using pointer_to_const_type = const stored_type*;
            using reference_type = stored_type&;
            using reference_to_const_type = const stored_type&;
            static YATO_CONSTEXPR_VAR size_t size = sizeof(stored_type);
            static YATO_CONSTEXPR_VAR size_t max_size = _SizeLimit;
            static YATO_CONSTEXPR_VAR bool   allocates_memory = false;

        private:
            std::array<uint8_t, size> m_buffer;
            //-------------------------------------------------------

            void _create(const stored_type & obj)
            {
                new(m_buffer.data())stored_type(obj);
            }

            pointer_type _get_pointer() YATO_NOEXCEPT_KEYWORD
            {
                return pointer_cast<pointer_type>(m_buffer.data());
            }

            YATO_CONSTEXPR_FUNC
            pointer_to_const_type _get_pointer() const YATO_NOEXCEPT_KEYWORD
            {
                return pointer_cast<pointer_to_const_type>(m_buffer.data());
            }

            void _destroy()
            {
                _get_pointer()->~stored_type();
            }
            //-------------------------------------------------------

        public:
            storage_impl(const _T & obj)
                : m_buffer{}
            {
                _create(obj);
            }

            storage_impl(const my_type & other)
                : m_buffer{}
            {
                _create(*other.get());
            }

            storage_impl(my_type && other)
                : m_buffer{}
            {
                _create(*other.get());
            }

            ~storage_impl()
            {
                _destroy();
            }

            void swap(my_type & other) 
            {
                if (this != &other) {
                    stored_type tmp{ *_get_pointer() };
                    _destroy();
                    _create(*other._get_pointer());
                    other._destroy();
                    other._create(tmp);
                }
            }

            my_type & operator = (const my_type & other)
            {
                if (this != &other) {
                    _destroy();
                    _create(*other._get_pointer());
                }
                return *this;
            }

            my_type & operator = (my_type && other)
            {
                if (this != &other) {
                    _destroy();
                    _create(*other._get_pointer());
                }
                return *this;
            }

            pointer_type get()
            {
                return _get_pointer();
            }

            YATO_CONSTEXPR_FUNC
            pointer_to_const_type get() const
            {
                return _get_pointer();
            }

            pointer_type operator->()
            {
                return _get_pointer();
            }

            YATO_CONSTEXPR_FUNC
            pointer_to_const_type operator->() const
            {
                return _get_pointer();
            }

            reference_type operator*()
            {
                return *_get_pointer();
            }

            YATO_CONSTEXPR_FUNC
            reference_to_const_type operator*() const
            {
                return *_get_pointer();
            }
        };

        template <typename _T, size_t _SizeLimit, bool _Allocates>
        inline void swap(storage_impl<_T, _SizeLimit, _Allocates> & one, storage_impl<_T, _SizeLimit, _Allocates> & another)
        {
            one.swap(another);
        }
    }

    /**
     *  Copyable, movable and assignable container for CopyConstructible types
     *  If object size is small then stores it object internally, otherwise - on heap
     */
    template <typename _T, size_t _SizeLimit = 64>
    using storage = details::storage_impl <_T, _SizeLimit, (sizeof(_T) > _SizeLimit)>;

    template <typename _T>
    using storage_always_stack = details::storage_impl <_T, sizeof(_T), false>;

    template <typename _T>
    using storage_always_heap = details::storage_impl <_T, sizeof(_T*), true>;

}

#endif
