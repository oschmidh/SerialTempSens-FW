#ifndef SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_SPINLOCK_H
#define SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_SPINLOCK_H

#include <zephyr/spinlock.h>

namespace myLib {

class Spinlock {
  public:
    void lock() noexcept { m_key = k_spin_lock(&_lock); }
    void unlock() noexcept { k_spin_unlock(&_lock, m_key); }

  private:
    k_spinlock_key_t m_key{};
    k_spinlock _lock{};
};

}    // namespace myLib

#endif    // SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_SPINLOCK_H
