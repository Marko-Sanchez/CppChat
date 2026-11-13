#ifndef PROTOCOL_CODEC_H
#define PROTOCOL_CODEC_H

#include <cstdint>
#include <expected>
#include <span>
#include <vector>
#include <cstddef>

#include "messaging.h"
#include "byteserdes.h"

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

    inline void write(ByteWriter&, const Ping&)
    { };

    inline void write(ByteWriter& bw, const Login& msg)
    {
        bw.WriteField(msg.user);
    }

    inline void write(ByteWriter& bw, const Joined& msg)
    {
        bw.WriteField(msg.user);
    }

    inline void write(ByteWriter& bw, const MessageToGlobal& msg)
    {
        bw.WriteField(msg.text);
    }

    inline void write(ByteWriter& bw, const MessageFromGlobal& msg)
    {
        bw.WriteField(msg.user);
        bw.WriteField(msg.text);
    }

    inline void write(ByteWriter& bw, const MessageError& msg)
    {
        bw.WriteField(msg.error);
    }

    inline void read(ByteReader& br, Ping& msg)
    { }

    inline void read(ByteReader& br, Login& msg)
    {
        msg.user = br.ReadField();
    }

    inline void read(ByteReader& br, Joined& msg)
    {
        msg.user = br.ReadField();
    }

    inline void read(ByteReader& br, MessageToGlobal& msg)
    {
        msg.text = br.ReadField();
    }

    inline void read(ByteReader& br, MessageFromGlobal& msg)
    {
        msg.user = br.ReadField();
        msg.text = br.ReadField();
    }

    inline void read(ByteReader& br, MessageError& msg)
    {
        msg.error = br.ReadField();
    }

    inline std::expected<Bytes, EncodeError> encode(const Message& msg)
    {
        try
        {
            ByteWriter bytewriter (msg);
            std::visit([&bw = bytewriter](const auto& x)
                    {
                        write(bw, x);
                    },
                    msg);

            return bytewriter.GetBytes();
        }
        catch (const std::exception& error)
        {
            return std::unexpected(EncodeError{.error = error.what()});
        }
    }

    inline std::expected<Message, DecodeError> decode(std::span<const std::byte> in)
    {
        try
        {
            ByteReader bytereader (in);
            auto message = bytereader.CreateMessageObject();
            std::visit([&br = bytereader](auto& x)
                    {
                        read(br, x);
                    },
                    message);

            return std::move(message);
        }
        catch (const std::exception& error)
        {
            return std::unexpected(DecodeError{.error = error.what()});
        }
    }
}// namespace protocol
#endif
