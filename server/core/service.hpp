#ifndef SERVER_SERVICE_H
#define SERVER_SERVICE_H

#include <boost/asio.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/format.hpp>

#include <cstdio>
#include <cstddef>
#include <memory>
#include <queue>

using namespace boost;

namespace server::core
{
struct Request
{
    const std::string messageTemplate{"Author: %s\nTarget: %s\nContent-length: %zu\r\n\r\n%s\r\n\r\n"};

    std::string m_author;
    std::string m_target;
    std::string m_message;

    std::size_t m_length;

    Request() = default;

    Request(std::string author, std::string target, std::string message):
        m_author(author),
        m_target(target),
        m_message(message)
    {
        m_length = m_message.length();
    }
};

// TODO: When client disconnects, remove it from the list of clients.

/*
 * Service is a detached object, that handles reads and writes for a given client. Once
 * done processing client, Service will delete itself.
 */
class Service
{
    private:

        std::string m_clientName;
        Request     request;

        std::shared_ptr<asio::ip::tcp::socket> m_sock;
        std::shared_ptr<std::queue<Request>>   m_writeQueue;

        asio::streambuf m_buffer;

        // Add a return variale <int> or some enum, to dictate whether to continue reading or stop
        // the connections to the client.
        void ErrorCheck(const system::error_code &ec, std::size_t bytes_transferred);

        Request ParseHTTPHeader(asio::streambuf &bufferRequest);

        std::string ReadContents(asio::streambuf &bufferRequest);

        /*
         * Parses client request.
         *
         * behavior:
         *          makes nested asynchronous reads on client message header, then finally reading client message contents.
         */
        void ReadFromClient();

        /*
         * Output data read from client. In the future this will call write, and write to the clients
         * intended target.
         */
        void ReadComplete(Request readRequest);

        /*
         * Writes to client assioated with this service.
         */
        void Write(std::string &&sender, std::string &&message);

        /*
         * When client or server no longer want to talk to each other, delete the current
         * detached object.
         */
        void OnFinish();

    public:

        /* Constructor, defines socket variable. */
        Service(std::string clientName, std::shared_ptr<asio::ip::tcp::socket> sock, std::shared_ptr<std::queue<Request>> writeQueue);

        /* Start handling client, initially reading from client for there information. */
        void StartHandling();

        std::string GetClientName() const;

        /*
         * Invoked by worker thread, when request is being sent to this client.
         */
        void WriteToSelf(std::string sender, std::string message);

        /* Cancels all currently running asynchronous operations. */
        void Stop();
};
}// namespace server::core
#endif
