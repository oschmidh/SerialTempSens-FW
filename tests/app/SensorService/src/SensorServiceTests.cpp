#include "SensorService.hpp"

#include <zephyr/drivers/adc/adc_emul.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

ZTEST_SUITE(sensorService_tests, NULL, NULL, NULL, NULL, NULL);

#define SENSOREMUL_DT_GET(nodeId)         \
    {                                     \
        ADC_DT_SPEC_GET_BY_IDX(nodeId, 0) \
    }

class SensorEmul {
  public:
    SensorEmul(adc_dt_spec spec)
     : _spec(spec)
    {
        zassert_ok(adc_emul_ref_voltage_set(_spec.dev, ADC_REF_INTERNAL, refVoltage));
    }

    void setTemperature(int temp) noexcept
    {
        // TODO calculate actual temperature:
        std::uint32_t value = temp >= 0 ? temp * 32 : 0;
        zassert_ok(adc_emul_const_value_set(_spec.dev, _spec.channel_id, value));
    }

    void setOpen() noexcept { zassert_ok(adc_emul_const_value_set(_spec.dev, _spec.channel_id, refVoltage)); }

  private:
    static constexpr std::uint32_t refVoltage = 3300;

    const adc_dt_spec _spec;
};

ZTEST(sensorService_tests, test_init)
{
    SensorService sensors{};

    zassert_true(sensors.init());
}

ZTEST(sensorService_tests, test_periodicFetch_readData_valuesChange)
{
    SensorEmul emul = SENSOREMUL_DT_GET(DT_NODELABEL(sens0));
    SensorService sensors{};

    static constexpr int inputs[] = {7, 42, -31, 25};
    int values[std::size(inputs)] = {};

    sensors.init();
    sensors.start();

    for (std::size_t i = 1; i < std::size(values); ++i) {
        emul.setTemperature(inputs[i]);
        k_sleep(K_MSEC(CONFIG_SENSORSERVICE_FETCH_INTERVAL * 2));
        values[i] = sensors.get(0).value_or(0);
    }

    sensors.stop();

    for (std::size_t i = 1; i < std::size(values); ++i) {
        zassert_not_equal(values[i - 1], values[i], "i=%d", i);
    }
}

ZTEST(sensorService_tests, test_notStarted_notFetching)
{
    SensorEmul emul = SENSOREMUL_DT_GET(DT_NODELABEL(sens0));
    SensorService sensors{};

    emul.setTemperature(0);
    sensors.init();

    const auto ret1 = sensors.get(0);
    zassert_false(ret1.has_value());
    zassert_equal(ret1.error(), SensorError::NotPresent);

    emul.setTemperature(42);
    k_sleep(K_MSEC(CONFIG_SENSORSERVICE_FETCH_INTERVAL * 2));

    zassert_false(ret1.has_value());
    zassert_equal(ret1.error(), SensorError::NotPresent);
}

ZTEST(sensorService_tests, test_start_stop)
{
    SensorEmul emul = SENSOREMUL_DT_GET(DT_NODELABEL(sens0));
    SensorService sensors{};

    emul.setTemperature(15);

    sensors.init();
    sensors.start();
    k_sleep(K_MSEC(CONFIG_SENSORSERVICE_FETCH_INTERVAL * 2));
    sensors.stop();

    const auto ret1 = sensors.get(0);
    zassert_true(ret1.has_value());

    k_sleep(K_MSEC(CONFIG_SENSORSERVICE_FETCH_INTERVAL * 2));

    const auto ret2 = sensors.get(0);
    zassert_true(ret2.has_value());

    zassert_equal(ret1.value(), ret2.value());
}

ZTEST(sensorService_tests, test_sensorOpen_error)
{
    SensorEmul emul = SENSOREMUL_DT_GET(DT_NODELABEL(sens0));
    SensorService sensors{};

    emul.setOpen();

    sensors.init();
    sensors.start();
    k_sleep(K_MSEC(CONFIG_SENSORSERVICE_FETCH_INTERVAL * 2));
    sensors.stop();

    const auto ret = sensors.get(0);
    zassert_false(ret.has_value());
    zassert_equal(ret.error(), SensorError::NotPresent);
}

ZTEST(sensorService_tests, test_invalidId_error)
{
    SensorEmul emul = SENSOREMUL_DT_GET(DT_NODELABEL(sens0));
    SensorService sensors{};

    emul.setOpen();

    sensors.init();
    sensors.start();
    k_sleep(K_MSEC(CONFIG_SENSORSERVICE_FETCH_INTERVAL * 2));
    sensors.stop();

    const auto ret = sensors.get(6);
    zassert_false(ret.has_value());
    zassert_equal(ret.error(), SensorError::InvalidId);
}
