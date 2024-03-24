#include "UartReceiveBuffer.hpp"

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include <array>

ZTEST_SUITE(uartReceiveBuffer_tests, NULL, NULL, NULL, NULL, NULL);

ZTEST(uartReceiveBuffer_tests, test_push_pull)
{
    UartReceiveBuffer<32> buf{};

    zassert_true(buf.push(5));
    const auto ret = buf.pull();

    zassert_true(ret.has_value());
    zassert_equal(ret.value(), 5);
}

ZTEST(uartReceiveBuffer_tests, test_push_pull_multiple)
{
    const std::array<std::uint8_t, 5> testData = {1, 2, 3, 4, 5};

    UartReceiveBuffer<32> buf{};

    for (std::uint8_t d : testData) {
        zassert_true(buf.push(d));
    }

    for (unsigned int i = 0; i < testData.size(); ++i) {
        const auto ret = buf.pull();
        zassert_true(ret.has_value());
        zassert_equal(ret.value(), testData[i]);
    }
}

static constexpr int threadPrio = 0;
static constexpr int stackSize = 1024;

void consumerTask(void* buffer, void* outp, void*)
{
    UartReceiveBuffer<32>* buf = reinterpret_cast<UartReceiveBuffer<32>*>(buffer);

    std::optional<std::uint8_t>* out = reinterpret_cast<std::optional<std::uint8_t>*>(outp);
    *out = buf->pull();
}

K_THREAD_STACK_DEFINE(consumerStack, stackSize);

ZTEST(uartReceiveBuffer_tests, test_pull_blocking)
{
    const std::uint8_t input = 77;
    UartReceiveBuffer<32> buf{};
    std::optional<std::uint8_t> output;

    k_thread consumerThread{};
    k_thread_create(&consumerThread, consumerStack, K_THREAD_STACK_SIZEOF(consumerStack), consumerTask,
                    reinterpret_cast<void*>(&buf), reinterpret_cast<void*>(&output), NULL, threadPrio, 0, K_NO_WAIT);

    zassert_equal(k_thread_join(&consumerThread, K_MSEC(1000)), -EAGAIN);

    buf.push(input);

    zassert_equal(k_thread_join(&consumerThread, K_MSEC(1000)), 0);

    zassert_true(output.has_value());
    zassert_equal(output.value(), input);
}
