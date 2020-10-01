#pragma once

#include <stdexcept>
#include <type_traits>
#include <typeinfo>

#include "fay/core/fay.h"

#pragma region reference

// https://en.cppreference.com/w/cpp/utility/any

// small buffer optimization
// https://github.com/thelink2012/any/blob/master/any.hpp
// https://github.com/electronicarts/EASTL/blob/master/include/EASTL/any.h
// https://github.com/llvm-mirror/libcxx/blob/master/include/any

// virtual
// https://github.com/qicosmos/cosmos/blob/master/Any.hpp
// https://github.com/boostorg/any/blob/develop/include/boost/any.hpp



/* any synopsis

namespace std
{

class bad_any_cast : public bad_cast
{
    public:
    virtual const char* what() const noexcept;
};

class any
{
public:
    // 6.3.1 any construct/destruct
    any() noexcept;

    any(const any& other);
    any(any&& other) noexcept;

    template<class ValueType>
    any(ValueType&& value);

    ~any();

    // 6.3.2 any assignments
    any& operator=(const any& rhs);
    any& operator=(any&& rhs) noexcept;

    template<class ValueType>
    any& operator=(ValueType&& rhs);

    // 6.3.3 any modifiers
    template<class ValueType, class... Args>
    decay_t<ValueType>& emplace(Args&&... args);
    template<class ValueType, class U, class... Args>
    decay_t<ValueType>& emplace(initializer_list<U>, Args&&...);
    void reset() noexcept;
    void swap(any& rhs) noexcept;

    // 6.3.4 any observers
    bool has_value() const noexcept;
    const type_info& type() const noexcept;
};

// 6.4 Non-member functions
void swap(any& x, any& y) noexcept;

template<class T, class... Args>
any make_any(Args&&... args);
template<class T, class U, class... Args>
any make_any(initializer_list<U>, Args&&... args);

template<class ValueType>
ValueType any_cast(const any& operand);
template<class ValueType>
ValueType any_cast(any& operand);
template<class ValueType>
ValueType any_cast(any&& operand);

template<class ValueType>
const ValueType* any_cast(const any* operand) noexcept;
template<class ValueType>
ValueType* any_cast(any* operand) noexcept;

} // namespace std

*/

#pragma endregion reference



namespace fay
{

// TODO: concept, constexpr array

class bad_any_cast : public std::bad_cast
{
public:
    const char* what() const noexcept override { return "bad any cast"; }
};

inline namespace sbo
{

class any
{
public:
    any() noexcept : storage_{}
    {
        make_vtable<nullptr_t>();
    }

    any(const any& other) : vtable_{ other.vtable_ }
    {
        DCHECK(other.vtable_);
        other.vtable_->copy(other.storage_, storage_);
    }
    any(any&& other) noexcept : vtable_{ other.vtable_ }
    {
        DCHECK(other.vtable_);
        other.vtable_->move(other.storage_, storage_);
    }

    template<typename ValueType>
    any(ValueType&& value)
    {
        construct(std::forward<ValueType>(value));
    }

    ~any() { reset(); }

    any& operator=(const any& rhs)
    {
        any(rhs).swap(*this);
        return *this;
    }
    any& operator=(any&& rhs) noexcept
    {
        any(std::move(rhs)).swap(*this);
        return *this;
    }

    template<typename ValueType>
    any& operator=(ValueType&& rhs)
    {
        any(std::forward<ValueType>(value)).swap(*this);
        return *this;
    }

public:
    template<typename ValueType, typename... Args>
    std::decay_t<ValueType>& emplace(Args&&... args)
    {

    }
    template<typename ValueType, typename U, typename... Args>
    std::decay_t<ValueType>& emplace(std::initializer_list<U>, Args&&...)
    {

    }

    void reset() noexcept
    {
        if (has_value())
        {
            vtable_->destroy(storage_);
            vtable_ = nullptr;
        }
    }
    void swap(any& rhs) noexcept
    {
        if (vtable_ != rhs.vtable_)
        {
            any tmp(std::move(rhs));

            // move from *this to rhs.
            rhs.vtable_ = vtable_;
            if (vtable_ != nullptr)
            {
                vtable_->move(storage_, rhs.storage_);
                //vtable_ = nullptr; -- unneeded, see below
            }

            // move from tmp (previously rhs) to *this.
            vtable_ = tmp.vtable_;
            if (tmp.vtable_ != nullptr)
            {
                tmp.vtable_->move(tmp.storage_, storage_);
                tmp.vtable_ = nullptr;
            }
        }
        else // same types
        {
            if (vtable_ != nullptr)
                vtable_->swap(storage_, rhs.storage_);
        }
    }

    bool has_value() const noexcept
    {
        return vtable_ != nullptr;
    }
    const type_info& type() const noexcept
    {
        return has_value() ? vtable_->type() : typeid(void);
    }

protected:
    template<typename T>
    friend const T* any_cast(const any* operand) noexcept;
    template<typename T>
    friend T* any_cast(any* operand) noexcept;

    /// Casts (with no type_info checks) the storage pointer as const T*.
    template<typename T>
    const T* cast() const noexcept
    {
        return requires_dynamic<std::decay_t<T>>() ? reinterpret_cast<const T*>(storage_.dynamic)
                                                   : reinterpret_cast<const T*>(&storage_.stack);
    }

    /// Casts (with no type_info checks) the storage pointer as T*.
    template<typename T>
    T* cast() noexcept
    {
        return requires_dynamic<std::decay_t<T>>() ? reinterpret_cast<T*>(storage_.dynamic)
                                                   : reinterpret_cast<T*>(&storage_.stack);
    }

private:
    union storage_union 
    {
        using stack_storage_t = std::aligned_storage_t<2 * sizeof(void*), std::alignment_of_v<void*>>;

        void* dynamic;
        stack_storage_t stack{}; // 2 words for e.g. shared_ptr
    };

    struct vtable_type
    {
        // TODO constexpr(remove runtime overhead???)
        const std::type_info& (*type)() noexcept;

        void (*destroy)(storage_union&) noexcept;

        void (*copy)(const storage_union& src, storage_union& dest);

        void (*move)(storage_union& src, storage_union& dest) noexcept;

        void (*swap)(storage_union& lhs, storage_union& rhs) noexcept;
    };

    template<typename T>
    struct vtable_dynamic
    {
        static const std::type_info& type() noexcept { return typeid(T); }

        static void destroy(storage_union& storage) noexcept
        {
            //assert(reinterpret_cast<T*>(storage.dynamic));
            delete reinterpret_cast<T*>(storage.dynamic);
        }

        static void copy(const storage_union& src, storage_union& dest)
        {
            dest.dynamic = new T(*reinterpret_cast<const T*>(src.dynamic));
        }

        static void move(storage_union& src, storage_union& dest) noexcept
        {
            dest.dynamic = src.dynamic;
            src.dynamic = nullptr;
        }

        static void swap(storage_union& lhs, storage_union& rhs) noexcept
        {
            // just exchage the storage pointers.
            std::swap(lhs.dynamic, rhs.dynamic);
        }
    };

    template<typename T>
    struct vtable_stack
    {
        static const std::type_info& type() noexcept { return typeid(T); }

        static void destroy(storage_union& storage) noexcept { reinterpret_cast<T*>(&storage.stack)->~T(); }

        static void copy(const storage_union& src, storage_union& dest)
        {
            new (&dest.stack) T(reinterpret_cast<const T&>(src.stack));
        }

        static void move(storage_union& src, storage_union& dest) noexcept
        {
            new (&dest.stack) T(std::move(reinterpret_cast<T&>(src.stack)));
            destroy(src);
        }

        static void swap(storage_union& lhs, storage_union& rhs) noexcept
        {
            storage_union tmp;
            move(rhs, tmp);
            move(lhs, rhs);
            move(tmp, lhs);
        }
    };

    /*
    template<typename T>
    struct requires_allocation:
        std::integral_constant<bool,
                               !(std::is_nothrow_move_constructible_v<T> && 
                                 sizeof(T) <= sizeof(storage_union::stack) &&
                                 std::alignment_of_v<T> <= std::alignment_of_v<storage_union::stack_storage_t>)>
    {
    };
    template<typename T>
    static constexpr bool requires_allocation_v = requires_allocation<T>::value;
    */
    template<typename T>
    constexpr bool requires_dynamic()
    {
        return !(std::is_nothrow_move_constructible_v<T> && sizeof(T) <= sizeof(storage_union::stack) &&
                 std::alignment_of_v<T> <= std::alignment_of_v<storage_union::stack_storage_t>);
    }

    template<typename T>
    static void make_vtable()
    {
        using VTableType = std::conditional_t<requires_dynamic<T>(), vtable_dynamic<T>, vtable_stack<T>>;

        static vtable_type table = {
            VTableType::type, VTableType::destroy, VTableType::copy, VTableType::move, VTableType::swap,
        };

        vtable_ = &table;
    }

private:
    storage_union storage_; // on offset(0) so no padding for align
    vtable_type* vtable_;

    template<typename ValueType>
    void construct(ValueType&& value)
    {
        using T = std::decay_t<ValueType>;

        make_vtable<T>();
        make_storage<ValueType, T>(std::forward<ValueType>(value));
    }

    template<typename ValueType, typename T>
    void make_storage(ValueType&& value)
    {
        if constexpr (requires_dynamic<T>())
            storage_.dynamic = new T(std::forward<ValueType>(value));
        else
            new (&storage_.stack) T(std::forward<ValueType>(value));
    }

}; // class any

} // inline namespace sbo



namespace trivial
{

} // namespace trivial

} // namespace fay