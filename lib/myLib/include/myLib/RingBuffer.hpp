#ifndef SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_RINGBUFFER_H
#define SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_RINGBUFFER_H

#include <optional>
#include <array>
#include <cstdint>

namespace myLib {

namespace Concepts {

template <typename T>
concept Lockable = requires(T lock) {
    lock.lock();
    lock.unlock();
};

}    // namespace Concepts

namespace internal {

template <std::size_t SIZE_V>    // TODO special case for sizes that are powers of 2: wraparound can be simplified
class RingIndex {
  public:
    std::size_t operator-(std::size_t val)
    {
        if (_idx < val) {
            val -= _idx;
            return SIZE_V - val;
        } else {
            return _idx - val;
        }
    }

    RingIndex& operator++()    // prefix
    {
        // _idx = ++_idx % SIZE_V;
        if (++_idx >= SIZE_V) {
            _idx = 0;
        }
        return *this;
    }

    RingIndex operator++(int)    // postfix
    {
        const RingIndex old = *this;
        operator++();
        return old;
    }

    RingIndex& operator--()    // prefix
    {
        if (_idx > 0) {
            --_idx;
        } else {
            _idx = SIZE_V - 1;
        }
        return *this;
    }

    RingIndex operator--(int)    // postfix
    {
        const RingIndex old = *this;
        operator--();
        return old;
    }

    operator std::size_t() const noexcept { return _idx; }

  private:
    std::size_t _idx{};
};

template <Concepts::Lockable LOCK_T>
class LockGuard {
  public:
    LockGuard(LOCK_T& lock) noexcept
     : _lock(lock)
    {
        _lock.lock();
    }
    LockGuard(const LockGuard&) = delete;
    ~LockGuard() noexcept { _lock.unlock(); }

  private:
    LOCK_T& _lock;
};

}    // namespace internal

namespace Policies {

template <Concepts::Lockable LOCK_T>
class ThreadSafe {

  public:
    // using LockGuardType = std::lock_guard<LOCK_T>;
    using LockGuardType = internal::LockGuard<LOCK_T>;

    LockGuardType makeLockGuard() const noexcept { return {m_lock}; }

  private:
    mutable LOCK_T m_lock;
};

}    // namespace Policies

namespace internal {

class NonThreadSafe {
  public:
    constexpr auto makeLockGuard() const noexcept { return *this; }
};

template <template <typename> typename, typename, typename...>
struct GetPolicy;

template <template <typename> typename POLICY_T, typename DEFAULT_T, typename FIRST_T, typename... REST_Ts>
struct GetPolicy<POLICY_T, DEFAULT_T, FIRST_T, REST_Ts...> {
    using Type = typename GetPolicy<POLICY_T, DEFAULT_T, REST_Ts...>::Type;
};

template <template <typename> typename POLICY_T, typename DEFAULT_T, typename T, typename... REST_Ts>
struct GetPolicy<POLICY_T, DEFAULT_T, POLICY_T<T>, REST_Ts...> {
    using Type = POLICY_T<T>;
};

template <template <typename> typename POLICY_T, typename DEFAULT_T>
struct GetPolicy<POLICY_T, DEFAULT_T> {
    using Type = DEFAULT_T;
};

}    // namespace internal

// TODO add overwrite policy?
template <typename DATA_T, std::size_t SIZE_V, typename... POLICY_Ts>
class RingBuffer : private internal::GetPolicy<Policies::ThreadSafe, internal::NonThreadSafe, POLICY_Ts...>::Type {
  public:
    consteval std::size_t size() const noexcept { return SIZE_V - 1; }

    constexpr bool isEmpty() const noexcept
    {
        auto lockguard = this->makeLockGuard();

        return _tail == _head;
    }

    constexpr bool isFull() const noexcept
    {
        auto lockguard = this->makeLockGuard();

        return _tail == _head - 1;
    }

    constexpr bool push(DATA_T b) noexcept
    {
        if (isFull()) {
            return false;
        }
        _buf[_head++] = b;
        return true;
    }

    constexpr std::optional<DATA_T> pull() noexcept
    {
        if (isEmpty()) {
            return std::nullopt;
        }
        return _buf[_tail++];
    }

  private:
    internal::RingIndex<SIZE_V> _tail{};
    internal::RingIndex<SIZE_V> _head{};
    std::array<DATA_T, SIZE_V> _buf;
};

}    // namespace myLib

#endif    // SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_RINGBUFFER_H
