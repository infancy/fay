#pragma once

#include <tbb/parallel_do.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <tbb/parallel_reduce.h>

namespace fay
{

template<typename Iterator>
constexpr bool _is_random_access_iterator()
{
	return std::is_same<std::random_access_iterator_tag,
		typename std::iterator_traits<Iterator>::iterator_category>::value;
}

// parallel_for : mainly for array, vector... ----------------------------------

template<typename RandomIterator, typename Function>
inline void parallel_for(RandomIterator begin, RandomIterator end, const Function& func)
{
	static_assert(_is_random_access_iterator<RandomIterator>(),
		"parallel_for only accepts random access iterators\n");

	tbb::parallel_for(tbb::blocked_range<RandomIterator>(begin, end), func);
}

template<typename Container, typename Function>
inline void parallel_for(Container& C, const Function& func)
{
	// parallel_for(tbb::blocked_range<decltype(v.cbegin())>(v.cbegin(), v.cend()), func);
	parallel_for(v.cbegin(), v.cend(), func);
}

// parallel_reduce -------------------------------------------------------------

template<typename RandomIterator, typename Value, typename SubRangeFunc, typename ReduceFunction>
inline Value parallel_reduce(RandomIterator begin, RandomIterator end, const Value& init, 
	const SubRangeFunc& func, const ReduceFunction& reduce)
{
	static_assert(_is_random_access_iterator<RandomIterator>(),
		"parallel_reduce only accepts random access iterators\n");

	return tbb::parallel_reduce(tbb::blocked_range<RandomIterator>(begin, end), init,
		func, reduce);
}

} // namespace fay
