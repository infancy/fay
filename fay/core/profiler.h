#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_PROFILER_H
#define FAY_CORE_PROFILER_H

#include "fay/core/fay.h"

#include <chrono>
#include <functional>
#include <type_traits>
#include <boost/core/enable_if.hpp>

#include "fay/utility/type_traits.h"

namespace fay
{

// https://segmentfault.com/a/1190000002548499

template<typename Precision = std::chrono::milliseconds>
struct time_profiler
{
	typename Precision::rep count{};

	template<typename Func, typename ...Args, typename R=
		std::enable_if_t<std::is_void<typename std::function<Func>::result_type>::value, 
		void>>
	void call(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();
	}

	template<typename Func, typename ...Args, typename R =
		std::enable_if_t<!std::is_void<typename std::function<Func>::result_type>::value,
		typename std::function<Func>::result_type>>
	R call(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		R ret = func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();

		return ret;
	}
};

// TOOD:SFINAE
template<typename Func, typename Precision = std::chrono::milliseconds, 
	bool VoidReturn = std::is_void<typename std::function<Func>::result_type>::value>
struct time_wrapper {};

template<typename Func, typename Precision>
struct time_wrapper<Func, Precision, true>
{
	using R = typename std::function<Func>::result_type;
	std::function<Func> func{};
	typename Precision::rep count{};

	time_wrapper(std::function<Func>&& func) { wrap(std::move(func)); }

	void wrap(std::function<Func>&& func)
	{
		this->func = func;
	}

	template<typename... Args>
	//std::enable_if_t<std::is_void_v<R>, void>
	void operator()(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();
	}
};

template<typename Func, typename Precision>
struct time_wrapper<Func, Precision, false>
{
	using R = typename std::function<Func>::result_type;
	std::function<Func> func{};
	typename Precision::rep count{};

	time_wrapper(std::function<Func>&& func) { wrap(std::move(func)); }

	void wrap(std::function<Func>&& func)
	{
		this->func = func;
	}

	template<typename... Args>
	//typename std::enable_if<!std::is_void_v<R>, R>::type 
	R operator()(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		R ret = func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();

		return ret;
	}
};

// -----------------------------------------------------------------------------
/*
// milliseconds : ∫¡√Î£¨microseconds: Œ¢√Î£¨nanoseconds : ƒ…√Î
template<typename Precision = std::chrono::milliseconds>
struct time_profiler
{
	typename Precision::rep count{};

	template<typename Func, typename... Args, typename = std::enable_if_t<
		std::is_void_v<typename std::function<Func>::result_type>, void>>
	void operator()(Func func, Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();
	}

	template<typename Func, typename... Args, typename R = std::enable_if_t<
		fay::is_not_void_v<typename std::function<Func>::result_type>, typename std::function<Func>::result_type>>
	R operator()(Func func, Args&&... args, nullptr_t ptr = nullptr)	// trick
	{
		auto start = std::chrono::system_clock::now();

		R ret = func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();

		return ret;
	}
};
*/
// -----------------------------------------------------------------------------

/*
template<typename Func, typename Precision = std::chrono::milliseconds,
	typename Enable = boost::disable_if<std::is_void<
	typename std::function<Func>::result_type>,
	typename std::function<Func>::result_type>::type>
struct time_wrapper
	<Func, Precision, typename std::enable_if_t<fay::is_not_void_v<
	typename std::function<Func>::result_type>, typename std::function<Func>::result_type>>
{
	using R = typename std::function<Func>::result_type;

	std::function<Func> func{};
	typename Precision::rep count{};

	time_wrapper(std::function<Func>&& func) { wrap(std::move(func)); }

	void wrap(std::function<Func>&& func)
	{
		this->func = func;
	}

	// template<typename... Args>
	// typename boost::enable_if<fay::is_not_void<R>, R>::type operator()(Args&&... args)

	// template<typename... Args, typename = std::enable_if_t<fay::is_not_void_v<R>, R>>
	// R operator()(Args&&... args)
	
	// R operator()(Args&&... args, nullptr_t ptr = nullptr)	// trick

	template<typename... Args>
	R operator()(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		R ret = func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();

		return ret;
	}
};

template<typename Func, typename Precision = std::chrono::milliseconds>
struct time_wrapper
{
	static_assert(!std::is_void_v<typename std::function<Func>::result_type>, 
		"time_wrapper: the func can't return void");

	using R = typename std::function<Func>::result_type;

	std::function<Func> func{};
	typename Precision::rep count{};

	time_wrapper(std::function<Func>&& func) { wrap(std::move(func)); }

	void wrap(std::function<Func>&& func)
	{
		this->func = func;
	}

	template<typename... Args>
	R operator()(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		R ret = func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();

		return ret;
	}
};
*/

// milliseconds : ∫¡√Î£¨microseconds: Œ¢√Î£¨nanoseconds : ƒ…√Î

// using Msec = std::chrono::milliseconds;
// TODO : std::invoke
// or TODO: template<typename std::function<void(Args...)>, typename Precision = std::chrono::milliseconds>

//auto timeWrapper = [&](Args&&...) -> R { ... }

// R functor(Args&&... args);
// time_wrapper<R(Args&&...)> wrapper(functor);
// std::function<decltype(wrapper)> func(wrapper);

} // namespace fay

#endif // FAY_CORE_PROFILER_H
