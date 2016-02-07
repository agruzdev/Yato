/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ARRAY_RPOXY_H_
#define _YATO_ARRAY_RPOXY_H_

#include "assert.h"
#include "not_null.h"
#include "types.h"
#include "range.h"

namespace yato
{

#ifdef YATO_MSVC
    /*  Disable unreachable code warning appearing due to additional code in ternary operator with throw
    *	MSVC complains about type cast otherwise
    */
#pragma warning(push)
#pragma warning(disable:4702) 
#endif
    namespace details
    {
        template<typename _DataIterator, typename _SizeIterator, size_t _DimsNum>
        class sub_array_proxy;

        //-------------------------------------------------------
        // Iterator for proxy types
        // 

        template<typename _DataIterator, typename _SizeIterator, size_t _DimsNum>
        class proxy_iterator
        {
            using size_iterator = _SizeIterator;
            using data_iterator = _DataIterator;
            static YATO_CONSTEXPR_VAR size_t dimensions_num = _DimsNum;

            using sub_proxy = sub_array_proxy<data_iterator, size_iterator, dimensions_num - 1>;
            using const_sub_proxy = sub_array_proxy<const data_iterator, size_iterator, dimensions_num - 1>;

        public:
            using value_type = sub_proxy;
            using difference_type = std::ptrdiff_t;
            using pointer = typename std::add_pointer<value_type>::type;
            using pointer_to_const = typename std::add_pointer<const value_type>::type;
            using reference = typename std::add_lvalue_reference<value_type>::type;
            using reference_to_const = typename std::add_lvalue_reference<const value_type>::type;
            using iterator_category = std::random_access_iterator_tag;
            //-------------------------------------------------------

        private:
            data_iterator m_data_iter;
            size_iterator m_sizes_iter;
            size_iterator m_offsets_iter;

            //-------------------------------------------------------
        public:
            YATO_CONSTEXPR_FUNC
            proxy_iterator(const data_iterator & data, const size_iterator & sizes, const size_iterator & offsets) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(data), m_sizes_iter(sizes), m_offsets_iter(offsets)
            {}

            proxy_iterator(const proxy_iterator&) = default;

            proxy_iterator(proxy_iterator && other)
                : m_data_iter(other.m_data_iter), m_sizes_iter(other.m_sizes_iter), m_offsets_iter(other.m_offsets_iter)
            { }

            proxy_iterator & operator= (const proxy_iterator &) = default;

            proxy_iterator & operator= (proxy_iterator && other)
            {
                if (this != &other) {
                    m_data_iter = std::move(other.m_data_iter);
                    m_sizes_iter = std::money_base(other.m_sizes_iter);
                    m_offsets_iter = std::move(other.m_offsets_iter);
                }
                return *this;
            }

            ~proxy_iterator()
            {}

            /**
             *  Return proxy for the current position
             */
            YATO_CONSTEXPR_FUNC
                auto operator* () const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if < (_DimsNum > 1), const_sub_proxy > ::type;

            /**
             *  Return proxy for the current position
             */
            auto operator* () YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if < (_DimsNum > 1), sub_proxy > ::type;

            /**
             *  Increment iterator
             */
            proxy_iterator operator++() YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter += m_offsets_iter[1];
                return *this;
            }
            /**
             *  Increment iterator
             */
            proxy_iterator operator++(int) 
            {
                auto temp = *this;
                m_data_iter += m_offsets_iter[1];
                return temp;
            }
            /**
             *  Decrement iterator
             */
            proxy_iterator operator--() YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter -= m_offsets_iter[1];
                return *this;
            }

            /**
             *  Decrement iterator
             */ 
            proxy_iterator operator--(int)
            {
                auto temp = *this;
                m_data_iter -= m_offsets_iter[1];
                return temp;
            }

            /**
             *  Add offset
             */
            proxy_iterator& operator+= (size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter += offset * m_offsets_iter[1];
                return *this;
            }

            /**
             *  Add offset
             */
            proxy_iterator operator+ (size_t offset) const
            {
                auto temp = *this;
                return (temp += offset);
            }

            /**
             *  Add offset
             */
            friend 
            proxy_iterator operator+ (size_t offset, const proxy_iterator& iter)
            {
                return iter + offset;
            }

            /**
             *  Subtract offset
             */
            proxy_iterator& operator-= (size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter -= offset * m_offsets_iter[1];
                return *this;
            }

            /**
             *  Distance between iterators
             */
            difference_type operator- (const proxy_iterator& other)
            {
                return (m_data_iter - other.m_data_iter) / m_offsets_iter[1];
            }

            /**
             *  Random access
             */
            YATO_CONSTEXPR_FUNC
            const_sub_proxy operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                return *(*this + idx);
            }

            /**
             *  Random access
             */
            sub_proxy operator[](size_t idx) YATO_NOEXCEPT_KEYWORD
            {
                return *(*this + idx);
            }

            /**
             *  Equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator== (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter == other.m_data_iter;
            }
            
            /**
             *  Not equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator!= (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter != other.m_data_iter;
            }

            /**
             *  Less
             */
            YATO_CONSTEXPR_FUNC
            bool operator< (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter < other.m_data_iter;
            }

            /**
             *  Greater
             */
            YATO_CONSTEXPR_FUNC
            bool operator> (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter > other.m_data_iter;
            }

            /**
             *  Less or equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator<= (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter <= other.m_data_iter;
            }

            /**
             *  Greater or equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator>= (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter >= other.m_data_iter;
            }
        };

        //-------------------------------------------------------
        // 1D case
        //
        template<typename _DataIterator, typename _SizeIterator>
        class proxy_iterator<_DataIterator, _SizeIterator, 1>
        {
            using size_iterator = _SizeIterator;
            using data_iterator = _DataIterator;
            static YATO_CONSTEXPR_VAR size_t dimensions_num = 1;

        public:
            using value_type = decltype(*std::declval<data_iterator>());
            using difference_type = std::ptrdiff_t;
            using pointer = typename std::add_pointer<value_type>::type;
            using pointer_to_const = typename std::add_pointer<const value_type>::type;
            using reference = typename std::add_lvalue_reference<value_type>::type;
            using reference_to_const = typename std::add_lvalue_reference<const value_type>::type;
            using iterator_category = std::random_access_iterator_tag;
            //-------------------------------------------------------

        private:
            data_iterator m_data_iter;

            //-------------------------------------------------------
        public:
            YATO_CONSTEXPR_FUNC
            proxy_iterator(const data_iterator & data, const size_iterator&, const size_iterator&) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(data)
            {}

            YATO_CONSTEXPR_FUNC
            proxy_iterator(const data_iterator & data) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(data)
            {}

            proxy_iterator(const proxy_iterator&) = default;

            proxy_iterator(proxy_iterator && other)
                : m_data_iter(other.m_data_iter)
            {}

            proxy_iterator & operator= (const proxy_iterator &) = default;

            proxy_iterator & operator= (proxy_iterator && other)
            {
                if (this != &other) {
                    m_data_iter = std::move(other.m_data_iter);
                }
                return *this;
            }

            ~proxy_iterator()
            {}

            /**
             *  Return proxy for the current position
             */
            YATO_CONSTEXPR_FUNC
            reference_to_const operator* () const YATO_NOEXCEPT_KEYWORD
            {
                return *m_data_iter;
            }

            /**
             *  Return proxy for the current position
             */
            reference operator* () YATO_NOEXCEPT_KEYWORD
            {
                return *m_data_iter;
            }

            /**
             *  Increment iterator
             */
            proxy_iterator operator++() YATO_NOEXCEPT_KEYWORD
            {
                ++m_data_iter;
                return *this;
            }
            /**
             *  Increment iterator
             */
            proxy_iterator operator++(int)
            {
                auto temp = *this;
                ++m_data_iter;
                return temp;
            }
            /**
             *  Decrement iterator
             */
            proxy_iterator operator--() YATO_NOEXCEPT_KEYWORD
            {
                --m_data_iter;
                return *this;
            }

            /**
             *  Decrement iterator
             */
            proxy_iterator operator--(int)
            {
                auto temp = *this;
                --m_data_iter;
                return temp;
            }

            /**
             *  Add offset
             */
            proxy_iterator& operator+= (size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter += offset;
                return *this;
            }

            /**
             *  Add offset
             */
            proxy_iterator operator+ (size_t offset) const
            {
                auto temp = *this;
                return (temp += offset);
            }

            /**
             *  Add offset
             */
            friend
            proxy_iterator operator+ (size_t offset, const proxy_iterator& iter)
            {
                return iter + offset;
            }

            /**
             *  Subtract offset
             */
            proxy_iterator& operator-= (size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter -= offset;
                return *this;
            }

            /**
             *  Distance between iterators
             */
            difference_type operator- (const proxy_iterator& other)
            {
                return (m_data_iter - other.m_data_iter);
            }

            /**
             *  Random access
             */
            YATO_CONSTEXPR_FUNC
            reference_to_const operator[](size_t idx) const YATO_NOEXCEPT_KEYWORD
            {
                return *(*this + idx);
            }

            /**
             *  Random access
             */
            reference operator[](size_t idx) YATO_NOEXCEPT_KEYWORD
            {
                return *(*this + idx);
            }

            /**
             *  Equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator== (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter == other.m_data_iter;
            }

            /**
             *  Not equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator!= (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter != other.m_data_iter;
            }

            /**
             *  Less
             */
            YATO_CONSTEXPR_FUNC
            bool operator< (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter < other.m_data_iter;
            }

            /**
             *  Greater
             */
            YATO_CONSTEXPR_FUNC
            bool operator> (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter > other.m_data_iter;
            }

            /**
             *  Less or equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator<= (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter <= other.m_data_iter;
            }

            /**
             *  Greater or equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator>= (const proxy_iterator& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter >= other.m_data_iter;
            }
        };





        //-------------------------------------------------------
        // Multidimensional proxy class
        // 

        template<typename _DataIterator, typename _SizeIterator, size_t _DimsNum>
        class sub_array_proxy
        {
            using size_iterator = _SizeIterator;
            using data_iterator = _DataIterator;
            using const_data_iterator = const data_iterator;
            using reference = decltype(*std::declval<data_iterator>());

            static YATO_CONSTEXPR_VAR size_t dimensions_num = _DimsNum;

            using sub_view = sub_array_proxy<data_iterator, size_iterator, dimensions_num - 1>;

        public:
            using iterator = proxy_iterator<data_iterator, const size_iterator, dimensions_num - 1>;
            using const_iterator = proxy_iterator<data_iterator, const size_iterator, dimensions_num - 1>;
            //-------------------------------------------------------

        private:
            data_iterator m_data_iter;
            size_iterator m_sizes_iter;
            size_iterator m_offsets_iter;
            //-------------------------------------------------------

            template<size_t _MyDimsNum = _DimsNum>
            auto _create_iterator(size_t offset)
                -> typename std::enable_if<(_MyDimsNum > 1), iterator>::type
            {
                return iterator(std::next(m_data_iter, offset), m_sizes_iter, m_offsets_iter);
            }

            template<size_t _MyDimsNum = _DimsNum>
            auto _create_iterator(size_t offset)
                -> typename std::enable_if<(_MyDimsNum == 1), iterator>::type
            {
                return iterator(std::next(m_data_iter, offset));
            }

            template<size_t _MyDimsNum = _DimsNum>
            auto _create_const_iterator(size_t offset)
                -> typename std::enable_if<(_MyDimsNum > 1), const_iterator> ::type
            {
                return const_iterator(std::next(m_data_iter, offset), m_sizes_iter, m_offsets_iter);
            }

            template<size_t _MyDimsNum = _DimsNum>
            auto _create_const_iterator(size_t offset)
                -> typename std::enable_if<(_MyDimsNum == 1), const_iterator>::type
            {
                return const_iterator(std::next(m_data_iter, offset));
            }
            //-------------------------------------------------------

        public:
            YATO_CONSTEXPR_FUNC
            sub_array_proxy(const data_iterator & data, const size_iterator & sizes, const size_iterator & offsets) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(data), m_sizes_iter(sizes), m_offsets_iter(offsets)
            {}

            sub_array_proxy(const sub_array_proxy&) = default;

            sub_array_proxy(sub_array_proxy && other) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(std::move(other.m_data_iter)), m_sizes_iter(std::move(other.m_sizes_iter)), m_offsets_iter(std::move(other.m_offsets_iter))
            {}

            sub_array_proxy & operator= (const sub_array_proxy&) = default;

            sub_array_proxy & operator= (sub_array_proxy && other) YATO_NOEXCEPT_KEYWORD
            {
                if (this != &other) {
                    m_data_iter = std::move(other.m_data_iter);
                    m_sizes_iter = std::move(other.m_sizes_iter);
                    m_offsets_iter = std::move(other.m_offsets_iter);
                }
                return *this;
            }

            ~sub_array_proxy()
            {}

            template<size_t _MyDimsNum = _DimsNum>
            YATO_CONSTEXPR_FUNC
            auto operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if<(_MyDimsNum > 1), sub_view>::type
            {
#if YATO_DEBUG
                return (idx < *m_sizes_iter)
                ? sub_view{ m_data_iter + idx * (*std::next(m_offsets_iter)), std::next(m_sizes_iter), std::next(m_offsets_iter) }
            : (YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!"), sub_view{ m_data_iter , m_sizes_iter, m_offsets_iter });
#else
                return sub_view{
                m_data_iter + idx * (*std::next(m_offsets_iter)),
                std::next(m_sizes_iter),
                std::next(m_offsets_iter) };
#endif
            }


            template<size_t _MyDimsNum = _DimsNum>
            YATO_CONSTEXPR_FUNC
            auto operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if <(_MyDimsNum == 1), const reference>::type
            {
#if YATO_DEBUG
                return (idx < *m_sizes_iter)
                    ? *(m_data_iter + idx) :
                    (YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!"), *(m_data_iter));
#else
                return *(m_data_iter + idx);
#endif
            }

            template<size_t _MyDimsNum = _DimsNum>
            auto operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if <(_MyDimsNum == 1), reference>::type
            {
#if YATO_DEBUG
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
#endif
                return *(m_data_iter + idx);
            }


            template<size_t _MyDimsNum = _DimsNum, typename... _IdxTail>
            auto at(size_t idx, _IdxTail... tail) const
                -> typename std::enable_if < (_MyDimsNum > 1), const reference>::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx].at(tail...);
            }

            template<size_t _MyDimsNum = _DimsNum, typename... _IdxTail>
            auto at(size_t idx, _IdxTail... tail)
                -> typename std::enable_if < (_MyDimsNum > 1), reference> ::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx].at(tail...);
            }

            template<size_t _MyDimsNum = _DimsNum>
            auto at(size_t idx) const
                -> typename std::enable_if < _MyDimsNum == 1, const reference>::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return *(m_data_iter + idx);
            }

            template<size_t _MyDimsNum = _DimsNum>
            auto at(size_t idx)
                -> typename std::enable_if < _MyDimsNum == 1, reference>::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return *(m_data_iter + idx);
            }

            /**
            *  Get number of dimensions
            */
            YATO_CONSTEXPR_FUNC
            size_t dimensions() const YATO_NOEXCEPT_KEYWORD
            {
                return dimensions_num;
            }

            /**
            *  Get number of dimensions
            */
            auto dimensions_range() const
                -> yato::range<size_iterator>
            {
                return yato::range<size_iterator>(m_sizes_iter, std::next(m_sizes_iter, dimensions_num));
            }

            /**
             *  Get size along one dimension
             */
            YATO_CONSTEXPR_FUNC
            size_t size(size_t idx) const YATO_NOEXCEPT_IN_RELEASE
            {
#if YATO_DEBUG
                return (idx < dimensions_num)
                    ? *(std::next(m_sizes_iter, idx))
                    : (YATO_THROW_ASSERT_EXCEPT("yato::sub_array_proxy[size]: Dimension index is out of range"), 0);
#else
                return *(std::next(m_sizes_iter, idx));
#endif
            }

            /**
             *  Get total size of multidimensional proxy
             */
            YATO_CONSTEXPR_FUNC
            size_t total_size() const YATO_NOEXCEPT_KEYWORD
            {
                return *m_offsets_iter;
            }

            /**
             *  Get begin iterator for going through arrays of lower dimensionality 
             */
            YATO_CONSTEXPR_FUNC
            const_iterator cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return _create_const_iterator(0);
            }

            /**
             *  Get begin iterator for going through arrays of lower dimensionality
             */
            iterator begin() const YATO_NOEXCEPT_KEYWORD
            {
                return _create_iterator(0);
            }

            /**
             *  Get end iterator for going through arrays of lower dimensionality
             */
            YATO_CONSTEXPR_FUNC
            const_iterator cend() const YATO_NOEXCEPT_KEYWORD
            {
                return _create_const_iterator(size(0));
            }

            /**
             *  Get end iterator for going through arrays of lower dimensionality
             */
            iterator end() const YATO_NOEXCEPT_KEYWORD
            {
                return _create_iterator(size(0));
            }

            /**
             *  Get begin iterator for going through all elements of all dimensions
             */
            YATO_CONSTEXPR_FUNC
            const_data_iterator plain_cbegin() const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter;
            }
            /**
             *  Get begin iterator for going through all elements of all dimensions
             */
            data_iterator plain_begin() YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter;
            }

            /**
             *  Get end iterator for going through all elements of all dimensions
             */
            YATO_CONSTEXPR_FUNC
            const_data_iterator plain_cend() const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter + total_size();
            }

            /**
             *  Get end iterator for going through all elements of all dimensions
             */
            data_iterator plain_end() YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter + total_size();
            }

            /**
             *  Get range of iterators for going through the top dimension
             */
            YATO_CONSTEXPR_FUNC
            yato::range<const_iterator> crange() const YATO_NOEXCEPT_KEYWORD
            {
                return make_range(cbegin(), cend());
            }

            /**
             *  Get range of iterators for going through the top dimension
             */
            yato::range<iterator> range() YATO_NOEXCEPT_KEYWORD
            {
                return make_range(begin(), end());
            }

            /**
             *  Get range of iterators for going through all elements of all dimensions
             */
            YATO_CONSTEXPR_FUNC
            yato::range<const_data_iterator> plain_crange() const YATO_NOEXCEPT_KEYWORD
            {
                return make_range(plain_cbegin(), plain_cend());
            }

            /**
             *  Get range of iterators for going through all elements of all dimensions
             */
            yato::range<data_iterator> plain_range() YATO_NOEXCEPT_KEYWORD
            {
                return make_range(plain_begin(), plain_end());
            }


        };


        template<typename _DataIterator, typename _SizeIterator, size_t _DimsNum>
        YATO_CONSTEXPR_FUNC
        auto proxy_iterator<_DataIterator, _SizeIterator, _DimsNum>::operator*() const YATO_NOEXCEPT_KEYWORD
            -> typename std::enable_if<(_DimsNum > 1), typename proxy_iterator<_DataIterator, _SizeIterator, _DimsNum>::const_sub_proxy>::type
        { 
            return const_sub_proxy(m_data_iter, std::next(m_sizes_iter), std::next(m_offsets_iter));
        }

        template<typename _DataIterator, typename _SizeIterator, size_t _DimsNum>
        inline auto proxy_iterator<_DataIterator, _SizeIterator, _DimsNum>::operator*() YATO_NOEXCEPT_KEYWORD
            -> typename std::enable_if<(_DimsNum > 1), typename proxy_iterator<_DataIterator, _SizeIterator, _DimsNum>::sub_proxy>::type
        {
            return sub_proxy(m_data_iter, std::next(m_sizes_iter), std::next(m_offsets_iter));
        }
    }

#ifdef YATO_MSVC
#pragma warning(pop)
#endif

}

#endif
