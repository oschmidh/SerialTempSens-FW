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

    bool send(auto msg) noexcept
    {
        // Framer<WriteBuffer>
        // WriteBuffer writeBuf;
        FramedWriteBuffer writeBuf;

        // printk("sending rply\n");

        if (msg.serialize(writeBuf) != ::EmbeddedProto::Error::NO_ERRORS) {
            // printk("failed to serialize rply\n");
            return false;
        }

        // _uart.send(writeBuf.frame());    // TODO switch when ready
        _uart.sendPolling(writeBuf.frame());
        return true;
    }

    // template <typename MSG_T>
    // MSG_T receiveBlocking() noexcept    // TEST
    // {
    //     ReadBufferType readBuf;
    //     Framer framer(readBuf);
    //     m_uart.enableRx(&framer.unframe);

    //     do {
    //         // framer.unframe(b);

    //         // yield??
    //     } while (!framer.msgComplete());

    //     // ReadBufferType readBuf = framer.getMsg();
    //     MSG_T receivedMsg;
    //     if (receivedMsg.deserialize(readBuf) != ::EmbeddedProto::Error::NO_ERRORS) {
    //         return {};    // TODO ??
    //     }
    //     m_uart.disableRx();
    //     return receivedMsg;
    // }

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
            // printk("unframed\n");
        } while (!framer.msgComplete());
        // printk("rec done\n");

        MSG_T receivedMsg;
        if (receivedMsg.deserialize(readBuf) != ::EmbeddedProto::Error::NO_ERRORS) {
            return std::nullopt;
        }

        return receivedMsg;
    }

    template <typename PROCESS_FUNC_T>
    void run(PROCESS_FUNC_T process)
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
    // void receive(std::uint8_t b) noexcept
    // {
    //     _framer.unframe(b);
    //     if (!_framer.msgComplete()) {
    //         return;
    //     }
    //     // msg = _framer.getMsg();
    //     k_msgq_put(&_framer.getMsg(), &msg_buf, K_NO_WAIT);

    //     _framer.reset();    // TODO RAII??
    // }

    // void receive(std::uint8_t b) noexcept { _framer.unframe(b); }

    // Framer _framer;
    // Uart _uart{receive};
    UartReceiveBuffer<32> _buf{};
    Uart<UartReceiveBuffer<32>> _uart;
};

#endif    // SERIALTEMPSENS_FW_APP_INCLUDE_MESSAGECHANNEL_H
