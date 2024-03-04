#ifndef LIB_MYLIB_INCLUDE_RINGBUFFER_H
#define LIB_MYLIB_INCLUDE_RINGBUFFER_H

#include "Spinlock.hpp"

#include <array>
#include <cstdint>

namespace myLib {

namespace internal {

template <std::size_t SIZE_V>    // TODO special case for sizes that are powers of 2: wraparound can be simplified
class RingIndex {
  public:
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
    std::size_t _idx;
};

}    // namespace internal

// TODO add overwrite policy?
template <typename DATA_T, std::size_t SIZE_V>    // TODO add tread-safe policy?
class RingBuffer {
  public:
    constexpr bool isEmpty() const noexcept
    {
        SpinlockGuard lock(_lock);
        return _isEmpty();
    }
    constexpr bool isFull() const noexcept
    {
        SpinlockGuard lock(_lock);
        return _isFull();
    }

    constexpr bool push(DATA_T b) noexcept
    {
        SpinlockGuard lock(_lock);

        if (_isFull()) {
            return false;
        }
        _buf[_head++] = b;
        if (_tail == _head) {
            _full = true;
        }
        return true;
    }

    constexpr DATA_T pull() noexcept
    {
        SpinlockGuard lock(_lock);

        if (_isEmpty()) {
            return {};    // TODO what to do here?
        }
        _full = false;
        return _buf[_tail++];
    }

  private:
    // not thread-safe:
    constexpr bool _isEmpty() const noexcept { return (_tail == _head) && !_isFull(); }
    constexpr bool _isFull() const noexcept { return _full; }

    mutable Spinlock _lock{};
    bool _full{};
    internal::RingIndex<SIZE_V> _tail{};
    internal::RingIndex<SIZE_V> _head{};
    std::array<DATA_T, SIZE_V> _buf;
};

}    // namespace myLib

#endif    // LIB_MYLIB_INCLUDE_RINGBUFFER_H
