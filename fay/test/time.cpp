#include <iostream>

#include <gtest/gtest.h>
#include <fay/utility/profiler.h>

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
	TimeProfiler<> profiler;
	
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
	TimeWrapper<nullptr_t()> wrapper([]() { func(); return nullptr;});
	TimeWrapper<int(int)> iwrapper(ifunc);
	TimeWrapper<nullptr_t(int)> viwrapper([](int i) { vifunc(i); return nullptr; });

	std::cout << std::boolalpha;
	std::cout << fay::is_not_void_v<TimeWrapper<nullptr_t()>::R> << '\n';
	std::cout << fay::is_not_void_v<TimeWrapper<int(int)>::R> << '\n';
	std::cout << fay::is_not_void_v<TimeWrapper<nullptr_t(int)>::R> << '\n';

	wrapper();
	std::cout << wrapper.count << " ms" << std::endl;

	iwrapper(1);
	std::cout << iwrapper.count << " ms" << std::endl;

	viwrapper(1);
	std::cout << viwrapper.count << " ms" << std::endl;
}

