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

		constexpr range(const range<IteratorType>&) = default;
		constexpr range(range<IteratorType>&&) = default;

		constexpr range<IteratorType>& operator=(const range<IteratorType>&) = default;
		constexpr range<IteratorType>& operator=(range<IteratorType>&&) = default;

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


	
}

#endif
