#include "SensorService.hpp"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sensorService, CONFIG_SENSORSERVICE_LOG_LEVEL);

namespace {

K_THREAD_STACK_DEFINE(threadStack, CONFIG_SENSORSERVICE_STACK_SIZE);

}

bool SensorService::init() const noexcept
{
    for (const auto sensor : tempSensors) {
        if (!device_is_ready(sensor)) {
            LOG_WRN("Sensor not ready\n");
            return false;
        }
    }
    return true;
}

void SensorService::start() noexcept
{
    _threadId = k_thread_create(&_thread, threadStack, K_THREAD_STACK_SIZEOF(threadStack), threadEntry, this, nullptr,
                                nullptr, CONFIG_SENSORSERVICE_THREAD_PRIO, 0, K_NO_WAIT);
}

auto SensorService::get(std::size_t idx) const noexcept -> std::expected<int, SensorError>
{
    if (idx >= tempSensors.size()) {
        return std::unexpected(SensorError::InvalidId);
    }

    struct sensor_value temp;
    if (const int rc = sensor_channel_get(tempSensors[idx], SENSOR_CHAN_AMBIENT_TEMP, &temp); rc != 0) {
        return std::unexpected(SensorError::ReadFail);
    }

    const int val = sensor_value_to_milli(&temp);

    if (val == -25000) {
        return std::unexpected(SensorError::NotPresent);
    }

    return val;
}

void SensorService::fetchSensors() const noexcept
{
    while (1) {

        for (const auto sensor : tempSensors) {
            if (const int rc = sensor_sample_fetch(sensor); rc != 0) {
                LOG_ERR("failed to fetch sensor, %d", rc);
            }
        }

        k_sleep(K_MSEC(CONFIG_SENSORSERVICE_FETCH_INTERVAL));
    }
}
