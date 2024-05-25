#ifndef SERIALTEMPSENS_FW_APP_INCLUDE_FRAMER_H
#define SERIALTEMPSENS_FW_APP_INCLUDE_FRAMER_H

#include <cstdint>

template <typename BUFFER_T>
class Framer {
  public:
    constexpr Framer(BUFFER_T& buf) noexcept
     : _buf(buf)
    { }

    void deframe(std::uint8_t b) noexcept
    {
        switch (_state) {
            case State::Header:
                _len = b;
                _state = State::Data;
                break;

            case State::Data:
                if (_len-- <= 0) {
                    return;    // TODO error?
                }

                _buf.push(b);
                break;
        }
    }

    constexpr bool msgComplete() const noexcept { return _len <= 0; }

  private:
    enum class State { Header, Data };

    BUFFER_T& _buf;
    State _state = State::Header;
    std::size_t _len = -1;
};

#endif    // SERIALTEMPSENS_FW_APP_INCLUDE_FRAMER_H
