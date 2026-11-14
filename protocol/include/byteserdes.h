#ifndef PROTOCOL_BYTESERDES_H
#define PROTOCOL_BYTESERDES_H

#include <cstdint>
#include <vector>
#include <string>
#include <span>

#include "byteprimitives.h"
#include "frame.h"
#include "messaging.h"

/*
 * Converts stream of std::byte into 'Message' objects and 'Message' objects into a stream of std::byte.
 * Appended to a Frame header, first k_FrameHeaderSize bytes, that contain the message type and length of the stream of bytes.
 *
 * Formatted in the following way:
 *
 * [message type][total length][payload length][payload]
 * 1 bytes       4 bytes        4 bytes        x bytes
 */
namespace protocol
{
/*
 * ByteReader converts a stream of std::byte into a data structure, Deserialization.
 * The first k_FrameHeaderSize bytes are read to obtain message type and length; After,
 * of which, for any number of fields their payload length and payload are extracted.
 * The position in the stream is tracked with a position value.
 */
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
            _position    = k_FrameHeaderSize;
            _messageType = type;
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

/*
 * ByteWriter converts a data structure into a stream of std::byte, Serialization.
 * Reserves the first k_FrameHeaderSize, where it will place the type of message and
 * length of the stream, not including the header. Data structure member variables length and contents
 * are converted to std::byte and appeneded to the stream of bytes.
 */
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

        std::vector<std::byte> GetBytes() &&
        {
            std::uint32_t framesize = static_cast<std::uint32_t>(_out.size()) - k_FrameHeaderSize;

            std::ranges::copy(write_frame_header(_messageType, framesize), _out.begin());
            return std::move(_out);
        }
};
}// namespace protocol
#endif
