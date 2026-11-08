#ifndef PROTOCOL_BYTESERDES_H
#define PROTOCOL_BYTESERDES_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>
#include <span>

namespace protocol
{
namespace
{
    struct ParsedString
    {
        std::uint32_t consumed;
        std::string value;
    };

    inline void put_u32(std::vector<std::byte>& b, std::uint32_t length)
    {
        b.push_back(static_cast<std::byte>(length >> 24));
        b.push_back(static_cast<std::byte>(length >> 16));
        b.push_back(static_cast<std::byte>(length >> 8));
        b.push_back(static_cast<std::byte>(length));
    }

    inline void put_str(std::vector<std::byte>& bytes, const std::string& s)
    {
        put_u32(bytes, s.length());

        const auto payloadBytes = std::as_bytes(std::span {s});
        bytes.insert(bytes.end(), payloadBytes.begin(), payloadBytes.end());
    }

    inline std::uint32_t get_u32(std::span<const std::byte> b)
    {
        return
        {
            0u
            | (std::to_integer<std::uint32_t>(b[0]) << 24)
            | (std::to_integer<std::uint32_t>(b[1]) << 16)
            | (std::to_integer<std::uint32_t>(b[2]) << 8)
            | std::to_integer<std::uint32_t>(b[3])
        };
    }

    inline ParsedString get_str(std::span<const std::byte>& b)
    {
        auto length {get_u32(b)};
        if (length > b.size())
        {
            throw std::runtime_error("get_str: buffer underflow.");
        }

        std::string payload (reinterpret_cast<const char*>(b.data() + 4), length);

        return {4u + length, std::move(payload)};
    }
}// anonymous namespace

class ByteReader
{
    private:

        std::uint32_t              _position;
        std::span<const std::byte> _in;

    public:

        ByteReader(std::span<const std::byte> bytes):
        _in(bytes)
        {}

        std::string ReadField()
        {
            auto [consumed, payload] = get_str(_in.subspan(_position));
            _position += consumed;

            return payload;
        }

};

class ByteWriter
{
    private:

        std::vector<std::byte> _out;

    public:

        ByteWriter()
        {}

        void WriteField(const std::string& field)
        {
            put_str(_out, field);
        }
};
}// namespace
#endif
