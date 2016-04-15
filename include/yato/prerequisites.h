/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_PREREQUISITES_H_
#define _YATO_PREREQUISITES_H_

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
#define YATO_DEBUG (1)
#define YATO_DEBUG_BOOL true
#define YATO_RELEASE (0)
#define YATO_RELEASE_BOOL false
#else
#define YATO_DEBUG (0)
#define YATO_DEBUG_BOOL false
#define YATO_RELEASE (1)
#define YATO_RELEASE_BOOL true
#endif 
static_assert(YATO_DEBUG != YATO_RELEASE, "Wrong configuration");
static_assert(YATO_DEBUG_BOOL != YATO_RELEASE_BOOL, "Wrong configuration");

#if defined(__x86_64__) || defined(_M_X64) || defined(__ANDROID__)
#define YATO_X64
#elif defined(__i386) || defined(_M_IX86)
#define YATO_X86
#else
#error "Unknown architecture!"
#endif

#ifdef _MSC_VER
#define YATO_MSVC
#endif

#if defined(_MSC_VER) && (_MSC_VER == 1800)
#define YATO_MSVC_2013 
#endif

#if defined(_MSC_VER) && (_MSC_VER == 1900)
#define YATO_MSVC_2015 
#endif

#ifdef __ANDROID__
#define  YATO_ANDROID
#endif

#if defined(YATO_MSVC_2015) || (__cplusplus > 201100L) || (defined(YATO_ANDROID) && __cplusplus > 201300L)
#define YATO_CONSTEXPR_VAR constexpr
#define YATO_CONSTEXPR_FUNC constexpr
#define YATO_NOEXCEPT_KEYWORD noexcept
#define YATO_NOEXCEPT_KEYWORD_EXP(Condition) noexcept(Condition)
#define YATO_NOEXCEPT_OPERATOR(Condition) noexcept(Condition)
#else
#define YATO_CONSTEXPR_VAR const
#define YATO_CONSTEXPR_FUNC inline
#define YATO_NOEXCEPT_KEYWORD throw()
#define YATO_NOEXCEPT_KEYWORD_EXP(Condition)
#define YATO_NOEXCEPT_OPERATOR(Condition) 
#endif

#define YATO_NOEXCEPT_IN_RELEASE YATO_NOEXCEPT_KEYWORD_EXP(YATO_RELEASE_BOOL)




#define _YATO_QUOTE_IMPL(X) #X
#define YATO_QUOTE(X) _YATO_QUOTE_IMPL(X)

#define YATO_GET_FILE_LINE YATO_QUOTE(__FILE__) ": " YATO_QUOTE(__LINE__)

#define YATO_MAYBE_UNUSED(X) ((void)(X));

#endif