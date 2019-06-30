#pragma once

#include <fay/core/container.h>
#include <fay/core/fay.h>

namespace fay
{

// fay::array is a variable-length arrays(VLA)
// TODO: fay::array is a fay::container
// WARNING: when use fay::container, always use 'fay::' to avoid confusion with standard container

template<typename T>
class array : container<T>
{
public:
    explicit array(uint size)
    {

    }

    size_type size()
    {
        return sz_;
    }

private:
    size_type sz_;
    std::unique_ptr<T[]> a_;
};

} // namespace fay