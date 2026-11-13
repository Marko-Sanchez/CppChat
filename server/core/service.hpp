#ifndef SERVER_SERVICE
#define SERVER_SERVICE

#include <boost/asio.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/format.hpp>

#include <cstdio>
#include <exception>
#include <iostream>
#include <cstddef>
#include <memory>
#include <queue>

using namespace boost;

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
        Request request;

        std::shared_ptr<asio::ip::tcp::socket> m_sock;
        std::shared_ptr<std::queue<Request>> m_writeQueue;
        asio::streambuf m_buffer;

        // Add a return variale <int> or some enum, to dictate whether to continue reading or stop
        // the connections to the client.
        void errorCheck(const system::error_code &ec, std::size_t bytes_transferred)
        {
            if(ec == asio::error::operation_aborted)
            {
                std::cerr << "Server has canceled operation..." << std::endl;
            }
            else if(ec == asio::error::eof && bytes_transferred == 0)
            {
                std::cerr << "Client has closed connection..." << std::endl;
            }
            else
            {
                std::cerr << "Error reading author name: " << ec.message() << std::endl;
            }
        }

        Request parseHTTPHeader(asio::streambuf &bufferRequest)
        {
            Request l_request;
            std::istream is(&bufferRequest);

            std::size_t commited{0};

            try
            {
                getline(is, l_request.m_author);
                l_request.m_author = l_request.m_author.substr(l_request.m_author.find(' ') + 1);

                getline(is, l_request.m_target);
                l_request.m_target = l_request.m_target.substr(l_request.m_target.find(' ') + 1);

                for(char c; is.get(c) && c != '\r';)
                {
                    if(std::isdigit(c))
                    {
                        l_request.m_length = l_request.m_length * 10 + static_cast<std::size_t>(c % 'a');
                    }
                }

                // those last three '\n\r\n' characters will remain in the buffer.
                m_buffer.consume(3);
            }catch(std::exception &ec)
            {
                std::cerr << ec.what() << std::endl;
            }

            return l_request;
        }

        std::string readContents(asio::streambuf &bufferRequest)
        {
            std::istream is(&bufferRequest);

            std::string l_message;
            for(char c; is.get(c);)
            {
                l_message.push_back(c);
            }

            // remove '\r\n\r\n'
            l_message.erase(l_message.length() - 4);

            return l_message;
        }

        /*
         * Parses client request.
         *
         * behavior:
         *          makes nested asynchronous reads on client message header, then finally reading client message contents.
         */
        void readFromClient()
        {
            asio::async_read_until(*m_sock.get(), m_buffer, "\r\n\r\n",
                    [this](const system::error_code &ec, std::size_t bytes_transferred)
            {
                        if(ec.value() != 0)
                        {
                            errorCheck(ec, bytes_transferred);
                            return;
                        }

                        auto clientRequest = parseHTTPHeader(m_buffer);

                        // Read Contents of message.
                        asio::async_read_until(*m_sock.get(), m_buffer, "\r\n\r\n",
                                [this,  clientRequest = std::move(clientRequest)](const system::error_code &ec, std::size_t bytes_transferred) mutable
                        {
                                if(ec.value() != 0)
                                {
                                    errorCheck(ec, bytes_transferred);
                                    return;
                                }
                                clientRequest.m_message = readContents(m_buffer);

                                readComplete(clientRequest);
                        });
            });
        }

        /*
         * Output data read from client. In the future this will call write, and write to the clients
         * intended target.
         */
        void readComplete(Request readRequest)
        {
            // m_writeQueue->lock();
            m_writeQueue->push(readRequest);
            // m_writeQueue->unlock();

            // continue reading.
            readFromClient();
        }


        /*
         * Writes to client assioated with this service.
         */
        void write(std::string &&sender, std::string &&message)
        {
            Request l_request(sender, m_clientName, message);
            boost::format fmt = boost::format(l_request.messageTemplate) % l_request.m_author % l_request.m_target
                                % l_request.m_length % l_request.m_message;

            std::cout << fmt.str() << std::endl;
            asio::async_write(*m_sock.get(), asio::buffer(fmt.str()),
                             [this, fmt](const system::error_code &ec, std::size_t bytes_transferred)
                             {
                                if(ec.value() != 0)
                                {
                                    std::cerr << "Error writing to client: " << ec.message() << std::endl;
                                }else if(bytes_transferred != fmt.str().length())
                                {
                                    std::cerr << "Failed to send all bytes:\nTotal bytes: " << fmt.str().length()
                                             << "\nBytes sent: " << bytes_transferred << std::endl;
                                }

                                std::cout << "Successfully sent client message..." << bytes_transferred << std::endl;
                             });
        }

        /*
         * When client or server no longer want to talk to each other, delete the current
         * detached object.
         */
        void onFinish()
        {
            std::cout << "Finished processing request" << std::endl;
        }

    public:

        /* Constructor, defines socket variable. */
        Service(std::string clientName, std::shared_ptr<asio::ip::tcp::socket> sock, std::shared_ptr<std::queue<Request>> writeQueue):
            m_clientName(clientName),
            m_sock(sock),
            m_writeQueue(writeQueue)
        {}

        /* Start handling client, initially reading from client for there information. */
        void startHandling()
        {
            std::cout << "Handling client..." << std::endl;
            readFromClient();
        }

        std::string getClientName() const
        {
            return m_clientName;
        }

        /*
         * Invoked by worker thread, when request is being sent to this client.
         */
        void writeToSelf(std::string sender, std::string message)
        {
            write(std::move(sender), std::move(message));
        }

        /* Cancels all currently running asynchronous operations. */
        void stop()
        {
            m_sock->cancel();

            // shutdown connection to socket.
            m_sock->shutdown(asio::socket_base::shutdown_both);
        }
};

#endif // !SERVER_SERVICE
