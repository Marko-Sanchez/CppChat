#ifndef SERVER_SERVICE
#define SERVER_SERVICE

#include <boost/asio.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/format.hpp>

#include <cstdio>
#include <iostream>
#include <cstddef>
#include <memory>

using namespace boost;

struct Request
{
    const std::string messageTemplate{"Author: %s\nTarget: %s\nContent-length: %zu\r\n\r\n%s"};
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

// TODO: when would i call onFinish() ? when it's own socket is shutdown ?
class Service
{
    private:

        std::shared_ptr<asio::ip::tcp::socket> m_sock;
        asio::streambuf m_buffer;
        Request request;

        void readComplete(const system::error_code &ec, std::size_t bytes_transferred)
        {

            writeToTarget();
        }

        /*
         * Parses client request.
         *
         * behavior:
         *          makes nested asynchronous reads on client message header, then finally reading client message contents.
         */
        void readFromClient()
        {
            // read author name
            asio::async_read_until(*m_sock.get(), m_buffer, '\n',
                    [this](const system::error_code &ec, std::size_t bytes_transferred){

                        if(ec.value() != 0)
                        {
                            std::cerr << "Error reading author name: " << ec.message() << std::endl;
                            onFinish();
                        }

                        std::istream is(&m_buffer);
                        std::getline(is, request.m_author);
                        request.m_author = request.m_author.substr(request.m_author.find(' '));

                        std::cout << "Author: " << request.m_author << std::endl;

                        // read target name
                        asio::async_read_until(*m_sock.get(), m_buffer, '\n',
                                [this](const system::error_code &ec, std::size_t bytes_transferred){

                            if(ec.value() != 0)
                            {
                                std::cerr << "Error reading target name: " << ec.message() << std::endl;
                                onFinish();
                            }

                            std::istream is(&m_buffer);
                            std::getline(is, request.m_target);
                            auto size = request.m_target.length();
                            request.m_target = request.m_target.substr(request.m_target.find(' '));

                            std::cout << "Target: " << request.m_target << std::endl;


                            // read content-length
                            asio::async_read_until(*m_sock.get(), m_buffer, "\r\n\r\n",
                                    [this](const system::error_code &ec, std::size_t bytes_transferred)
                                    {
                                        if(ec.value() != 0)
                                        {
                                            std::cerr << "Error reading content-length: " << ec.message() << std::endl;
                                            onFinish();
                                        }

                                        std::istream is(&m_buffer);
                                        std::string temp;

                                        getline(is, temp);
                                        sscanf(temp.c_str(), "%*s %zu", &request.m_length);
                                        getline(is, temp);

                                        std::cout << "Content-length: " << request.m_length << std::endl;

                                        // read contents
                                        asio::async_read_until(*m_sock.get(), m_buffer, "\r\n\r\n",
                                                [this](const system::error_code &ec, std::size_t bytes_transferred)
                                                {
                                                    if(ec.value() != 0)
                                                    {
                                                        std::cerr << "Error reading contents of message: " << ec.message() << std::endl;
                                                        onFinish();
                                                    }

                                                    std::istream is(&m_buffer);

                                                    for(char c; is.get(c); )
                                                    {
                                                        request.m_message.push_back(c);
                                                    }

                                                    std::cout << "Message:\n" << request.m_message << std::endl;

                                                    // TODO: reply to client.
                                                    /* writeToTarget(); */
                                                });
                                    });
                                });
                    });
        }

        /*
         * write to the clients target recipient.
         */
        void writeToTarget()
        {
            std::cout << "Writing to client..." << std::endl;

            asio::streambuf buf;
            std::ostream os(&buf);

            Request l_request("server", "client", "Hello client");
            boost::format fmt = boost::format(l_request.messageTemplate) % l_request.m_author % l_request.m_target
                                % l_request.m_length % l_request.m_message;
            std::cout << "Printing out message:\n" << fmt.str() << std::endl;

             asio::async_write(*m_sock.get()/*this would be the other clients socket, placeholder for now*/,
                             asio::buffer(fmt.str()),
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

                                std::cout << "Successfully sent client message..." << std::endl;
                             });

            onFinish();
        }

        void onFinish()
        {
            std::cout << "Finished processing request" << std::endl;
            /* delete this; */
        }

    public:

        Service(std::shared_ptr<asio::ip::tcp::socket> sock):
            m_sock(sock)
        {}

        void startHandling()
        {
            // TODO: function for first time client connection to store there name and also adds them to database
            std::cout << "Handling client..." << std::endl;
            readFromClient();
        }
};

#endif // !SERVER_SERVICE
