#include "UartReceiveBuffer.hpp"

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

ZTEST_SUITE(uartReceiveBuffer_tests, NULL, NULL, NULL, NULL, NULL);

ZTEST(uartReceiveBuffer_tests, test_push_pull)
{
    UartReceiveBuffer buf{};

    zassert_true(buf.push(5));
    const auto ret = buf.pull();

    zassert_true(ret.has_value());
    zassert_equal(ret.value(), 5);
}

ZTEST(uartReceiveBuffer_tests, test_push_pull_multiple)
{
    const std::uint8_t testData[] = {1, 2, 3, 4, 5};
    UartReceiveBuffer buf{};

    for (std::uint8_t d : testData) {
        zassert_true(buf.push(d));
    }

    for (int i = 0; i < testData.size(); ++i) {
        const auto ret = buf.pull();
        zassert_true(ret.has_value());
        zassert_equal(ret.value(), testData[i]);
    }
}

static constexpr int threadPrio = 0;
static constexpr int stackSize = 1024;

void producerTask(void* buffer, void* inp, void*)
{
    UartReceiveBuffer* buf = reinterpret_cast<UartReceiveBuffer*>(buffer);
    const std::uint8_t* inp = reinterpret_cast<std::uint8_t*>(inp);

    zassert_true(buffer->push(*inp));
}

void consumerTask(void* buffer, void* outp, void*)
{
    UartReceiveBuffer* buf = reinterpret_cast<UartReceiveBuffer*>(buffer);

    std::optional<std::uint8_t>* out = reinterpret_cast<std::optional<std::uint8_t>*>(outp);
    *out = buffer->pull();
}

ZTEST(ringBuffer_tests, test_pull_blocking)
{
    const std::uint8_t input = 77;
    UartReceiveBuffer buf{};
    std::optional<std::uint8_t> output;

    K_THREAD_DEFINE(consumerThread, stackSize, consumerTask, reinterpret_cast<void*>(&buf),
                    reinterpret_cast<void*>(&output), NULL, threadPrio, 0, 0);

    zassert_false(k_thread_join(consumerThread, K_MSEC(1000)));

    K_THREAD_DEFINE(producerThread, stackSize, producerTask, reinterpret_cast<void*>(&buf),
                    reinterpret_cast<void*>(&input), NULL, threadPrio, 0, 0);

    zassert_true(k_thread_join(producerThread, K_MSEC(1000)));
    zassert_true(k_thread_join(consumerThread, K_MSEC(1000)));

    zassert_true(output.has_value());
    zassert_equal(output.value(), input);
}
