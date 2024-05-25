#ifndef SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_SEMAPHORE_H
#define SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_SEMAPHORE_H

#include <zephyr/kernel.h>

#include <chrono>
#include <cstdint>

namespace myLib {

class Semaphore {
  public:
    Semaphore(std::size_t limit = 1, std::size_t startCnt = 0) noexcept { k_sem_init(&_sem, startCnt, limit); }

    void give() noexcept { k_sem_give(&_sem); }
    void take() noexcept { k_sem_take(&_sem, K_FOREVER); }
    bool take(std::chrono::milliseconds timeout) noexcept { return k_sem_take(&_sem, K_MSEC(timeout.count())) == 0; }
    void reset() noexcept { k_sem_reset(&_sem); }

  private:
    k_sem _sem;
};

}    // namespace myLib

#endif    // SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_SEMAPHORE_H
