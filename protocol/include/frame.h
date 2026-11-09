#ifndef PROTOCOL_FRAME_H
#define PROTOCOL_FRAME_H

#include <cstdint>
#include <stdexcept>
#include <format>

#include "messaging.h"

namespace protocol
{
    inline constexpr std::uint32_t k_FrameHeaderSize {5};
    inline constexpr std::uint32_t k_MaxFrameSize    {1024};

    /*
     * Frame header parses the first k_FrameHeaderSize bytes; modifying the byte stream.
     */
    struct FrameHeader
    {
        std::uint8_t  _messageType;
        std::uint32_t _frameSize;

        FrameHeader(std::span<const std::byte>& bytes)
        {
            if (bytes.size() < k_FrameHeaderSize)
            {
                throw std::runtime_error(std::format("Frame header needs {} bytes got {}.", k_FrameHeaderSize, bytes.size()));
            }

            auto _header = bytes.first(k_FrameHeaderSize);

            _messageType = static_cast<std::uint8_t>(_header[0]);
            _frameSize   = 0u
            | (std::to_integer<std::uint32_t>(_header[1]) << 24)
            | (std::to_integer<std::uint32_t>(_header[2]) << 16)
            | (std::to_integer<std::uint32_t>(_header[3]) << 8)
            |  std::to_integer<std::uint32_t>(_header[4]);

            if (_frameSize > k_MaxFrameSize)
            {
                throw std::length_error(std::format("Frame overflow, got: {} expected: <{}", _frameSize, k_MaxFrameSize));
            }
            // Note: _frameSize only counts network packets, not frame header.
            else if (_frameSize > (bytes.size() - k_FrameHeaderSize))
            {
                throw std::length_error(std::format("Not enough bytes in stream, expected: {} got: {}", _frameSize, bytes.size()));
            }
            else if (_frameSize < (bytes.size() - k_FrameHeaderSize))
            {
                throw std::length_error(std::format("More bytes in stream then expected, expected: {} got: {}", _frameSize, bytes.size()));
            }

            bytes = bytes.subspan(k_FrameHeaderSize);
        }

        Message GetMessageObject() const
        {
            auto messgeOption = UintToMessage(_messageType);
            if (!messgeOption)
            {
                throw std::invalid_argument("Message type invalid.");
            }

            return messgeOption.value();
        }

        std::vector<std::byte> GetFrameBytes() const
        {
            std::vector<std::byte> v;
            v.push_back(static_cast<std::byte>(_messageType));

            v.push_back(static_cast<std::byte>(_frameSize >> 24));
            v.push_back(static_cast<std::byte>(_frameSize >> 16));
            v.push_back(static_cast<std::byte>(_frameSize >> 8));
            v.push_back(static_cast<std::byte>(_frameSize));

            return v;
        }
    };

}// namespace protocol
#endif
