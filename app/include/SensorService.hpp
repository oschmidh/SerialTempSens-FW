#ifndef SERIALTEMPSENS_FW_APP_INCLUDE_SENSORSERVICE_H
#define SERIALTEMPSENS_FW_APP_INCLUDE_SENSORSERVICE_H

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

#include <expected>
#include <array>
#include <cstdint>

enum class SensorError {
    InvalidId,
    NotPresent,
    ReadFail,
};

class SensorService {
  public:
    constexpr SensorService() noexcept { }

    bool init() const noexcept;
    void start() noexcept;
    void stop() const noexcept { k_thread_abort(_threadId); }
    auto get(std::size_t idx) const noexcept -> std::expected<int, SensorError>;

  private:
    static constexpr std::array<const device* const, 3> tempSensors{
        DEVICE_DT_GET(DT_NODELABEL(sens0)),
        DEVICE_DT_GET(DT_NODELABEL(sens1)),
        DEVICE_DT_GET(DT_NODELABEL(sens2)),
    };

    void fetchSensors() const noexcept;
    static void threadEntry(void* p1, void*, void*) noexcept { reinterpret_cast<SensorService*>(p1)->fetchSensors(); }

    k_thread _thread{};
    k_tid_t _threadId;
};

#endif    // SERIALTEMPSENS_FW_APP_INCLUDE_SENSORSERVICE_H
