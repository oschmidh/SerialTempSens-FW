#ifndef SERIALTEMPSENS_FW_APP_INCLUDE_MESSAGECHANNEL_H
#define SERIALTEMPSENS_FW_APP_INCLUDE_MESSAGECHANNEL_H

#include "Uart.hpp"
#include "FramedWriteBuffer.hpp"
#include <ReadBufferFixedSize.h>

#include <myLib/Framer.hpp>

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
    MSG_T receive() noexcept    // TEST
    {
        EmbeddedProto::ReadBufferFixedSize<32> readBuf;
        Framer framer(readBuf);

        do {
            const std::uint8_t b = _buf.pull();
            framer.deframe(b);
            // printk("unframed\n");
        } while (!framer.msgComplete());
        // printk("rec done\n");

        MSG_T receivedMsg;
        if (receivedMsg.deserialize(readBuf) != ::EmbeddedProto::Error::NO_ERRORS) {
            return {};    // TODO ??
        }

        return receivedMsg;
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
    UartBuffer<32> _buf{};
    Uart<UartBuffer<32>> _uart;
};

#endif    // SERIALTEMPSENS_FW_APP_INCLUDE_MESSAGECHANNEL_H
