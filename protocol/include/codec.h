#ifndef PROTOCOL_CODEC
#define PROTOCOL_CODEC

#include <cstdint>
#include <expected>
#include <span>
#include <vector>
#include <cstddef>

#include "messaging.h"

namespace protocol
{
    using Bytes = std::vector<std::byte>;

    struct EncodeError
    {
        std::string error;
    };

    struct DecodeError
    {
        std::string error;
    };

    // Payload length size is four bytes, so it need to be broken down; otherwise, when casting to std::byte the
    // top 3 bytes get cropped off; top-most byte gets pushed into array first.
    inline void put_u32(Bytes& b, std::uint32_t length)
    {
        for (int shift {24}; shift >= 0; shift -= 8)
        {
            b.push_back(static_cast<std::byte>(length >> shift));
        }
    }

    // convert string into bytes and add to byte array.
    inline void put_str(Bytes& bytes, const std::string& s)
    {
        const auto payloadBytes = std::as_bytes(std::span {s});
        bytes.insert(bytes.end(), payloadBytes.begin(), payloadBytes.end());
    }

    inline void write(Bytes&, const Ping&)
    { };

    inline void write(Bytes& b, const Login& msg)
    {
        put_u32(b, static_cast<std::uint32_t>(msg.user.length()));
        put_str(b, msg.user);
    }

    inline void write(Bytes& b, const Joined& msg)
    {
        put_u32(b, static_cast<std::uint32_t>(msg.user.length()));
        put_str(b, msg.user);
    }

    inline void write(Bytes& b, const MessageToGlobal& msg)
    {
        put_u32(b, msg.text.length());
        put_str(b, msg.text);
    }

    inline void write(Bytes& b, const MessageFromGlobal& msg)
    {
        put_u32(b, msg.user.length());
        put_str(b, msg.user);

        put_u32(b, msg.text.length());
        put_str(b, msg.text);
    }

    inline void write(Bytes& b, const MessageError& msg)
    {
        put_u32(b, msg.error.length());
        put_str(b, msg.error);
    }

    // [message type][payload length][payload]
    // 1 bytes       4 bytes          (payload length) bytes

    inline std::expected<Bytes, EncodeError> encode(const Message& msg)
    {
        Bytes out;
        out.push_back(static_cast<std::byte>(MessageToUint(msg)));

        std::visit([&o = out](const auto& x)
                {
                    write(o, x);
                },
                msg);

        return out;
    }

    inline std::expected<Message, DecodeError> decode(std::span<std::byte> in)
    {
        // TODO:
        // Get the message type.
        // Get the payload length.
        // Payload.
    }
}// namespace protocol
#endif
