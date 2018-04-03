#include <atomic>
#include <numeric>
#include <future>
#include <list>

#include <gtest/gtest.h>

#include <fay/core/parallel.hpp>
#include <fay/core/thread_pool.hpp>

using namespace fay;

// -----------------------------------------------------------------------------

/*
TEST(concurrency, test_thread_pool)
{
	thread_pool pool(16);

	auto task = [&pool]() 
	{
		auto id = std::this_thread::get_id();
		for (int i = 0; i < 16; ++i)
			pool.add([id]() { std::cout << "sync thread id: " << id << std::endl; });
	};

	std::async(task);
	std::async(task);
}
*/

// -----------------------------------------------------------------------------

TEST(parallel, parallel_)
{
	std::atomic<int> ai{512};

	std::vector<int> v(1024, 0);

	tbb::parallel_do(v.begin(), v.end(), [&ai](/*auto*/int& i) { --ai; i += ai; });
	tbb::parallel_for_each(v.begin(), v.end(), [&ai](int& i)   { ++ai; i += ai; });

	fay::parallel_for(v.begin(), v.end(), 
		[](auto& range)	// tbb::blocked_range<decltype(v.cbegin())>&
	{ 
		int num{};
		for (auto i : range)
			num += i;
		std::cout << (std::to_string(num) + ' ');
	});

	auto sum = fay::parallel_reduce(v.cbegin(), v.cend(), 0,
		[](const auto& subrange, int value) {
			return std::accumulate(subrange.begin(), subrange.end(), value);
		},
		std::plus<int>()
	);

	std::cout << "\n sum: " << sum << '\n';
}