#ifndef SERIALTEMPSENS_FW_APP_INCLUDE_UARTRECEIVEBUFFER_H
#define SERIALTEMPSENS_FW_APP_INCLUDE_UARTRECEIVEBUFFER_H

#include <myLib/RingBuffer.hpp>
#include <myLib/Spinlock.hpp>
#include <myLib/Semaphore.hpp>

#include <optional>
#include <cstdint>

template <std::size_t SIZE_V>
class UartReceiveBuffer {
  public:
    using DataType = std::uint8_t;

    std::optional<DataType> pull() noexcept
    {
        _sem.take();
        return _buf.pull();
    }

    bool push(DataType b) noexcept
    {
        if (!_buf.push(b)) {
            return false;
        }

        _sem.give();
        return true;
    }

  private:
    myLib::Semaphore _sem{SIZE_V};
    myLib::RingBuffer<DataType, SIZE_V, myLib::Policies::ThreadSafe<myLib::Spinlock>> _buf;
};

#endif    // SERIALTEMPSENS_FW_APP_INCLUDE_UARTRECEIVEBUFFER_H
