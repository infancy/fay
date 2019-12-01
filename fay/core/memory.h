#pragma once

#include <memory>

#include "fay/core/fay.h"

namespace fay
{

// allocate count * sizeof(T) bytes
template<typename T>
inline T* allocate(size_t count)
{	
    DCHECK(count > 0) << "allocate(size_t n): 'count' is 0";
    DCHECK(count < (static_cast<size_t>(-1) / sizeof(T))) << "allocate(size_t n): 'count' exceeds maximum supported size";

    return static_cast<T*>(::operator new(count * sizeof(T)));
}

inline void deallocate(void* ptr) //, size_t count)
{
    ::operator delete(ptr);
}

template<typename T, typename... Args>
void construct(const T* ptr, Args&&... args)
{	
    // new((void*)U) U{args}, construct U(Args...) at ptr
    ::new(const_cast<void*>(static_cast<const volatile void*>(ptr)))
        T{ std::forward<Args>(args)... };
}

template<typename T>
void destruct(const T* ptr)
{
    ptr->~T();
}

// inline auto memory_deleter = [](void* ptr) { ::operator delete(ptr); };

// TODO
// is_standard_layout 
// is_trivial

// TODO: heap_value, heap_array

// rename: buffer
// TODO: shared_memory
// template<typename D = allocator>
class memory
{
public:
    memory() = default;
    // TODO: byte*
    memory(const uint8_t* data, uint32_t size)
    {
        DCHECK((data != nullptr) && (size > 0));

        uint8_t* dst = allocate<uint8_t>(size);
        std::memcpy(dst, data, size); // WARNNING: memcmp???

        size_ = size; // WARNNING: you forget it!!!
        data_.reset(dst);
    }
    memory(const memory& mem)
    {
        // construct(this, mem.data(), mem.size());

        // TODO: better way
        if (mem.size() > 0)
        {
            memory tmp(mem.data(), mem.size());
            std::swap(tmp, *this);
        }
    }
    memory& operator=(const memory& mem)
    {
        if (mem.size() > 0)
        {
            memory tmp(mem.data(), mem.size());
            std::swap(tmp, *this); // std::swap(std::move(tmp), *this);
        }
        return *this;
    }
    memory(memory&& mem) = default;
    memory& operator=(memory&& mem) = default;
    ~memory() = default;

    uint8_t*       data()       { return data_.get(); }
    const uint8_t* data() const { return data_.get(); }

    uint32_t size()       { return size_; }
    uint32_t size() const { return size_; }

private:
    // std::unique_ptr<void, decltype(memory_deleter)> data_{ nullptr, memory_deleter }; /*[](void* ptr) { ::operator delete(ptr); }*/
    // std::unique_ptr<uint8_t, decltype(deallocate)> data_{ nullptr, deallocate }; /*[](void* ptr) { ::operator delete(ptr); }*/
    std::unique_ptr<uint8_t, decltype(deallocate)*> data_{ nullptr, deallocate }; /*[](void* ptr) { ::operator delete(ptr); }*/
    uint32_t size_{};
};

template <typename T>
inline memory copy_memory(const T& data)
{
    return memory(&data, sizeof(T));
}

} // namespace fay