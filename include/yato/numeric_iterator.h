/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_NUMERIC_ITERATOR_H_
#define _YATO_NUMERIC_ITERATOR_H_

#include "assert.h"
#include "type_traits.h"

namespace yato
{
	
	/**
	 *	Iterator for passing through a 'virtual' integer sequence
	 *  Enumerates without pointing to any container
	 */
	template <typename _T>
	class numeric_iterator
	{
		static_assert(std::is_integral<_T>::value, "numeric_iterator can hold only integral types");
	public:
		using this_type = numeric_iterator<_T>;

		using value_type = _T;
		using difference_type = std::ptrdiff_t;
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

		constexpr numeric_iterator(const numeric_iterator & other) noexcept
			: m_value(other.m_value)
		{ }

		numeric_iterator(numeric_iterator&&) noexcept = default;

		numeric_iterator& operator=(const numeric_iterator&) noexcept = default;
		numeric_iterator& operator=(numeric_iterator&&) noexcept = default;

		~numeric_iterator() noexcept
		{ }

		constexpr reference_to_const operator*() const noexcept {
			return m_value;
		}

		constexpr pointer_to_const operator->() const noexcept {
			return &m_value;
		}

		this_type & operator++() {
			YATO_ASSERT(m_value < std::numeric_limits<value_type>::max(), "yato::numeric_iterator is out of range");
			++m_value;
			return *this;
		}

		this_type & operator++(int) {
			YATO_ASSERT(m_value < std::numeric_limits<value_type>::max(), "yato::numeric_iterator is out of range");
			auto temp = *this;
			++m_value;
			return temp;
		}

		this_type & operator--() {
			YATO_ASSERT(m_value > std::numeric_limits<value_type>::min(), "yato::numeric_iterator is out of range");
			--m_value;
			return *this;
		}

		this_type & operator--(int) {
			YATO_ASSERT(m_value > std::numeric_limits<value_type>::min(), "yato::numeric_iterator is out of range");
			auto temp = *this;
			--m_value;
			return temp;
		}

		this_type & operator+=(difference_type offset) {
			YATO_ASSERT(m_value <= std::numeric_limits<value_type>::max() - yato::narrow_cast<value_type>(offset), "yato::numeric_iterator is out of range");
			m_value += yato::narrow_cast<value_type>(offset);
			return *this;
		}

		this_type operator+(difference_type offset) const {	
			this_type tmp = *this;
			return (tmp += offset);
		}

		this_type & operator-=(difference_type offset) {
			YATO_ASSERT(m_value >= std::numeric_limits<value_type>::min() + yato::narrow_cast<value_type>(offset), "yato::numeric_iterator is out of range");
			m_value -= yato::narrow_cast<value_type>(offset);
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
