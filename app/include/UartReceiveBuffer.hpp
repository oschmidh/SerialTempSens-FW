#ifndef SERIALTEMPSENS_FW_APP_INCLUDE_UARTRECEIVEBUFFER_H
#define SERIALTEMPSENS_FW_APP_INCLUDE_UARTRECEIVEBUFFER_H

#include <myLib/RingBuffer.hpp>
#include <myLib/Semaphore.hpp>

#include <cstdint>

template <std::size_t SIZE_V>
class UartReceiveBuffer {
  public:
    using DataType = std::uint8_t;

    DataType pull() noexcept
    {
        _sem.take();
        const DataType val = _buf.pull();
        return val;
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
    myLib::RingBuffer<DataType, SIZE_V> _buf;
};

#endif    // SERIALTEMPSENS_FW_APP_INCLUDE_UARTRECEIVEBUFFER_H
