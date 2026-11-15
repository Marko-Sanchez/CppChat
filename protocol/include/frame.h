#ifndef PROTOCOL_FRAME_H
#define PROTOCOL_FRAME_H

#include <cstdint>
#include <stdexcept>
#include <format>

#include "byteprimitives.h"
#include "messaging.h"

namespace protocol
{
    /*
     * Handles Frame Header contents: Encoding, Decoding, and validating header content.
     */
    inline constexpr std::uint32_t k_FrameHeaderSize {5};
    inline constexpr std::uint32_t k_MaxFrameSize    {1024};

    inline Message GetMessageObject(std::uint8_t messageType)
    {
        if (auto messageOption = UintToMessage(messageType); messageOption)
        {
            return std::move(messageOption.value());
        }

        throw std::invalid_argument("Message type invalid.");
    }

    inline std::pair<std::uint8_t, std::uint32_t> read_frame_header(std::span<const std::byte> b)
    {
        if (b.size() < k_FrameHeaderSize)
        {
            throw std::runtime_error(std::format("Frame header needs {} bytes got {}.", k_FrameHeaderSize, b.size()));
        }

        const std::uint8_t messageType = static_cast<std::uint8_t>(b[0]);
        const std::uint32_t frameSize  = read_u32(b.subspan(1));

        GetMessageObject(messageType);

        if (frameSize > k_MaxFrameSize)
        {
            throw std::length_error(std::format("Frame overflow, got: {} expected: <{}", frameSize, k_MaxFrameSize));
        }
        if (auto buffersize = static_cast<std::uint32_t>(b.size()) - k_FrameHeaderSize; buffersize != frameSize)
        {
            throw std::length_error(std::format("Buffer ({}) frame size ({}) mismatch.", buffersize, frameSize));
        }

        return {messageType, frameSize};
    }

    inline std::vector<std::byte> write_frame_header(std::uint8_t type, std::uint32_t framesize)
    {
        std::vector<std::byte> out;

        out.reserve(k_FrameHeaderSize);
        out.push_back(static_cast<std::byte>(type));
        out.append_range(write_u32(framesize));

        return out;
    }

}// namespace protocol
#endif
