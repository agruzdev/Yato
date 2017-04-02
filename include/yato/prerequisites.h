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
# define YATO_MSVC
# if (_MSC_VER == 1800)
#  define YATO_MSVC_2013 
# elif (_MSC_VER == 1900)
#  define YATO_MSVC_2015 
# elif (_MSC_VER == 1910)
#  define YATO_MSVC_2017 
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



#if (defined(_MSC_VER) && (_MSC_VER >= 1900)) || (defined(__cplusplus) && __cplusplus > 201300L)
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
#if (defined(_MSC_VER) && (_MSC_VER > 1900)) || (defined(__cplusplus) && __cplusplus >= 201400L)
#define YATO_HAS_EXTENDED_CONSTEXPR
#define YATO_CONSTEXPR_FUNC_EX constexpr 
#else
#define YATO_CONSTEXPR_FUNC_EX inline
#endif

#if (defined(_MSC_VER) && (_MSC_VER >= 1900)) || (defined(__cplusplus) && (__cplusplus >= 201400L))
# define YATO_HAS_LITERALS
#endif

// ToDo (a.gruzdev): The condition will be made more precise after releasing C++17
#if defined(__cplusplus) && (__cplusplus >= 201700L)
# define YATO_CXX17
#endif

#if defined(__cplusplus) && (__cplusplus >= 201700L)
# define YATO_INLINE_VARIABLE inline
#else
# define YATO_INLINE_VARIABLE
#endif

#ifndef YATO_MSVC_2013
# define YATO_ALIGN(Alignment)  alignas(Alignment)
# define YATO_ALIGN_OF(Type)    alignof(Type)
#else
# define YATO_ALIGN(Alignment)  __declspec(align(Alignment))
# define YATO_ALIGN_OF(Type)    __alignof(Type)
#endif

#define YATO_NOEXCEPT_IN_RELEASE YATO_NOEXCEPT_KEYWORD_EXP(YATO_RELEASE_BOOL)

#define _YATO_QUOTE_IMPL(X) #X
#define YATO_QUOTE(X) _YATO_QUOTE_IMPL(X)

#define YATO_GET_FILE_LINE YATO_QUOTE(__FILE__) ": " YATO_QUOTE(__LINE__)

#define YATO_MAYBE_UNUSED(X) ((void)(X));



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


#endif
