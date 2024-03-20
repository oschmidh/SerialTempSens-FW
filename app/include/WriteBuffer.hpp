#ifndef APP_INC_UARTWRITEBUFFER_H
#define APP_INC_UARTWRITEBUFFER_H

#include <WriteBufferInterface.h>

#include <algorithm>
#include <array>
#include <cstdint>

template <std::size_t SIZE_V = 32>
class WriteBuffer final : public ::EmbeddedProto::WriteBufferInterface {
  public:
    constexpr WriteBuffer() noexcept = default;
    ~WriteBuffer() noexcept final = default;

    constexpr void clear() noexcept final { _idx = 0; };
    constexpr uint32_t get_size() const noexcept final { return _idx; };
    constexpr uint32_t get_max_size() const noexcept final { return _buf.size(); };
    constexpr uint32_t get_available_size() const noexcept final { return get_max_size() - get_size(); };
    constexpr const uint8_t* cbegin() const noexcept { return &_buf[0]; }
    constexpr uint8_t* begin() noexcept { return &_buf[0]; }
    constexpr const uint8_t* cend() const noexcept { return &_buf[_idx]; }
    constexpr uint8_t* end() noexcept { return &_buf[_idx]; }

    constexpr bool push(const uint8_t byte) noexcept final
    {
        if (get_available_size() == 0) {
            return false;
        }
        _buf[_idx++] = byte;
        return true;
    };

    constexpr bool push(const uint8_t* bytes, const uint32_t length) noexcept final
    {
        if (get_available_size() <= length) {
            return false;
        }
        std::copy_n(bytes, length, &_buf[_idx]);
        _idx += length;
        return true;
    };

  private:
    std::size_t _idx{};
    std::array<std::uint8_t, SIZE_V> _buf;
};

#endif    // APP_INC_UARTWRITEBUFFER_H
