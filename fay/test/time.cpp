#include <iostream>

#include <gtest/gtest.h>
#include <fay/core/profiler.h>

using namespace fay;

// -----------------------------------------------------------------------------

struct functor
{
	int state;
	functor(int state) : state(state) {}
	void operator()() const
	{
		std::cout << "In functor run for ";
	}
};

void func()
{
	std::cout << "In function, run for ";
}

int ifunc(int i)
{
	std::cout << "In function, run for ";
	return i;
}

void vifunc(int i)
{
	std::cout << i << " In function, run for ";
}
/*
TEST(profiler, time_profiler)
{
	time_profiler<> profiler;
	
	profiler([]() { std::cout << "In lambda run for "; });
	std::cout << profiler.count << " ms" << std::endl;

	profiler(functor(3));
	std::cout << profiler.count << " ms" << std::endl;

	profiler(func);
	std::cout << profiler.count << " ms" << std::endl;
}
*/
TEST(profiler, time_wrapper)
{
	time_wrapper<void()> wrapper(func);
	time_wrapper<int(int)> iwrapper(ifunc);
	time_wrapper<void(int)> viwrapper(vifunc);

	//std::cout << std::boolalpha;
	//std::cout << fay::is_not_void_v<time_wrapper<void()>::R> << '\n';
	//std::cout << fay::is_not_void_v<time_wrapper<int(int), Msec>::R> << '\n';
	//std::cout << fay::is_not_void_v<time_wrapper<void(int)>::R> << '\n';

	// std::function<void()> func = std::bind(&time_wrapper<void()>::void_call, wrapper);

	wrapper.void_call();
	std::cout << wrapper.count << " ms" << std::endl;

	iwrapper.call(1);
	std::cout << iwrapper.count << " ms" << std::endl;

	viwrapper.void_call(1);
	std::cout << viwrapper.count << " ms" << std::endl;
}

