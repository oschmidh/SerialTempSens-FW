#include <SerialTempSens_messages.h>

#include "WriteBuffer.hpp"
#include "UartReceiveBuffer.hpp"

#include <myLib/Uart.hpp>
#include <ReadBufferFixedSize.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
// #include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

using ReadBufferType = EmbeddedProto::ReadBufferFixedSize<32>;

static const struct device* const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_com_uart));

int main()
{
    constexpr std::array<const device* const, 3> tempSensors{
        DEVICE_DT_GET(DT_NODELABEL(sens0)),
        DEVICE_DT_GET(DT_NODELABEL(sens1)),
        DEVICE_DT_GET(DT_NODELABEL(sens2)),
    };

    UartReceiveBuffer<32> rxBuf{};
    Uart uart{uart_dev, rxBuf};

    uart.init();

    uart.enableRx();

    while (1) {
        const std::uint8_t numBytes = rxBuf.pull();

        ReadBufferType readBuf;
        while (readBuf.get_size() < numBytes) {
            readBuf.push(rxBuf.pull());
        }

        Command receivedCmd;
        if (receivedCmd.deserialize(readBuf) == ::EmbeddedProto::Error::NO_ERRORS) {

            if (receivedCmd.sensorId() >= tempSensors.size()) {
                LOG_WRN("Invalid sensor ID received");
                continue;
            }
            sensor_sample_fetch(tempSensors[receivedCmd.sensorId()]);

            struct sensor_value temp;
            sensor_channel_get(tempSensors[receivedCmd.sensorId()], SENSOR_CHAN_AMBIENT_TEMP, &temp);

            Reply rply;
            rply.set_temperature(sensor_value_to_milli(&temp));

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
