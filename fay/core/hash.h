#pragma once

#include "fay/core/fay.h"

namespace fay
{

// http://www.isthe.com/chongo/tech/comp/fnv/

constexpr inline uint64_t iterate1_(uint8_t ch, uint64_t hash)
{
    return (hash * 0x100000001b3ull) ^ ch; // (hash * FNV_prime) xor octet_of_data
}

constexpr inline uint64_t iterate_(uint8_t ch, uint64_t hash)
{
    return (hash ^ uint64_t(ch)) * 0x100000001b3ull; // (hash xor octet_of_data) * FNV_prime
}

template <typename T>
struct fnv;

template <>
struct fnv<uint64_t>
{
    static constexpr inline uint64_t default_offset_basis = 0xcbf29ce484222325ull;

    static constexpr uint64_t hash(const char* str, const uint64_t val = default_offset_basis)
    {
        return (str[0] == '\0') ? val : hash(&str[1], iterate_(str[0], val)); // can't use ++str
    }

    static constexpr uint64_t merge_(const uint64_t a, const uint64_t b)
    {
        return
            iterate_(uint8_t(a >> 56),
                iterate_(uint8_t(a >> 48),
                    iterate_(uint8_t(a >> 40),
                        iterate_(uint8_t(a >> 32),
                            iterate_(uint8_t(a >> 24),
                                iterate_(uint8_t(a >> 16),
                                    iterate_(uint8_t(a >> 8),
                                        iterate_(uint8_t(a >> 0), b))))))));
    }

    static constexpr uint64_t merge(uint64_t hash)
    {
        return hash;
    }

    template <typename T, typename... Ts>
    static constexpr uint64_t merge(T hash, T hash2, Ts... hashes)
    {
        return merge(merge_(hash, hash2), hashes...);
    }
};

class hasher
{
public:
    hasher() = default;
    hasher(uint64_t h)
        : h(h)
    {
    }

    template <typename T>
    void hash(T data) // don't need hash(T* ptr)
    {
        hash(&data, 1);
    }

    void hash(const char* str)
    {
        //u32(0xff);
        hash(str, std::strlen(str));
    }

    void hash(const std::string& str)
    {
        hash(str.data(), str.size());
    }

    template <typename T>
    void hash(const T* data, size_t size)
    {
        auto ch = reinterpret_cast<const char*>(data);
        size *= sizeof(T);
        for (size_t i = 0; i < size; i++)
            h = iterate_(ch[i], h);
    }

    uint64_t value() const
    {
        return h;
    }

private:
    uint64_t h{ fnv<uint64_t>::default_offset_basis };
};

} // namespace fay