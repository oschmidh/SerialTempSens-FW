#include <SerialTempSens_messages.h>

#include "WriteBuffer.hpp"
#include "SensorService.hpp"
#include "UartReceiveBuffer.hpp"

#include <myLib/Uart.hpp>
#include <ReadBufferFixedSize.h>

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

using ReadBufferType = EmbeddedProto::ReadBufferFixedSize<32>;

static const struct device* const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_com_uart));

int main()
{
    SensorService sensors;

    sensors.init();
    sensors.start();

    if (!device_is_ready(uart_dev)) {
        printk("UART device not found!");
        return 0;
    }

    UartReceiveBuffer<32> rxBuf{};
    Uart uart{uart_dev, rxBuf};

    uart.init();

    uart.enableRx();

    while (1) {
        const auto numBytes = rxBuf.pull();
        if (!numBytes.has_value()) {
            LOG_WRN("failed to pull from uart rx buffer");
            continue;
        }

        ReadBufferType readBuf;
        while (readBuf.get_size() < numBytes.value()) {
            const auto data = rxBuf.pull();
            if (!data.has_value()) {
                LOG_WRN("failed to pull from uart rx buffer");
                continue;
            }
            readBuf.push(data.value());
        }

        Command receivedCmd;
        if (receivedCmd.deserialize(readBuf) == ::EmbeddedProto::Error::NO_ERRORS) {
            LOG_DBG("command received");

            Reply rply;

            const auto ret = sensors.get(receivedCmd.sensorId());
            if (!ret.has_value()) {
                switch (ret.error()) {
                    case SensorError::InvalidId:
                        rply.set_error(ErrorCode::InvalidSensorId);
                        LOG_ERR("Invalid sensor ID received");
                        break;

                    case SensorError::NotPresent:
                        rply.set_error(ErrorCode::NotPresent);
                        LOG_ERR("no sensor connected on channel %d", receivedCmd.sensorId());
                        break;

                    case SensorError::ReadFail:
                        rply.set_error(ErrorCode::NotPresent);
                        LOG_ERR("Failed to read sensor");
                        break;
                }

            } else {
                rply.set_error(ErrorCode::NoError);
                rply.set_temperature(ret.value());
            }

            WriteBuffer writeBuf;
            if (rply.serialize(writeBuf) == ::EmbeddedProto::Error::NO_ERRORS) {

                const int n_bytes = writeBuf.get_size();
                uart.sendPolling(n_bytes);
                uart.sendPolling(writeBuf);
            }
        }
    }
    return 0;
}
