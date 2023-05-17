#ifndef TCP_CLIENT
#define TCP_CLIENT

#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/format.hpp>

#include <cstddef>
#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <utility>

using namespace boost;

/* Container for colored text values to output onto terminal */
const struct Colors{

    const std::string pass{"\033[0;32m"};
    const std::string sMsg{"\033[0;36m"};
    const std::string warning{"\033[0;33m"};
    const std::string error{"\033[0;31m"};

    const std::string end{"\033[0m"};

}color;

struct Request {

    /* asio::streambuf m_buffer; */

    const std::string cm_template{"Author: %s\nTarget: %s\nContent-length: %zu\r\n\r\n%s\r\n\r\n"};
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

class TCPClient {

    private:

        const std::string cm_username;
        std::shared_ptr<Request> m_readRequest;
        Request m_request;
        asio::streambuf m_buffer;

        asio::io_service m_ios;
        std::shared_ptr<asio::ip::tcp::socket> m_sock;
        std::unique_ptr<asio::io_service::work> m_work;
        std::vector<std::unique_ptr<std::thread>> m_threads;

        // Add a return variale <int> or some enum, to dictate whether to continue reading or stop
        // the connections to the client.
        void errorCheck(const system::error_code &ec, std::size_t bytes_transferred)
        {
            if(ec == asio::error::operation_aborted)
            {
                std::cerr << "Client has canceled operation..." << std::endl;
            }
            else if(ec == asio::error::eof && bytes_transferred == 0)
            {
                std::cerr << "Server has closed connection..." << std::endl;
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
         * Process request, then continue reading.
         *
         * precondition:
         *              request must have been fully parsed / read.
         */
        void readComplete(Request request)
        {
            std::cout << color.sMsg << request.m_author << color.end << ": " << request.m_message << std::endl;

            read();
        }

        /*
         * Reads message from server.
         * behavior:
         *          continously calls async functions until request is read.
         */
        void read()
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
         * Write formatted request to server.
         * precondition:
         *             parameter should be formatted appropriately ending with a newline char.
         * parameters:
         *             {std::string} request: formated message to send to server.
         */
        void writeToServer(std::string request)
        {
            asio::async_write(*m_sock.get(), asio::buffer(request),
                    [this, request](const system::error_code &ec, std::size_t bytes_transferred){
                        if(ec.value() != 0)
                        {
                            // Pop up a GUI to let client to know we failed to send message.
                            std::string err{"Failed to write message to server: " + ec.message()};
                        }

                        if(bytes_transferred != request.length())
                        {
                            std::cerr << "Did not send all bytes:\n total bytes: " << request.length()
                                      << "bytes sent: " << bytes_transferred << std::endl;
                        }
                    });
        }


    public:

        TCPClient(std::string user, std::size_t numThreads)
            :cm_username(user)
        {
            // keep threads running and listening, when there are no async operations ongoing.
            m_work = std::make_unique<asio::io_service::work>(asio::io_service::work(m_ios));

            for(std::size_t i{0}; i < numThreads; ++i)
            {
                auto thread = std::make_unique<std::thread>([this] () { m_ios.run(); });

                m_threads.push_back(std::move(thread));
            }
        }

        void connect(std::string_view rawIPAddress, unsigned short portNum)
        {
            m_sock = std::make_shared<asio::ip::tcp::socket>(asio::ip::tcp::socket(m_ios));
            asio::ip::tcp::endpoint ep(asio::ip::address::from_string(rawIPAddress.data()), portNum);

            m_sock->open(ep.protocol());

            m_sock->async_connect(ep,
                    [this](const boost::system::error_code &ec)
                    {
                        if(ec.value() != 0)
                        {
                            std::cerr << "Failed to connect to server: " << ec.message() << std::endl;
                        }

                        std::cout << color.warning << "Succesfully connected to server..." << color.end << std::endl;

                        std::string firstContact{"Author: "};
                        firstContact.append(cm_username);
                        firstContact.append("\r\n\r\n");

                        std::cout << color.pass << "Currently Connected as " << cm_username << color.end << std::endl;

                        // once connected send message to server.
                        asio::async_write(*m_sock.get(), asio::buffer(firstContact),
                                [this](const system::error_code &ec, std::size_t bytes_transferred)
                                {
                                    if(ec.value() != 0)
                                    {
                                       std::cerr << "Failed to respond to server-connection: " << ec.message() << std::endl;
                                    }

                                    // continuously listen for server.
                                    read();
                                });
                    });

        }

        /*
         * Formats request before writing to server.
         */
        void serveRequest(const std::string &target, const std::string &message)
        {

            std::cout << "Serving request..." << std::endl;
            Request request(cm_username, target, message);
            boost::format fmt = boost::format(request.cm_template) % request.m_author % request.m_target
                                                                              % request.m_length % request.m_message;
            writeToServer(std::string(fmt.str()));
        }

        /* stop waiting for async operations. */
        void close()
        {
            m_sock->cancel();
            m_sock->shutdown(asio::socket_base::shutdown_both);

            m_work.reset(nullptr);

            for(auto &thread: m_threads)
            {
                thread->join();
            }
        }
};
#endif // !TCP_CLIENT
