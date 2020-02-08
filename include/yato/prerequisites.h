/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_PREREQUISITES_H_
#define _YATO_PREREQUISITES_H_

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
#define YATO_DEBUG (1)
#else
#define YATO_DEBUG (0)
#endif 


#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
#define YATO_X64
#elif defined(__i386) || defined(_M_IX86) || defined(__ANDROID__)
#define YATO_X86
#else
#error "Unknown architecture!"
#endif


#ifdef _MSC_VER
# if (_MSC_VER >= 1920)
#  define YATO_MSVC_2019
#  define YATO_MSVC 16
# elif (_MSC_VER >= 1910)
#  define YATO_MSVC_2017
#  define YATO_MSVC 15
# elif (_MSC_VER >= 1900)
#  define YATO_MSVC_2015
#  define YATO_MSVC 14
# elif (_MSC_VER >= 1800)
#  define YATO_MSVC 12
#  define YATO_MSVC_2013
# else
#  define YATO_MSVC 1
# endif
#endif

#ifdef __GNUC__
# if defined(__clang__)
#  define YATO_CLANG
# elif defined(__MINGW32__)
#  define YATO_MINGW
# else
#  define YATO_GCC
# endif
#endif

#ifdef __ANDROID__
# define YATO_ANDROID
#endif



#if (defined(YATO_MSVC) && (YATO_MSVC >= 14)) || (defined(__cplusplus) && __cplusplus > 201300L)
#define YATO_CONSTEXPR_VAR constexpr
#define YATO_CONSTEXPR_FUNC constexpr
#define YATO_NOEXCEPT_KEYWORD noexcept
#define YATO_NOEXCEPT_KEYWORD_EXP(Condition) noexcept(Condition)
#define YATO_NOEXCEPT_OPERATOR(Condition) noexcept(Condition)
#define YATO_DEPRECATED(Reason) [[deprecated(Reason)]]
#define YATO_NORETURN [[noreturn]]
#else
#define YATO_CONSTEXPR_VAR const
#define YATO_CONSTEXPR_FUNC inline
#define YATO_NOEXCEPT_KEYWORD throw()
#define YATO_NOEXCEPT_KEYWORD_EXP(Condition)
#define YATO_NOEXCEPT_OPERATOR(Condition)
#define YATO_DEPRECATED(Reason)
#define YATO_NORETURN
#endif

// Extended constexpr
#if (defined(YATO_MSVC) && (YATO_MSVC >= 15)) || (defined(__cplusplus) && (__cplusplus >= 201400L))
#define YATO_HAS_CONSTEXPR_CXX14
#define YATO_CONSTEXPR_FUNC_CXX14 constexpr 
#else
#define YATO_CONSTEXPR_FUNC_CXX14 inline
#endif

#if (defined(YATO_MSVC) && (YATO_MSVC >= 14)) || (defined(__cplusplus) && (__cplusplus >= 201400L))
# define YATO_HAS_LITERALS
#endif

// ToDo (a.gruzdev): Change to HAS_FEATURE flag
#if defined(__cplusplus) && (__cplusplus >= 201700L)
# define YATO_CXX17
#endif

#if (defined(__cplusplus) && (__cplusplus >= 201700L))
# define YATO_INLINE_VARIABLE inline
# define YATO_ATTR_FALLTHROUGH fallthrough
#else
# define YATO_INLINE_VARIABLE
# define YATO_ATTR_FALLTHROUGH
#endif

// ToDo (a.gruzdev): Need to check condition more carefully
#if (defined(YATO_GCC) || defined(YATO_MINGW)) && (defined(__cplusplus) && (__cplusplus >= 201700L))
# define YATO_HAS_LAUNDER
#endif

#if (defined(YATO_MSVC) && (YATO_MSVC >= 14)) || (defined(__cplusplus) && (__cplusplus > 201300L))
# define YATO_ALIGN(Alignment)  alignas(Alignment)
# define YATO_ALIGN_OF(Type)    alignof(Type)
#else
# define YATO_ALIGN(Alignment)  __declspec(align(Alignment))
# define YATO_ALIGN_OF(Type)    __alignof(Type)
#endif

#define _YATO_QUOTE_IMPL(X) #X
#define YATO_QUOTE(X) _YATO_QUOTE_IMPL(X)

#define YATO_GET_FILE_LINE YATO_QUOTE(__FILE__) ": " YATO_QUOTE(__LINE__)

#define YATO_MAYBE_UNUSED(X) ((void)(X));


#define _YATO_JOIN(X, Y) X##Y
#define _YATO_JOIN2(X, Y) _YATO_JOIN(X, Y)
#define YATO_UNIQUE_NAME(Prefix_) _YATO_JOIN2(Prefix_, __COUNTER__)


#if defined(YATO_MSVC)
# define YATO_PRAGMA_WARNING_PUSH __pragma(warning(push))
# define YATO_PRAGMA_WARNING_POP  __pragma(warning(pop))
# define YATO_MSCV_WARNING_IGNORE(Number) __pragma(warning( disable: Number ))
# define YATO_CLANG_WARNING_IGNORE(X)
# define YATO_GCC_WARNING_IGNORE(Flag)
#elif defined(YATO_CLANG)
# define YATO_PRAGMA_WARNING_PUSH _Pragma(_YATO_QUOTE_IMPL(clang diagnostic push))
# define YATO_PRAGMA_WARNING_POP _Pragma(_YATO_QUOTE_IMPL(clang diagnostic pop))
# define YATO_MSCV_WARNING_IGNORE(X) 
# define YATO_CLANG_WARNING_IGNORE(Flag) _Pragma(_YATO_QUOTE_IMPL(clang diagnostic ignored Flag))
# define YATO_GCC_WARNING_IGNORE(Flag)
#elif defined(YATO_GCC) || defined(YATO_MINGW)
# define YATO_PRAGMA_WARNING_PUSH _Pragma(_YATO_QUOTE_IMPL(GCC diagnostic push))
# define YATO_PRAGMA_WARNING_POP _Pragma(_YATO_QUOTE_IMPL(GCC diagnostic pop))
# define YATO_MSCV_WARNING_IGNORE(X)
# define YATO_CLANG_WARNING_IGNORE(Flag)
# define YATO_GCC_WARNING_IGNORE(Flag) _Pragma(_YATO_QUOTE_IMPL(GCC diagnostic ignored Flag))
#else
# define YATO_PRAGMA_WARNING_PUSH 
# define YATO_PRAGMA_WARNING_POP 
# define YATO_MSCV_WARNING_IGNORE(X) 
# define YATO_CLANG_WARNING_IGNORE(X) 
# define YATO_GCC_WARNING_IGNORE(Flag)
#endif

#ifdef _WIN32
# define YATO_EXPORT_ATTRIBUTE __declspec(dllexport)
# define YATO_IMPORT_ATTRIBUTE __declspec(dllimport)
#else
# define YATO_EXPORT_ATTRIBUTE __attribute__((visibility("default")))
# define YATO_IMPORT_ATTRIBUTE 
#endif


#if defined(YATO_MSVC)
# define YATO_FORCED_INLINE __forceinline
#elif defined(YATO_MINGW) || defined(YATO_GCC) || defined(YATO_CLANG)
# define YATO_FORCED_INLINE inline __attribute__((__always_inline__))
#else
# define YATO_FORCED_INLINE inline
#endif

#endif //_YATO_PREREQUISITES_H_
