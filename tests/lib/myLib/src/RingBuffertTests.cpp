#include <myLib/RingBuffer.hpp>

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
