#include "Framer.hpp"

#include <zephyr/ztest.h>

#include <array>

template <typename T, std::size_t SIZE_V>
class WriteBuf {
  public:
    bool push(T item) noexcept
    {
        if (size() >= SIZE_V) {
            return false;
        }

        _buf[_idx++] = item;
        return true;
    }

    void clear() noexcept { _idx = 0; }
    std::size_t size() const noexcept { return _idx; }
    T get(std::size_t i) const noexcept { return _buf[i]; }

  private:
    std::size_t _idx{};
    std::array<T, SIZE_V> _buf{};
};

ZTEST_SUITE(framer_tests, NULL, NULL, NULL, NULL, NULL);

ZTEST(framer_tests, test_init_msgNotComplete)
{
    WriteBuf<std::uint8_t, 32> buf;
    Framer framer(buf);

    zassert_false(framer.msgComplete());
}

ZTEST(framer_tests, test_deframe_msgCompleteSet)
{
    WriteBuf<std::uint8_t, 32> buf;
    Framer framer(buf);

    framer.deframe(2);
    zassert_false(framer.msgComplete());

    framer.deframe(5);
    zassert_false(framer.msgComplete());

    framer.deframe(8);
    zassert_true(framer.msgComplete());
}

ZTEST(framer_tests, test_deframe_bufDataCorrect)
{
    WriteBuf<std::uint8_t, 32> buf;
    Framer framer(buf);

    framer.deframe(2);
    framer.deframe(5);
    framer.deframe(8);

    zassert_equal(buf.size(), 2);
    zassert_equal(buf.get(0), 5);
    zassert_equal(buf.get(1), 8);
}
