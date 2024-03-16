#ifndef SERIALTEMPSENS_FW_APP_INCLUDE_SENSORSERVICE_H
#define SERIALTEMPSENS_FW_APP_INCLUDE_SENSORSERVICE_H

#include "Sensor.hpp"

#include <zephyr/kernel.h>

#include <array>
#include <cstdint>

K_THREAD_STACK_DEFINE(threadStack, CONFIG_SENSORSERVICE_STACKSIZE);

class SensorService {
  public:
    constexpr SensorService() noexcept { }

    void init() noexcept
    {
        // TODO check readyness for all sensors
        // if (!device_is_ready(uart_dev)) {
        //     printk("UART device not found!");
        //     return 0;
        // }

        _threadId = k_thread_create(&_thread, threadStack, K_THREAD_STACK_SIZEOF(threadStack), threadEntry, this,
                                    nullptr, nullptr, prio, 0, K_FOREVER);
    }

    void start() const noexcept { k_thread_start(_threadId); }

    void stop() const noexcept { k_thread_abort(_threadId); }

    int get(std::size_t idx) noexcept
    {
        if (idx >= _tempSensors.size()) {
            return 0;    // TODO return error code
        }

        return _tempSensors[idx].getTempMilli();
    }

  private:
    static constexpr std::array<Sensor, 3> _tempSensors{{{DEVICE_DT_GET(DT_NODELABEL(sens0))},
                                                         {DEVICE_DT_GET(DT_NODELABEL(sens1))},
                                                         {DEVICE_DT_GET(DT_NODELABEL(sens2))}}};

    static constexpr int prio = 0;

    void fetchSensors()
    {
        while (1) {

            for (const auto sensor : _tempSensors) {
                sensor.fetch();
            }

            k_sleep(K_MSEC(CONFIG_SENSORSERVICE_FETCH_INTERVAL));
        }
    }

    static void threadEntry(void* p1, void*, void*) noexcept { reinterpret_cast<SensorService*>(p1)->fetchSensors(); }

    k_thread _thread;
    k_tid_t _threadId;
};

#endif    // SERIALTEMPSENS_FW_APP_INCLUDE_SENSORSERVICE_H
