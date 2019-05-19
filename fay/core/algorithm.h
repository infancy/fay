#pragma once
#include "fay/core/fay.h"

namespace fay
{

// TODO
template<class InputIt, class OutputIt, class BinaryOperation>
inline void transform(InputIt first1, InputIt last1, OutputIt first2,
	BinaryOperation binary_op)
{
	while (first1 != last1) 
	{
		binary_op(*first1++, *first2++);
	}
}

inline void loop(uint cnt, std::function<void()> functor)
{
    for (uint i = 0; i < cnt; ++i)
        functor();
}

inline void loop(uint cnt, std::function<void(uint)> functor)
{
    for (uint i = 0; i < cnt; ++i)
        functor(i);
}

} // namespace fay
