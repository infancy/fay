#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_CONFIG_H
#define FAY_CORE_CONFIG_H

#if defined(_DEBUG)
#define FAY_DEBUG
#endif

// platform

#if defined(_WIN32) || defined(_WIN64)
	#define FAY_IN_WINDOWS
	#if defined(_MSC_VER)
		#define FAY_IN_MSVC
	#elif defined(__MINGW32__)  
		#define FAY_IN_MINGW
	#endif
#elif defined(__linux__)
	#define FAY_IN_LINUX
#elif defined(__APPLE__)
	#define FAY_IN_OSX
#endif

/*
#if defined(_MSC_VER) 
	#if _MSC_VER == 1910
	#define FAY_IN_MSVC2017
	#elif _MSC_VER == 1900
	#define FAY_IN_MSVC2015
	#endif
#endif
*/

// platform features

#if defined(FAY_IN_LINUX) || defined(FAY_IN_WINDOWS)
#define FAY_HAVE_MALLOC_H
#endif

#if defined(FAY_IN_MSVC)
#define FAY_THREAD_LOCAL __declspec(thread)
#else
#define FAY_THREAD_LOCAL __thread
#endif

#if defined(FAY_IN_MSVC)
#define FAY_FORCEINLINE __forceinline
#else
#define FAY_FORCEINLINE __attribute__((always_inline)) inline
#endif

#endif // FAY_CORE_CONFIG_H



