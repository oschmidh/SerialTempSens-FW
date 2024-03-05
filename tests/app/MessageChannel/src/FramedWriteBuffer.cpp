#include "FramedWriteBuffer.hpp"

#include <zephyr/ztest.h>

ZTEST_SUITE(framedWriteBuffer_tests, NULL, NULL, NULL, NULL, NULL);

ZTEST(framedWriteBuffer_tests, test_size)
{
    FramedWriteBuffer<32> buf;

    zassert_equal(buf.get_size(), 0);

    buf.push(42);
    zassert_equal(buf.get_size(), 1);

    buf.push(13);
    zassert_equal(buf.get_size(), 2);

    buf.push(13);
    zassert_equal(buf.get_size(), 3);
}

ZTEST(framedWriteBuffer_tests, test_frame)
{
    FramedWriteBuffer<32> buf;

    buf.push(42);
    buf.push(13);

    const auto ret = buf.frame();
    zassert_equal(ret.size(), 3);
    zassert_equal(ret[0], 2);
    zassert_equal(ret[1], 42);
    zassert_equal(ret[2], 13);
}
