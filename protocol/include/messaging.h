#ifndef PROTOCOL_MESSAGING_H
#define PROTOCOL_MESSAGING_H

#include <string>
#include <optional>
#include <variant>
#include <cstdint>

namespace protocol
{
    inline constexpr std::uint8_t k_MessageTypeCount {6};

    /*
     * Header containing all possible messages between Server and Client.
     */
    enum class MessageType: std::uint8_t
    {
        PING = 0,
        LOGIN,
        JOINED,
        MESSAGETOGLOBAL,
        MESSAGEFROMGLOBAL,
        ERROR
    };
    static_assert(static_cast<std::uint8_t>(MessageType::ERROR) == k_MessageTypeCount - 1);

    struct Ping
    {
        static constexpr MessageType type = MessageType::PING;
    };

    struct Login
    {
        static constexpr MessageType type = MessageType::LOGIN;
        std::string user;
    };

    struct Joined
    {
        static constexpr MessageType type = MessageType::JOINED;
        std::string user;
    };

    struct MessageToGlobal
    {
        static constexpr MessageType type = MessageType::MESSAGETOGLOBAL;
        std::string text;
    };

    struct MessageFromGlobal
    {
        static constexpr MessageType type = MessageType::MESSAGEFROMGLOBAL;
        std::string user;
        std::string text;
    };

    struct MessageError
    {
        static constexpr MessageType type = MessageType::ERROR;
        std::string error;
    };

    using Message = std::variant<Ping, Login, Joined, MessageToGlobal, MessageFromGlobal, MessageError>;
    static_assert(std::variant_size_v<Message> == k_MessageTypeCount, "Message types and variant size must match.");

    inline std::uint8_t MessageToUint(const Message& m)
    {
        return static_cast<std::uint8_t>(std::visit([](const auto& msg){return msg.type;}, m));
    }

    inline std::optional<Message> UintToMessage(std::uint8_t id)
    {
        if (id >= k_MessageTypeCount)
        {
            return std::nullopt;
        }

        MessageType type {id};
        switch (type)
        {
            case MessageType::PING:
                return Ping {};

            case MessageType::LOGIN:
                return Login {};

            case MessageType::JOINED:
                return Joined {};

            case MessageType::MESSAGETOGLOBAL:
                return MessageToGlobal {};

            case MessageType::MESSAGEFROMGLOBAL:
                return MessageFromGlobal {};

            case MessageType::ERROR:
                return MessageError {};
        }
    }

}// namespace protocol
#endif
