#ifndef PROTOCOL_BYTESERDES_H
#define PROTOCOL_BYTESERDES_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>
#include <span>
#include <ranges>

#include "byteprimitives.h"
#include "frame.h"
#include "messaging.h"

namespace protocol
{

// [message type][total length][payload length][payload]
// 1 bytes       4 bytes        4 bytes        x bytes

class ByteReader
{
    private:

        std::span<const std::byte> _in;
        std::uint32_t              _position;
        std::uint8_t               _messageType;

    public:

        explicit ByteReader(std::span<const std::byte> bytes):
        _in(bytes)
        {
            auto [type, size] = read_frame_header(_in);
            _messageType = type;
            _position    = k_FrameHeaderSize;
        }

        std::string ReadField()
        {
            auto [consumed, payload] = read_str(_in.subspan(_position));
            _position += consumed;

            return std::move(payload);
        }

        Message CreateMessageObject() const
        {
            return GetMessageObject(_messageType);
        }
};

class ByteWriter
{
    private:

        std::uint8_t           _messageType;
        std::vector<std::byte> _out;

    public:

        ByteWriter(const Message& message):
        _messageType(MessageToUint(message)),
        _out(k_FrameHeaderSize)
        {}

        void WriteField(const std::string& field)
        {
            write_str(_out, field);
        }

        std::vector<std::byte> GetBytes()
        {
            std::uint32_t framesize = _out.size() - k_FrameHeaderSize;

            std::ranges::copy(write_frame_header(_messageType, framesize), _out.begin());
            return std::move(_out);
        }
};
}// namespace
#endif
