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
	//std::cout << "In function, run for ";
}

int ifunc(int i)
{
	//std::cout << "In function, run for ";
	return i;
}

void vifunc(int i)
{
	//std::cout << "In function, run for ";
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
	time_wrapper<void()> func_wrapper(func);
	time_wrapper<int(int)> ifunc_wrapper(ifunc);
	time_wrapper<void(int)> vifunc_wrapper(vifunc);

	//std::cout << std::boolalpha;
	//std::cout << fay::is_not_void_v<time_wrapper<void()>::R> << '\n';
	//std::cout << fay::is_not_void_v<time_wrapper<int(int), Msec>::R> << '\n';
	//std::cout << fay::is_not_void_v<time_wrapper<void(int)>::R> << '\n';

	func_wrapper();
	std::cout << func_wrapper.count << " ms" << std::endl;

	auto i = ifunc_wrapper(1);
	std::cout << ifunc_wrapper.count << " ms" << std::endl;

	vifunc_wrapper(1);
	std::cout << vifunc_wrapper.count << " ms" << std::endl;
}

