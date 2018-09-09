#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_ALGORITHM_H
#define FAY_CORE_ALGORITHM_H

namespace fay
{

// TODO
template<class InputIt, class OutputIt, class BinaryOperation>
void transform(InputIt first1, InputIt last1, OutputIt first2,
	BinaryOperation binary_op)
{
	while (first1 != last1) 
	{
		binary_op(*first1++, *first2++);
	}
}


} // namespace fay

#endif // FAY_CORE_ALGORITHM_H
