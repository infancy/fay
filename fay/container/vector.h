#pragma once

#include <fay/core/fay.h>

namespace fay
{

// a variable buffer
template<typename T>
class base_vector
{

};

template<typename T>
class vector : public base_vector<T>
{

};

} // namespace fay