/**
 * YATO library
 *
 * The MIT License (MIT)
 * Copyright (c) 2016 Alexey Gruzdev
 */

#ifndef _YATO_TYPES_H_
#define _YATO_TYPES_H_

#include <cstdint>
#include <limits>

#include "prerequisites.h"
#include "assert.h"

namespace yato
{
#ifndef INT8_C
	using int8_t = signed char;
#else
	using int8_t = ::int8_t;
#endif
	static_assert(sizeof(int8_t) == 1, "Wrong int8_t type!");

#ifndef UINT8_C 
	using uint8_t = unsigned char;
#else
	using uint8_t = ::uint8_t;
#endif
	static_assert(sizeof(uint8_t) == 1, "Wrong uint8_t type!");

#ifndef INT16_C 
	using int16_t = signed short;
#else
	using int16_t = ::int16_t;
#endif
	static_assert(sizeof(int16_t) == 2, "Wrong int16_t type!");

#ifndef UINT16_C 
	using uint16_t = unsigned short;
#else
	using uint16_t = ::uint16_t;
#endif
	static_assert(sizeof(uint16_t) == 2, "Wrong uint16_t type!");

#ifndef INT32_C 
	using int32_t = signed int;
#else
	using int32_t = ::int32_t;
#endif
	static_assert(sizeof(int32_t) == 4, "Wrong int32_t type!");

#ifndef UINT32_C 
	using uint32_t = unsigned int;
#else
	using uint32_t = ::uint32_t;
#endif
	static_assert(sizeof(uint32_t) == 4, "Wrong uint32_t type!");

#ifndef INT64_C 
	using int64_t = signed long long;
#else
	using int64_t = ::int64_t;
#endif
	static_assert(sizeof(int64_t) == 8, "Wrong int64_t type!");

#ifndef UINT64_C 
	using uint64_t = unsigned long long;
#else
	using uint64_t = ::uint64_t;
#endif
	static_assert(sizeof(uint64_t) == 8, "Wrong uint64_t type!");


	using float32_t = float;
	static_assert(sizeof(float32_t) == 4, "Wrong float32_t type!");

	using float64_t = double;
	static_assert(sizeof(float64_t) == 8, "Wrong float32_t type!");


	template<typename _T_Dst, typename _T_Src>
	constexpr typename std::enable_if<
		std::is_arithmetic<typename std::decay<_T_Dst>::type>::value && 
		std::is_arithmetic<typename std::decay<_T_Src>::type>::value, 
	_T_Dst>::type
		narrow_cast(_T_Src && val) noexcept(YATO_RELEASE_BOOL) {
#if YATO_DEBUG
		return static_cast<typename std::decay<_T_Src>::type>(static_cast<_T_Dst>(val)) == val ? static_cast<_T_Dst>(val)
			: (YATO_THROW_ASSERT_EXCEPT("narrow_cast failed!"), static_cast<_T_Dst>(0));
#else
		return static_cast<_T_Dst>(val);
#endif
	}

	namespace literals
	{
		constexpr int8_t operator"" _s8(unsigned long long number) noexcept(noexcept(narrow_cast<int8_t>(number))) {
			return narrow_cast<int8_t>(number);
		}

		constexpr uint8_t operator"" _u8(unsigned long long number) noexcept(noexcept(narrow_cast<uint8_t>(number))) {
			return narrow_cast<uint8_t>(number);
		}

		constexpr int16_t operator"" _s16(unsigned long long number) noexcept(noexcept(narrow_cast<int16_t>(number))) {
			return static_cast<int16_t>(number);
		}

		constexpr uint16_t operator"" _u16(unsigned long long number) noexcept(noexcept(narrow_cast<uint16_t>(number))) {
			return static_cast<uint16_t>(number);
		}

		constexpr int32_t operator"" _s32(unsigned long long number) noexcept(noexcept(narrow_cast<int32_t>(number))) {
			return static_cast<int32_t>(number);
		}

		constexpr uint32_t operator"" _u32(unsigned long long number) noexcept(noexcept(narrow_cast<uint32_t>(number))) {
			return static_cast<uint32_t>(number);
		}

		constexpr int64_t operator"" _s64(unsigned long long number) noexcept(noexcept(narrow_cast<int64_t>(number))) {
			return static_cast<int64_t>(number);
		}

		constexpr uint64_t operator"" _u64(unsigned long long number) noexcept(noexcept(narrow_cast<uint64_t>(number))) {
			return static_cast<uint64_t>(number);
		}

		constexpr float32_t operator"" _f32(long double number) noexcept(noexcept(narrow_cast<float32_t>(number))) {
			return static_cast<float32_t>(number);
		}

		constexpr float64_t operator"" _f64(long double number) noexcept(noexcept(narrow_cast<float64_t>(number))) {
			return static_cast<float64_t>(number);
		}
	}
}



#endif