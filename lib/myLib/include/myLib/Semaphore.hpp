#ifndef LIB_MYLIB_INCLUDE_SEMAPHORE_H
#define LIB_MYLIB_INCLUDE_SEMAPHORE_H

#include <zephyr/kernel.h>

#include <cstdint>

namespace myLib {

class Semaphore {
  public:
    Semaphore(std::size_t limit = 1, std::size_t startCnt = 0) noexcept { k_sem_init(&_sem, startCnt, limit); }

    void give() noexcept { k_sem_give(&_sem); }
    void take() noexcept { k_sem_take(&_sem, K_FOREVER); }
    void reset() noexcept { k_sem_reset(&_sem); }

  private:
    k_sem _sem;
};

}    // namespace myLib

#endif    // LIB_MYLIB_INCLUDE_SEMAPHORE_H
