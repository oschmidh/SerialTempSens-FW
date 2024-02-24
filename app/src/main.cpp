#include <SerialTempSens_messages.h>

#include "WriteBuffer.hpp"

#include <ReadBufferFixedSize.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

using ReadBufferType = EmbeddedProto::ReadBufferFixedSize<32>;

K_MSGQ_DEFINE(uart_msgq, sizeof(ReadBufferType), 10, 1);

static const struct device* const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_com_uart));

void serial_cb(const struct device* dev, void* user_data)
{
    enum class State { Header, Data };

    uint8_t c;
    static ReadBufferType rx_buf;
    static State state;
    static int n_bytes = 0;

    if (!uart_irq_update(uart_dev)) {
        return;
    }

    if (!uart_irq_rx_ready(uart_dev)) {
        return;
    }

    while (uart_fifo_read(uart_dev, &c, 1) == 1) {

        switch (state) {
            case State::Header:

                n_bytes = c;
                state = State::Data;
                break;

            case State::Data:
                rx_buf.push(c);
                if (--n_bytes <= 0) {
                    k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);
                    rx_buf.clear();
                    state = State::Header;
                }
                break;
        }
    }
}

int main()
{
    constexpr const device* const sens0_dev = DEVICE_DT_GET(DT_NODELABEL(sens0));

    if (!device_is_ready(uart_dev)) {
        printk("UART device not found!");
        return 0;
    }

    if (uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL) != 0) {
        return 0;
    }
    uart_irq_rx_enable(uart_dev);

    ReadBufferType readBuf;
    while (k_msgq_get(&uart_msgq, &readBuf, K_FOREVER) == 0) {

        Command receivedCmd;
        if (receivedCmd.deserialize(readBuf) == ::EmbeddedProto::Error::NO_ERRORS) {

            sensor_sample_fetch(sens0_dev);
            struct sensor_value temp;
            sensor_channel_get(sens0_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
            int tempMilliDegC = sensor_value_to_milli(&temp);
            Reply rply;
            rply.set_temperature(tempMilliDegC);

            WriteBuffer writeBuf;
            if (rply.serialize(writeBuf) == ::EmbeddedProto::Error::NO_ERRORS) {

                const int n_bytes = writeBuf.get_size();
                uart_poll_out(uart_dev, n_bytes);

                for (std::uint8_t b : writeBuf) {
                    uart_poll_out(uart_dev, b);
                }
            }
        }
    }
    return 0;
}
