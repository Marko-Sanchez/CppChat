#ifndef PROTOCOL_BYTEPRIMITIVES_H
#define PROTOCOL_BYTEPRIMITIVES_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <span>
#include <string>

namespace protocol
{
    inline std::vector<std::byte> write_u32(std::uint32_t value)
    {
        return std::vector<std::byte>
        {
            static_cast<std::byte>(value >> 24),
            static_cast<std::byte>(value >> 16),
            static_cast<std::byte>(value >> 8),
            static_cast<std::byte>(value)
        };
    }

    inline void write_u32(std::vector<std::byte>& b, std::uint32_t value)
    {
        b.push_back(static_cast<std::byte>(value >> 24));
        b.push_back(static_cast<std::byte>(value >> 16));
        b.push_back(static_cast<std::byte>(value >> 8));
        b.push_back(static_cast<std::byte>(value));
    }

    inline std::uint32_t read_u32(std::span<const std::byte> b)
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

    inline std::vector<std::byte> write_str(const std::string& str)
    {
        std::vector<std::byte> v = write_u32(static_cast<std::uint32_t>(str.length()));
        v.append_range(std::as_bytes(std::span {str}));

        return v;
    }

    inline void write_str(std::vector<std::byte>& b, const std::string& str)
    {
        b.append_range(write_u32(static_cast<std::uint32_t>(str.length())));
        b.append_range(std::as_bytes(std::span {str}));
    }

    inline std::pair<std::uint32_t, std::string> read_str(std::span<const std::byte> b)
    {
        const std::uint32_t lengthByteCount {4};
        if (b.size() < lengthByteCount)
        {
            throw std::runtime_error("read_str: buffer to small for length prefix.");
        }

        std::uint32_t length = read_u32(b);
        if (length > (b.size() - lengthByteCount))
        {
            throw std::runtime_error("read_str: buffer overflow.");
        }

        return {lengthByteCount + length, std::string {reinterpret_cast<const char*>(b.data() + lengthByteCount), length}};
    }
}// namespace protocol
#endif
