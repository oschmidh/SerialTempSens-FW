#ifndef SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_UART_H
#define SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_UART_H

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <cstdint>
#include <span>

namespace Concept {

template <typename BUFFER_T>
concept UartBuffer = requires(BUFFER_T buf, std::uint8_t c) { buf.push(c); };

}    // namespace Concept

template <Concept::UartBuffer RX_BUFFER_T>
class Uart {
  public:
    constexpr Uart(const device* const dev, RX_BUFFER_T& rxBuf) noexcept
     : _dev(dev)
     , _rxBuf(rxBuf)
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

    void sendPolling(std::span<const std::uint8_t> data) noexcept
    {
        for (std::uint8_t d : data) {
            sendPolling(d);
        }
    }
    void sendPolling(std::uint8_t byte) noexcept { uart_poll_out(_dev, byte); }

    void enableRx() const noexcept { uart_irq_rx_enable(_dev); }
    void disableRx() const noexcept { uart_irq_rx_disable(_dev); }

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
            _rxBuf.push(c);
        }
    }

    static void rxCallbackHelper(const struct device* dev, void* user_data) noexcept
    {
        reinterpret_cast<Uart*>(user_data)->rxCallback();
    }

    const device* const _dev;
    RX_BUFFER_T& _rxBuf;
};

#endif    // SERIALTEMPSENS_FW_LIB_MYLIB_INCLUDE_MYLIB_UART_H
