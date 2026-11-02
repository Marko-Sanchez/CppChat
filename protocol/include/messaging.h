#ifndef PROTOCOL_MESSAGING
#define PROTOCOL_MESSAGING

#include <string>
#include <variant>

namespace protocol
{
    /*
     * Header containing all possible messages between Server and Client.
     */
    enum class MessageType
    {
        PING,
        LOGIN,
        JOINED,
        MESSAGETOGLOBAL,
        MESSAGEFROMGLOBAL,
        ERROR
    };

    struct Ping
    {

    };

    struct Login
    {
        std::string user;
    };

    struct Joined
    {
        std::string user;
    };

    struct MessageToGlobal
    {
        std::string text;
    };

    struct MessageFromGlobal
    {
        std::string user;
        std::string text;
    };

    struct MessageError
    {
        std::string error;
    };

    using Message = std::variant<Ping, Login, Joined, MessageToGlobal, MessageFromGlobal, MessageError>;
}// namespace protocol
#endif
