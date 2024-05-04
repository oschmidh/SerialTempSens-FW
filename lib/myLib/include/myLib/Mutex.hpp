#ifndef SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_MUTEX_H
#define SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_MUTEX_H

#include <zephyr/sys/mutex.h>

namespace myLib {

class Mutex {
  public:
    Mutex() noexcept { k_mutex_init(&_mutex); }

    void lock() noexcept { k_mutex_lock(&_mutex, K_FOREVER); }
    void unlock() noexcept { k_mutex_unlock(&_mutex); }

  private:
    k_mutex _mutex{};
};

}    // namespace myLib

#endif    // SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_MUTEX_H
