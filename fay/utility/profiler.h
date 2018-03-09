#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_UTILITY_PROFILER_H
#define FAY_UTILITY_PROFILER_H

#include "fay/utility/fay.h"

#include <chrono>
#include <functional>
#include <type_traits>
#include <boost/core/enable_if.hpp>

#include "fay/utility/type_traits.h"

namespace fay
{

// https://segmentfault.com/a/1190000002548499

template<typename TimeT = std::chrono::milliseconds>
struct measure
{
	template<typename F, typename ...Args>
	static typename TimeT::rep execution(F func, Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		// Now call the function with all the parameters you need.
		func(std::forward<Args>(args)...);

		auto duration = std::chrono::duration_cast< TimeT>
			(std::chrono::system_clock::now() - start);

		return duration.count();
	}
};

// -----------------------------------------------------------------------------
/*
// milliseconds : ∫¡√Î£¨microseconds: Œ¢√Î£¨nanoseconds : ƒ…√Î
template<typename Precision = std::chrono::milliseconds>
struct TimeProfiler
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
	typename Enable = std::enable_if_t<std::is_void_v<
	typename std::function<Func>::result_type>, 
	typename std::function<Func>::result_type>>
struct TimeWrapper
{
	std::function<Func> func{};
	typename Precision::rep count{};

	TimeWrapper(std::function<Func>&& func) { wrap(std::move(func)); }

	void wrap(std::function<Func>&& func)
	{
		this->func = func;
	}
	
	template<typename... Args>
	void operator()(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();
	}
};

template<typename Func, typename Precision = std::chrono::milliseconds,
	typename Enable = boost::disable_if<std::is_void<
	typename std::function<Func>::result_type>,
	typename std::function<Func>::result_type>::type>
struct TimeWrapper
//	<Func, Precision, typename std::enable_if_t<fay::is_not_void_v<
//	typename std::function<Func>::result_type>, typename std::function<Func>::result_type>>
{
	using R = typename std::function<Func>::result_type;

	std::function<Func> func{};
	typename Precision::rep count{};

	TimeWrapper(std::function<Func>&& func) { wrap(std::move(func)); }

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
struct TimeWrapper
{
	using FuncR = typename std::function<Func>::result_type;
	using R = std::conditional_t<std::is_void_v<FuncR>, nullptr_t, FuncR>;

	bool has_ret = std::is_void_v<FuncR>;
	std::function<Func> func{};
	typename Precision::rep count{};

	TimeWrapper(std::function<Func>&& func) { wrap(std::move(func)); }

	void wrap(std::function<Func>&& func)
	{
		this->func = func;
	}

	template<typename... Args>
	R operator()(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		R ret{};

		if constexpr(std::is_void_v<FuncR>)
			ret = func(std::forward<Args>(args)...);
		//else
		//	ret = func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();

		return ret;
	}
};
*/

// milliseconds : ∫¡√Î£¨microseconds: Œ¢√Î£¨nanoseconds : ƒ…√Î

using Msec = std::chrono::milliseconds;

template<typename Func, typename Precision = std::chrono::milliseconds>
struct TimeWrapper
{
	using R = typename std::function<Func>::result_type;

	std::function<Func> func{};
	typename Precision::rep count{};

	TimeWrapper(std::function<Func>&& func) { wrap(std::move(func)); }

	void wrap(std::function<Func>&& func)
	{
		this->func = func;
	}

	template<typename... Args>
	//std::enable_if_t<std::is_void_v<R>, void>
	void void_call(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();
	}

	template<typename... Args>
	//typename std::enable_if<!std::is_void_v<R>, R>::type 
	R call(Args&&... args)
	{
		auto start = std::chrono::system_clock::now();

		R ret = func(std::forward<Args>(args)...);

		count = (std::chrono::system_clock::now() - start).count();

		return ret;
	}
};

/*
template<typename Func, typename Precision = std::chrono::milliseconds>
struct TimeWrapper
{
	static_assert(!std::is_void_v<typename std::function<Func>::result_type>, 
		"TimeWrapper: the func can't return void");

	using R = typename std::function<Func>::result_type;

	std::function<Func> func{};
	typename Precision::rep count{};

	TimeWrapper(std::function<Func>&& func) { wrap(std::move(func)); }

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
// R functor(Args&&... args);
// TimeWrapper<R(Args&&...)> wrapper(functor);
// std::function<decltype(wrapper)> func(wrapper);

} // namespace fay

#endif // FAY_UTILITY_PROFILER_H
