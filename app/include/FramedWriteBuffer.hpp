#ifndef SERIALTEMPSENS_FW_APP_INCLUDE_FRAMEDWRITEBUFFER_H
#define SERIALTEMPSENS_FW_APP_INCLUDE_FRAMEDWRITEBUFFER_H

#include <WriteBufferInterface.h>

#include <algorithm>
#include <array>
#include <span>
#include <cstdint>

template <std::size_t SIZE_V = 32>
class FramedWriteBuffer final : public ::EmbeddedProto::WriteBufferInterface {
  public:
    constexpr FramedWriteBuffer() noexcept = default;
    ~FramedWriteBuffer() noexcept final = default;

    constexpr void clear() noexcept final { _idx = 0; };
    constexpr uint32_t get_size() const noexcept final { return _idx; };
    constexpr uint32_t get_max_size() const noexcept final { return _framedBuf.buf.size(); };
    constexpr uint32_t get_available_size() const noexcept final { return get_max_size() - get_size(); };

    constexpr bool push(const uint8_t byte) noexcept final
    {
        if (get_available_size() == 0) {
            return false;
        }
        _framedBuf.buf[_idx++] = byte;
        return true;
    };

    constexpr bool push(const uint8_t* bytes, const uint32_t length) noexcept final
    {
        if (get_available_size() <= length) {
            return false;
        }
        std::copy_n(bytes, length, &_framedBuf.buf[_idx]);
        _idx += length;
        return true;
    };

    constexpr std::span<const std::uint8_t> frame() noexcept
    {
        _framedBuf.len = get_size();
        return {reinterpret_cast<std::uint8_t*>(&_framedBuf), get_size() + 1};    // HACK ?
    }

  private:
    std::size_t _idx{};

    struct __attribute__((packed)) {
        std::uint8_t len;
        std::array<std::uint8_t, SIZE_V> buf;
    } _framedBuf;
    static_assert(sizeof(_framedBuf) == SIZE_V + 1);
};

#endif    // SERIALTEMPSENS_FW_APP_INCLUDE_FRAMEDWRITEBUFFER_H
