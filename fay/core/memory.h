#pragma once

#include <memory>

#include "fay/core/fay.h"

namespace fay
{

// memory block

/*
// allocate & deallocate
    pointer allocate(size_t count)
    {	// allocate array of count elements
        if (count > max_size())
            throw std::length_error("allocator<T>::allocate(size_t n)"
                " 'n' exceeds maximum supported size");
        return static_cast<pointer>(::operator new(count * sizeof(T)));
    }

    void deallocate(pointer ptr, size_t count)
    {
        ::operator delete(ptr);
    }

    // construct & destroy
    template<class U, class... Args>
    void construct(const U* ptr, Args&&... args)
    {	// new((void*)U) U{args}, construct U(Args...) at ptr
        ::new(const_cast<void*>(static_cast<const volatile void*>(ptr)))
            U{std::forward<Args>(args)...};
    }

    template<class U>
    void destroy(const U* ptr)
    {
        ptr->~U();
    }
*/

inline void deallocate(void* ptr) //, size_t count)
{
    ::operator delete(ptr);
}

// inline auto memory_deleter = [](void* ptr) { ::operator delete(ptr); };

// TODO: shared_memory
// template<typename D = allocator>
class memory
{
public:
    memory() = default;
    // TODO: byte*
    memory(const uint8_t* data, uint32_t size)
    {
        uint8_t* dst = static_cast<uint8_t*>(::operator new(size));
        std::memcpy(dst, data, size); // WARNNING: memcmp???

        size_ = size; // WARNNING: you forget it!!!
        data_.reset(dst);
    }
    memory(const memory& mem)
    {
        memory tmp(mem.data(), mem.size());
        std::swap(tmp, *this);
    }
    memory& operator=(const memory& mem)
    {
        memory tmp(mem.data(), mem.size());
        std::swap(tmp, *this); // std::swap(std::move(tmp), *this);
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