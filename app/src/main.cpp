#include <SerialTempSens_messages.h>

#include "MessageChannel.hpp"

// #include <zephyr/kernel.h>
#include <zephyr/device.h>
// #include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

// K_MSGQ_DEFINE(uart_msgq, sizeof(ReadBufferType), 10, 1);

static const struct device* const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_com_uart));

// void serial_cb(const struct device* dev, void* user_data)
// {
//     enum class State { Header, Data };

//     uint8_t c;
//     static ReadBufferType rx_buf;
//     static State state;
//     static int n_bytes = 0;

//     if (!uart_irq_update(uart_dev)) {
//         return;
//     }

//     if (!uart_irq_rx_ready(uart_dev)) {
//         return;
//     }

//     while (uart_fifo_read(uart_dev, &c, 1) == 1) {

//         switch (state) {
//             case State::Header:

//                 n_bytes = c;
//                 state = State::Data;
//                 break;

//             case State::Data:
//                 rx_buf.push(c);
//                 if (--n_bytes <= 0) {
//                     k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);
//                     rx_buf.clear();
//                     state = State::Header;
//                 }
//                 break;
//         }
//     }
// }

int main()
{
    constexpr std::array<const device* const, 3> tempSensors{
        DEVICE_DT_GET(DT_NODELABEL(sens0)),
        DEVICE_DT_GET(DT_NODELABEL(sens1)),
        DEVICE_DT_GET(DT_NODELABEL(sens2)),
    };

    MessageChannel channel(uart_dev);

    if (!channel.init()) {
        LOG_ERR("Failed to init MessageChannel");
    }

    channel.start();    // starts rx interrupt
    LOG_DBG("msgChannel started");

    while (1) {
        const Command cmd = channel.receive<Command>();
        LOG_DBG("command received");

        Reply rply;

        if (cmd.sensorId() >= tempSensors.size()) {
            LOG_WRN("Invalid sensor ID received");
            rply.set_error(ErrorCode::InvalidSensorId);
        } else {
            sensor_sample_fetch(tempSensors[cmd.sensorId()]);

            struct sensor_value temp;
            sensor_channel_get(tempSensors[cmd.sensorId()], SENSOR_CHAN_AMBIENT_TEMP, &temp);
            rply.set_error(ErrorCode::NoError);
            rply.set_temperature(sensor_value_to_milli(&temp));
        }

        channel.send(rply);    // TODO create raii replyFinisher
    }

    return 0;
}
