#include <myLib/RingBuffer.hpp>

#include <myLib/Mutex.hpp>

#include <zephyr/ztest.h>

ZTEST_SUITE(ringBuffer_tests, NULL, NULL, NULL, NULL, NULL);

ZTEST(ringBuffer_tests, test_index_prefix_increment_decrement)
{
    myLib::internal::RingIndex<4> idx;

    zassert_equal(idx, 0);
    zassert_equal(++idx, 1);
    zassert_equal(++idx, 2);

    zassert_equal(idx, 2);
    zassert_equal(--idx, 1);
    zassert_equal(--idx, 0);
}

ZTEST(ringBuffer_tests, test_index_postfix_increment_decrement)
{
    myLib::internal::RingIndex<4> idx;

    zassert_equal(idx++, 0);
    zassert_equal(idx++, 1);
    zassert_equal(idx, 2);

    zassert_equal(idx--, 2);
    zassert_equal(idx--, 1);
    zassert_equal(idx, 0);
}

ZTEST(ringBuffer_tests, test_index_increment_wraparound)
{
    static constexpr std::size_t size = 3;
    myLib::internal::RingIndex<size> idx;

    for (unsigned int i = 0; i < (size - 1); ++i) {
        ++idx;
    }
    zassert_equal(idx, size - 1, "is: %d", idx);
    ++idx;
    zassert_equal(idx, 0);
}

ZTEST(ringBuffer_tests, test_index_increment_wraparound_powerOf2)    // TODO ??
{
    static constexpr std::size_t size = 8;
    myLib::internal::RingIndex<size> idx;

    for (unsigned int i = 0; i < (size - 1); ++i) {
        ++idx;
    }
    zassert_equal(idx, size - 1);
    ++idx;
    zassert_equal(idx, 0);
}

ZTEST(ringBuffer_tests, test_index_decrement_wraparound)
{
    static constexpr std::size_t size = 8;
    myLib::internal::RingIndex<size> idx;

    --idx;
    zassert_equal(idx, size - 1);
}

ZTEST(ringBuffer_tests, test_buffer_empty)
{
    myLib::RingBuffer<int, 8> buf;

    zassert_true(buf.isEmpty());

    buf.push(42);
    zassert_false(buf.isEmpty());

    buf.pull();
    zassert_true(buf.isEmpty());
}

ZTEST(ringBuffer_tests, test_empty_pull)
{
    myLib::RingBuffer<int, 8> buf;

    zassert_true(buf.isEmpty());

    const auto ret = buf.pull();
    zassert_false(ret.has_value());
}

ZTEST(ringBuffer_tests, test_buffer_full)
{
    myLib::RingBuffer<int, 4> buf;

    zassert_false(buf.isFull());

    buf.push(42);
    zassert_false(buf.isFull());
    buf.push(42);
    zassert_false(buf.isFull());

    buf.push(42);
    zassert_true(buf.isFull());

    buf.pull();
    zassert_false(buf.isFull());

    buf.push(42);
    zassert_true(buf.isFull());
}

ZTEST(ringBuffer_tests, test_cnt)
{
    myLib::RingBuffer<int, 8> buf;

    unsigned int i = 0;
    while (!buf.isFull()) {
        zassert_equal(buf.cnt(), i);
        buf.push(86);
        ++i;
    }

    while (!buf.isEmpty()) {
        zassert_equal(buf.cnt(), i, "%d vs %d", buf.cnt(), i);
        buf.pull();
        --i;
    }
}

ZTEST(ringBuffer_tests, test_push_and_pull)
{
    {
        myLib::RingBuffer<int, 8> buf;

        zassert_true(buf.push(42));
        zassert_equal(buf.pull(), 42);
    }
}

ZTEST(ringBuffer_tests, test_wraparound)
{
    myLib::RingBuffer<int, 4> buf;

    zassert_true(buf.isEmpty());

    zassert_true(buf.push(5));
    zassert_true(buf.push(30));
    zassert_true(buf.push(186));

    zassert_true(buf.isFull());

    zassert_equal(buf.pull(), 5);
    zassert_equal(buf.pull(), 30);

    zassert_true(buf.push(99));
    zassert_true(buf.push(369));

    zassert_true(buf.isFull());

    zassert_equal(buf.pull(), 186);
    zassert_equal(buf.pull(), 99);
    zassert_equal(buf.pull(), 369);

    zassert_true(buf.isEmpty());
}

ZTEST(ringBuffer_tests, test_items_order)
{
    const std::array<int, 4> expected = {1, 2, 3, 4};

    myLib::RingBuffer<int, 4> buf;

    for (int v : expected) {
        buf.push(v);
    }

    std::array<int, 4> actual;
    for (int& v : actual) {
        v = buf.pull().value_or(0);
    }

    zassert_mem_equal(expected.data(), actual.data(), expected.size());
}

static constexpr int threadPrio = 0;
static constexpr int stackSize = 1024;

using RingbufferType = myLib::RingBuffer<int, 1024, myLib::Policies::ThreadSafe<myLib::Mutex>>;
// using RingbufferType = myLib::RingBuffer<int, 1024>;

void producerTask(void* buffer, void*, void*) noexcept
{
    RingbufferType* buf = reinterpret_cast<RingbufferType*>(buffer);

    for (int i = 0; i < 8192; ++i) {
        // for (int i = 0; i < 60000; ++i) {
        // if (buf->isFull()) {
        while (buf->isFull()) {
            // k_busy_wait(7000);
            k_yield();
        }
        // } else {

        //     k_yield();
        // }
        zassert_true(buf->push(i));
    }
}

void consumerTask(void* buffer, void*, void*) noexcept
{
    RingbufferType* buf = reinterpret_cast<RingbufferType*>(buffer);

    for (int i = 0; i < 8192; ++i) {
        // for (int i = 0; i < 30000; ++i) {
        // if (buf->isEmpty()) {
        while (buf->isEmpty()) {
            // k_busy_wait(11000);
            k_yield();
            // printk("yield");
        }
        // } else {

        //     k_yield();
        // }

        const auto ret = buf->pull();
        zassert_true(ret.has_value());
        // zassert_equal(ret.value(), i, "%d vs %d", ret.value(), i);
    }
}

K_THREAD_STACK_DEFINE(producerStack, stackSize);
K_THREAD_STACK_DEFINE(consumerStack, stackSize);

ZTEST(ringBuffer_tests, test_thread_safety)
{
    RingbufferType buf;

    k_thread producerThread{};
    k_thread_create(&producerThread, producerStack, K_THREAD_STACK_SIZEOF(producerStack), producerTask,
                    reinterpret_cast<void*>(&buf), NULL, NULL, threadPrio, 0, K_NO_WAIT);

    k_thread consumerThread{};
    k_thread_create(&consumerThread, consumerStack, K_THREAD_STACK_SIZEOF(consumerStack), consumerTask,
                    reinterpret_cast<void*>(&buf), NULL, NULL, threadPrio, 0, K_NO_WAIT);

    k_thread_join(&producerThread, K_FOREVER);
    k_thread_join(&consumerThread, K_FOREVER);
}
