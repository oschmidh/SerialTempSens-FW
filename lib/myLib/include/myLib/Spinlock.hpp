#ifndef LIB_MYLIB_INCLUDE_SPINLOCK_H
#define LIB_MYLIB_INCLUDE_SPINLOCK_H

#include <zephyr/spinlock.h>

namespace myLib {

using SpinlockKey = k_spinlock_key_t;

class Spinlock {
  public:
    SpinlockKey lock() noexcept { return k_spin_lock(&_lock); }
    void unlock(SpinlockKey key) noexcept { k_spin_unlock(&_lock, key); }

  private:
    k_spinlock _lock{};
};

class SpinlockGuard {
  public:
    SpinlockGuard(Spinlock& lock) noexcept
     : _lock(lock)
    {
        _key = _lock.lock();
    }
    SpinlockGuard(const SpinlockGuard&) = delete;
    ~SpinlockGuard() noexcept { _lock.unlock(_key); }

  private:
    Spinlock& _lock;
    SpinlockKey _key;
};

}    // namespace myLib

#endif    // LIB_MYLIB_INCLUDE_SPINLOCK_H
