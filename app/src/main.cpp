#include <SerialTempSens_messages.h>

#include "SensorService.hpp"
#include "MessageChannel.hpp"

// #include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

static const struct device* const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_com_uart));

int main()
{
    SensorService sensors;

    sensors.init();
    sensors.start();

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

        const auto ret = sensors.get(cmd.sensorId());
        if (!ret.has_value()) {
            switch (ret.error()) {
                case SensorError::InvalidId:
                    rply.set_error(ErrorCode::InvalidSensorId);
                    LOG_ERR("Invalid sensor ID received");
                    break;

                case SensorError::NotPresent:
                    rply.set_error(ErrorCode::SensorOpen);
                    LOG_ERR("no sensor connected on channel %d", cmd.sensorId());
                    break;

                case SensorError::ReadFail:
                    rply.set_error(ErrorCode::IoError);
                    LOG_ERR("Failed to read sensor");
                    break;
            }
        } else {
            rply.set_temperature(ret.value());
            rply.set_error(ErrorCode::NoError);
        }

        channel.send(rply);    // TODO create raii replyFinisher
    }

    return 0;
}
