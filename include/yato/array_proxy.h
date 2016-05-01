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
        // Multidimensional proxy class
        // Has trait of iterator
        // 

        template<typename _DataIterator, typename _SizeIterator, size_t _DimsNum>
        class sub_array_proxy
        {
            using size_iterator = _SizeIterator;
            using data_iterator = _DataIterator;
            using const_data_iterator = const data_iterator;
            using data_type = typename std::remove_reference< decltype(*std::declval<data_iterator>()) >::type;
            using data_reference = data_type &;
            using const_data_reference = const data_type &;

            static YATO_CONSTEXPR_VAR size_t dimensions_num = _DimsNum;

            using sub_proxy = sub_array_proxy<data_iterator, const size_iterator, dimensions_num - 1>;
            using const_sub_proxy = sub_array_proxy<const data_iterator, const size_iterator, dimensions_num - 1>;

        public:
            using value_type = typename std::conditional<(dimensions_num > 1), sub_proxy, data_type>::type;
            using reference = typename std::conditional<(dimensions_num > 1), sub_proxy, data_type&>::type;
            using pointer = typename std::conditional<(dimensions_num > 1), sub_proxy, data_type*>::type;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::random_access_iterator_tag;

            using iterator = sub_proxy;
            using const_iterator = const_sub_proxy;
            //-------------------------------------------------------

        private:
            data_iterator m_data_iter;
            size_iterator m_sizes_iter;
            size_iterator m_offsets_iter;
            //-------------------------------------------------------

            sub_proxy _create_sub_proxy(size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                return sub_proxy(std::next(m_data_iter, offset * m_offsets_iter[1]), std::next(m_sizes_iter), std::next(m_offsets_iter));
            }

            YATO_CONSTEXPR_FUNC
            const_sub_proxy _create_const_sub_proxy(size_t offset) const YATO_NOEXCEPT_KEYWORD
            {
                return const_sub_proxy(std::next(m_data_iter, offset * m_offsets_iter[1]), std::next(m_sizes_iter), std::next(m_offsets_iter));
            }
            //-------------------------------------------------------

        public:
            YATO_CONSTEXPR_FUNC
            sub_array_proxy(const data_iterator & data, const size_iterator & sizes, const size_iterator & offsets) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(data), m_sizes_iter(sizes), m_offsets_iter(offsets)
            {}

            sub_array_proxy(const sub_array_proxy<data_iterator, size_iterator, dimensions_num> & other)
                : m_data_iter(other.m_data_iter), m_sizes_iter(other.m_sizes_iter), m_offsets_iter(other.m_offsets_iter)
            { }

            template <typename _OtherDataIterator>
            sub_array_proxy(const sub_array_proxy<_OtherDataIterator, size_iterator, dimensions_num> & other)
                : m_data_iter(other.m_data_iter), m_sizes_iter(other.m_sizes_iter), m_offsets_iter(other.m_offsets_iter)
            {}

            sub_array_proxy(sub_array_proxy && other) YATO_NOEXCEPT_KEYWORD
                : m_data_iter(std::move(other.m_data_iter)), m_sizes_iter(std::move(other.m_sizes_iter)), m_offsets_iter(std::move(other.m_offsets_iter))
            {}

            sub_array_proxy & operator= (const sub_array_proxy & other)
            {
                if (this != &other){
                    m_data_iter = other.m_data_iter;
                    m_sizes_iter = other.m_sizes_iter;
                    m_offsets_iter = other.m_offsets_iter;
                }
                return *this;
            }

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
                -> typename std::enable_if<(_MyDimsNum > 1), const_sub_proxy>::type
            {
#if YATO_DEBUG
                return (idx < *m_sizes_iter)
                    ? _create_const_sub_proxy(idx)
                    : (YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!"), _create_const_sub_proxy(0));
#else
                return _create_const_sub_proxy(idx);
#endif
            }

            template<size_t _MyDimsNum = _DimsNum>
            auto operator[](size_t idx) YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if<(_MyDimsNum > 1), sub_proxy>::type
            {
#if YATO_DEBUG
                return (idx < *m_sizes_iter)
                ? _create_sub_proxy(idx)
                : (YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!"), _create_sub_proxy(0));
#else
                return _create_sub_proxy(idx);
#endif
            }

            template<size_t _MyDimsNum = _DimsNum>
            YATO_CONSTEXPR_FUNC
            auto operator[](size_t idx) const YATO_NOEXCEPT_IN_RELEASE
                -> typename std::enable_if <(_MyDimsNum == 1), const reference>::type
            {
#if YATO_DEBUG
                return (idx < *m_sizes_iter)
                    ? *(m_data_iter + idx)
                    : (YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!"), *(m_data_iter));
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
                -> typename std::enable_if < (_MyDimsNum > 1), const_data_reference>::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx].at(tail...);
            }

            template<size_t _MyDimsNum = _DimsNum, typename... _IdxTail>
            auto at(size_t idx, _IdxTail... tail)
                -> typename std::enable_if < (_MyDimsNum > 1), data_reference> ::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx].at(tail...);
            }

            template<size_t _MyDimsNum = _DimsNum>
            auto at(size_t idx) const
                -> typename std::enable_if < _MyDimsNum == 1, const_data_reference>::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx];
            }

            template<size_t _MyDimsNum = _DimsNum>
            auto at(size_t idx)
                -> typename std::enable_if < _MyDimsNum == 1, data_reference>::type
            {
                if (idx >= *m_sizes_iter) {
                    YATO_THROW_ASSERT_EXCEPT("yato::array_sub_view_nd: out of range!");
                }
                return (*this)[idx];
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
            template<size_t _MyDimsNum = _DimsNum>
            YATO_CONSTEXPR_FUNC
            auto cbegin() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_MyDimsNum > 1), const_iterator>::type
            {
                return _create_const_sub_proxy(0);
            }

            /**
             *  Get begin iterator for going through arrays of lower dimensionality
             */
            template<size_t _MyDimsNum = _DimsNum>
            YATO_CONSTEXPR_FUNC
            auto cbegin() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if < (_MyDimsNum == 1), const_data_iterator > ::type
            {
                return plain_cbegin();
            }

            /**
             *  Get begin iterator for going through arrays of lower dimensionality
             */
            template<size_t _MyDimsNum = _DimsNum>
            auto begin() YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_MyDimsNum > 1), iterator>::type
            {
                return _create_sub_proxy(0);
            }

            /**
            *  Get begin iterator for going through arrays of lower dimensionality
            */
            template<size_t _MyDimsNum = _DimsNum>
            auto begin() YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_MyDimsNum == 1), data_iterator>::type
            {
                return plain_begin();
            }

            /**
             *  Get end iterator for going through arrays of lower dimensionality
             */
            template<size_t _MyDimsNum = _DimsNum>
            YATO_CONSTEXPR_FUNC
            auto cend() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_MyDimsNum > 1), const_iterator>::type
            {
                return _create_const_sub_proxy(size(0));
            }

            /**
             *  Get end iterator for going through arrays of lower dimensionality
             */
            template<size_t _MyDimsNum = _DimsNum>
            YATO_CONSTEXPR_FUNC
            auto cend() const YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_MyDimsNum == 1), const_data_iterator>::type
            {
                return plain_cend();
            }

            /**
             *  Get end iterator for going through arrays of lower dimensionality
             */
            template<size_t _MyDimsNum = _DimsNum>
            auto end() YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_MyDimsNum > 1), iterator>::type
            {
                return _create_sub_proxy(size(0));
            }

            /**
            *  Get end iterator for going through arrays of lower dimensionality
            */
            template<size_t _MyDimsNum = _DimsNum>
            auto end() YATO_NOEXCEPT_KEYWORD
                -> typename std::enable_if<(_MyDimsNum == 1), data_iterator>::type
            {
                return plain_end();
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

            /**
             *  Return the current proxy
             */
            YATO_CONSTEXPR_FUNC
            const sub_array_proxy& operator* () const
            {
                return *this;
            }

            /**
             *  Return the current proxy
             */
            sub_array_proxy& operator* ()
            {
                return *this;
            }

            /**
             *  Increment iterator
             */
            sub_array_proxy& operator++() YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter += m_offsets_iter[0];
                return *this;
            }

            /**
             *  Increment iterator
             */
            sub_array_proxy& operator++(int)
            {
                auto temp = *this;
                ++(*this);
                return temp;
            }

            /**
             *  Decrement iterator
             */
            sub_array_proxy& operator--() YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter -= m_offsets_iter[0];
                return *this;
            }

            /**
             *  Decrement iterator
             */
            sub_array_proxy& operator--(int)
            {
                auto temp = *this;
                --(*this);
                return temp;
            }

            /**
             *  Add offset
             */
            sub_array_proxy& operator+= (size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter += offset * m_offsets_iter[0];
                return *this;
            }

            /**
             *  Add offset
             */
            sub_array_proxy operator+ (size_t offset) const
            {
                auto temp = *this;
                return (temp += offset);
            }

            /**
             *  Add offset
             */
            friend
            sub_array_proxy operator+ (size_t offset, const sub_array_proxy& iter)
            {
                return iter + offset;
            }

            /**
             *  Subtract offset
             */
            sub_array_proxy& operator-= (size_t offset) YATO_NOEXCEPT_KEYWORD
            {
                m_data_iter -= offset * m_offsets_iter[0];
                return *this;
            }

            /**
             *  Subtract offset
             */
            sub_array_proxy operator- (size_t offset) const
            {
                auto temp = *this;
                return (temp -= offset);
            }

            /**
             *  Distance between iterators
             */
            difference_type operator- (const sub_array_proxy& other)
            {
                return (m_data_iter - other.m_data_iter) / m_offsets_iter[0];
            }

            /**
             *  Equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator== (const sub_array_proxy& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter == other.m_data_iter;
            }

            /**
             *  Not equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator!= (const sub_array_proxy& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter != other.m_data_iter;
            }

            /**
             *  Less
             */
            YATO_CONSTEXPR_FUNC
            bool operator< (const sub_array_proxy& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter < other.m_data_iter;
            }

            /**
             *  Greater
             */
            YATO_CONSTEXPR_FUNC
            bool operator> (const sub_array_proxy& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter > other.m_data_iter;
            }

            /**
             *  Less or equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator<= (const sub_array_proxy& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter <= other.m_data_iter;
            }

            /**
             *  Greater or equal
             */
            YATO_CONSTEXPR_FUNC
            bool operator>= (const sub_array_proxy& other) const YATO_NOEXCEPT_KEYWORD
            {
                return m_data_iter >= other.m_data_iter;
            }

            //-------------------------------------------------------
            
            template<typename _OtherDataIterator, typename _OtherSizeIterator, size_t _OtherDimensions>
            friend class sub_array_proxy;
        };

    }

#ifdef YATO_MSVC
#pragma warning(pop)
#endif

}

#endif
