/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_RANGE_H_
#define _YATO_RANGE_H_

#include <vector>

#include "type_traits.h"

namespace yato
{
	/**
	 *	An immutable object aggregating two iterators
	 *  Helps to express a range of one container
	 */
	template<typename IteratorType>
	class range
	{
		static_assert(is_iterator<IteratorType>::value, "yato::range can be used only for iterators");

		const IteratorType _begin;
		const IteratorType _end;
	public:
		constexpr range(const IteratorType & begin, const IteratorType & end) noexcept
			: _begin(begin), _end(end)
		{ }

		constexpr range(IteratorType && begin, IteratorType && end) noexcept
			: _begin(begin), _end(end)
		{ }

		constexpr range(const range<IteratorType>&) noexcept = default;
		constexpr range(range<IteratorType>&&) noexcept = default;

		constexpr range<IteratorType>& operator=(const range<IteratorType>&) noexcept = default;
		constexpr range<IteratorType>& operator=(range<IteratorType>&&) noexcept = default;

		~range() noexcept
		{ }

		constexpr const IteratorType & begin() const noexcept {
			return _begin;
		}

		constexpr const IteratorType & end() const noexcept {
			return _end;
		}

		constexpr auto size() {
			return std::distance(_begin, _end);
		}

		constexpr bool empty() noexcept {
			return !(_begin != _end);
		}

		constexpr const IteratorType & head() const noexcept {
			return _begin;
		}

		constexpr range<IteratorType> tail() const {
			return range<IteratorType>(std::next(_begin), _end);
		}
	};

	template<typename IteratorType>
	constexpr typename std::enable_if< is_iterator< typename std::decay<IteratorType>::type >::value, range< typename std::decay<IteratorType>::type > >::type 
		make_range(const IteratorType & begin, const IteratorType & end)
	{
		return range<typename std::decay<IteratorType>::type>(begin, end);
	}

	template<typename IteratorType>
	constexpr typename std::enable_if< is_iterator< typename std::decay<IteratorType>::type >::value, range< typename std::decay<IteratorType>::type > >::type
		make_range(IteratorType && begin, IteratorType && end)
	{
		return range<typename std::decay<IteratorType>::type>(begin, end);
	}


	//-------------------------------------------------------
	
	template <typename _T>
	class numeric_iterator
	{
		static_assert(std::is_integral<_T>::value, "numeric_iterator can hold only integral types");
	public:
		using this_type = numeric_iterator<_T>;

		using value_type = _T;
		using difference_type = _T;
		using pointer = typename std::add_pointer<_T>::type;
		using pointer_to_const = typename std::add_pointer<const _T>::type;
		using reference = typename std::add_lvalue_reference<_T>::type;
		using reference_to_const = typename std::add_lvalue_reference<const _T>::type;
		using iterator_category = std::random_access_iterator_tag;

	private:
		value_type m_value;

	public:
		constexpr numeric_iterator(value_type value) noexcept
			: m_value(value)
		{ }

		constexpr numeric_iterator(const numeric_iterator&) noexcept = default;
		constexpr numeric_iterator(numeric_iterator&&) noexcept = default;

		constexpr numeric_iterator& operator=(const numeric_iterator&) noexcept = default;
		constexpr numeric_iterator& operator=(numeric_iterator&&) noexcept = default;

		~numeric_iterator() noexcept
		{ }

		constexpr reference_to_const operator*() const noexcept {
			return m_value;
		}

		constexpr pointer_to_const operator->() const noexcept {
			return &m_value;
		}

		constexpr this_type & operator++() {
			++m_value;
			return *this;
		}

		constexpr this_type & operator++(int) {
			auto temp = *this;
			++m_value;
			return temp;
		}

		constexpr this_type & operator--() {
			--m_value;
			return *this;
		}

		constexpr this_type & operator--(int) {
			auto temp = *this;
			--m_value;
			return temp;
		}

		constexpr this_type & operator+=(difference_type offset) {
			m_value += offset;
			return *this;
		}

		constexpr this_type operator+(difference_type offset) const {	
			this_type tmp = *this;
			return (tmp += offset);
		}

		constexpr this_type & operator-=(difference_type offset) {
			m_value -= offset;
			return *this;
		}

		constexpr this_type operator-(difference_type offset) const {
			this_type tmp = *this;
			return (tmp -= offset);
		}

		constexpr difference_type operator-(const this_type & right) const {
			return m_value - right.m_value;
		}

		constexpr reference_to_const operator[](difference_type offset) const {
			return (*(*this + offset));
		}

		constexpr bool operator!=(const this_type & other) const noexcept {
			return m_value != other.m_value;
		}

		constexpr bool operator==(const this_type & other) const noexcept {
			return m_value == other.m_value;
		}

		constexpr bool operator<(const this_type & right) const noexcept {
			return m_value < right.m_value;
		}

		constexpr bool operator>(const this_type & right) const noexcept {
			return m_value > right.m_value;
		}

		constexpr bool operator<=(const this_type & right) const noexcept {
			return m_value <= right.m_value;
		}

		constexpr bool operator>=(const this_type & right) const noexcept {
			return m_value >= right.m_value;
		}
	};

	

}

#endif
