#ifndef SERIALTEMPSENS_FW_APP_INCLUDE_MESSAGECHANNEL_H
#define SERIALTEMPSENS_FW_APP_INCLUDE_MESSAGECHANNEL_H

#include "UartReceiveBuffer.hpp"
#include "FramedWriteBuffer.hpp"
#include "Framer.hpp"
#include <ReadBufferFixedSize.h>

#include <myLib/Uart.hpp>

#include <type_traits>

static_assert(IS_ENABLED(CONFIG_MESSAGECHANNEL), "Messagechannel KConfig not enabled");

namespace internal {

template <typename>
struct FunctionTraits;

template <typename RET_T, typename FUNC_T, typename PARAM_T>
struct FunctionTraits<RET_T (FUNC_T::*)(PARAM_T)> {
    using ReturnType = RET_T;
    using ParamType = std::decay_t<PARAM_T>;
};

template <typename RET_T, typename FUNC_T, typename PARAM_T>
struct FunctionTraits<RET_T (FUNC_T::*)(PARAM_T) noexcept> {
    using ReturnType = RET_T;
    using ParamType = std::decay_t<PARAM_T>;
};

template <typename RET_T, typename FUNC_T, typename PARAM_T>
struct FunctionTraits<RET_T (FUNC_T::*)(PARAM_T) const> {
    using ReturnType = RET_T;
    using ParamType = std::decay_t<PARAM_T>;
};

template <typename RET_T, typename FUNC_T, typename PARAM_T>
struct FunctionTraits<RET_T (FUNC_T::*)(PARAM_T) const noexcept> {
    using ReturnType = RET_T;
    using ParamType = std::decay_t<PARAM_T>;
};

template <typename LAMBDA_T>
struct LambdaTraits {
    using FunctionType = decltype(&LAMBDA_T::operator());
    using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
    using ParamType = typename FunctionTraits<FunctionType>::ParamType;
};

}    // namespace internal

class MessageChannel {
  public:
    MessageChannel(const device* const dev) noexcept
     : _uart(dev, _buf)
    { }

    bool init() noexcept { return _uart.init(); }
    void start() noexcept { _uart.enableRx(); }
    void stop() noexcept { _uart.disableRx(); }

    template <typename PROCESS_FUNC_T>
    [[noreturn]] void run(PROCESS_FUNC_T process) noexcept
    {
        using MessageType = typename internal::LambdaTraits<PROCESS_FUNC_T>::ParamType;

        while (1) {
            const auto msg = receive<MessageType>();

            if (msg.has_value()) {
                send(process(msg.value()));
            }
        }
    }

  private:
    bool send(auto msg) noexcept
    {
        FramedWriteBuffer writeBuf;

        if (msg.serialize(writeBuf) != ::EmbeddedProto::Error::NO_ERRORS) {
            return false;
        }

        // _uart.send(writeBuf.frame());    // TODO switch when ready
        _uart.sendPolling(writeBuf.frame());
        return true;
    }

    template <typename MSG_T>
    std::optional<MSG_T> receive() noexcept
    {
        EmbeddedProto::ReadBufferFixedSize<32> readBuf;
        Framer framer(readBuf);

        do {
            const auto byte = _buf.pull(std::chrono::milliseconds(CONFIG_MESSAGECHANNEL_RECEIVE_TIMEOUT));
            if (!byte.has_value()) {
                return std::nullopt;
            }

            framer.deframe(byte.value());
        } while (!framer.msgComplete());

        MSG_T receivedMsg;
        if (receivedMsg.deserialize(readBuf) != ::EmbeddedProto::Error::NO_ERRORS) {
            return std::nullopt;
        }

        return receivedMsg;
    }

    using UartReceiveBufferType = UartReceiveBuffer<CONFIG_MESSAGECHANNEL_UART_BUFFER_SIZE>;

    UartReceiveBufferType _buf{};
    Uart<UartReceiveBufferType> _uart;
};

#endif    // SERIALTEMPSENS_FW_APP_INCLUDE_MESSAGECHANNEL_H
