#ifndef APP_INCLUDE_UART_H
#define APP_INCLUDE_UART_H

#include <myLib/RingBuffer.hpp>
#include <myLib/Semaphore.hpp>

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <cstdint>
#include <span>

template <std::size_t SIZE_V>
class UartBuffer {
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

template <typename BUFFER_T>    // TODO concept
class Uart {
  public:
    constexpr Uart(const device* const dev, BUFFER_T& recvBuf) noexcept
     : _dev(dev)
     , _recvBuf(recvBuf)
    { }

    bool init() noexcept
    {
        if (!device_is_ready(_dev)) {
            return false;
        }

        if (uart_irq_callback_user_data_set(_dev, rxCallbackHelper, reinterpret_cast<void*>(this)) != 0) {
            return false;
        }
        return true;
    }

    void send(std::span<const std::uint8_t> data) noexcept { }    // TODO implement

    void sendPolling(std::span<const std::uint8_t> data) noexcept
    {
        for (std::uint8_t d : data) {
            // printk("poll out %c\n", d + '0');
            uart_poll_out(_dev, d);
        }
    }

    void enableRx() const noexcept { uart_irq_rx_enable(_dev); }
    void disableRx() const noexcept { uart_irq_rx_disable(_dev); }

    void enableTx() const noexcept { uart_irq_tx_enable(_dev); }
    void disableTx() const noexcept { uart_irq_tx_disable(_dev); }

  private:
    void rxCallback() noexcept
    {
        if (!uart_irq_update(_dev)) {
            return;
        }

        if (!uart_irq_rx_ready(_dev)) {
            return;
        }

        std::uint8_t c;
        while (uart_fifo_read(_dev, &c, 1) == 1) {
            // printk("rec:%c\n", c + '0');
            _recvBuf.push(c);
        }
    }

    // void txCallback() noexcept   // TODO fix impl.
    // {
    //     if (!uart_irq_update(_dev)) {
    //         return;
    //     }

    //     size_t remCnt = _sendBuf.size();
    //     while (uart_irq_tx_ready(_dev) && remCnt > 0) {
    //         const int ret = uart_fifo_fill(&_dev, , );
    //         if (ret < 0) {
    //             // TODO abort
    //             uart_irq_tx_disable(_dev);
    //             return;
    //         }

    //         remCnt -= ret;
    //         if (remCnt <= 0) {
    //             uart_irq_tx_disable(_dev);
    //             return;
    //         }
    //         _sendBuf = _sendBuf.subspan();    // TODO
    //     }
    // }

    static void rxCallbackHelper(const struct device* dev, void* user_data) noexcept
    {
        reinterpret_cast<Uart*>(user_data)->rxCallback();
    }

    const device* const _dev;
    BUFFER_T& _recvBuf;
};

#endif    // APP_INCLUDE_UART_H
