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

struct Request {

    const std::string m_template{"Author: %s\nTarget: %s\nContent-length: %zu\r\n\r\n%s\r\n\r\n"};
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

/*     template<class T> */
/*     Request(T &&author, T &&target, T &&message): */
/*         m_author(std::forward<T>(author)), */
/*         m_target(std::forward<T>(target)), */
/*         m_message(std::forward<T>(message)) */
/*     { */
/*         m_length = m_message.length(); */
/*     } */

};

struct Session {
    asio::ip::tcp::socket m_sock;
    asio::ip::tcp::endpoint m_ep;
    std::string m_request;

    asio::streambuf m_response_buf;
    std::string m_response;

    system::error_code m_ec;
    bool m_was_cacelled;
    std::mutex m_cancel_gaurd;

    Session(asio::io_service &ios,
            const std::string &raw_ip_address,
            unsigned short port_num,
            const std::string &request):
        m_sock(ios),
        m_ep(asio::ip::address::from_string(raw_ip_address), port_num),
        m_request(request),
        m_was_cacelled(false)
    {}
};

class TCPClient {

    private:

        asio::io_service m_ios;
        std::mutex m_activeSession;
        std::shared_ptr<asio::ip::tcp::socket> m_sock;
        std::unique_ptr<asio::io_service::work> m_work;
        std::vector<std::unique_ptr<std::thread>> m_threads;

        const std::string m_author;
        std::atomic_bool m_isClosed;

        Request request;
        asio::streambuf buf;

        /*
         * Write formatted request to server.
         * precondition:
         *             parameter should be formatted appropriately ending with a newline char.
         * parameters:
         *             {std::string} request: formated message to send to server.
         */
        void writeToServer(std::string request)
        {
            std::cout << request << std::endl;
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

        /*
         * Reads message from server.
         * behavior:
         *          continously calls async functions until request is read.
         */
        void readFromServer()
        {
            std::cout << "Listening for server..." << std::endl;


            if(m_isClosed.load())
            {
                return;
            }

            // read author
            asio::async_read_until(*m_sock.get(), buf, '\n',
                    [this](const system::error_code &ec, std::size_t bytes_transferred)
                    {
                        if(ec.value() != 0)
                        {
                            std::cerr << "Error reading author name: " << ec.message() << std::endl;
                            return;
                        }

                        std::istream is(&buf);
                        std::string temp;

                        std::getline(is, temp);
                        request.m_author = temp.substr(temp.find(' '));

                        std::cout << "Author: " << request.m_author << std::endl;

                        // read target
                        asio::async_read_until(*m_sock.get(), buf, '\n',
                        [this](const system::error_code &ec, std::size_t bytes_transferred)
                        {
                            if(ec.value() != 0)
                            {
                                std::cerr << "Error reading target name: " << ec.message() << std::endl;
                                return;
                            }

                            std::istream is(&buf);
                            std::string temp;

                            std::getline(is, temp);
                            request.m_target = temp.substr(temp.find(' '));

                            std::cout << "Target: " << request.m_target << std::endl;

                            // read content length:
                            asio::async_read_until(*m_sock.get(), buf, '\n',
                            [this](const system::error_code &ec, std::size_t bytes_transferred)
                            {
                                if(ec.value() != 0)
                                {
                                    std::cerr << "Error reading content-length: " << ec.message() << std::endl;
                                    return;
                                }

                                std::istream is(&buf);
                                std::string temp;

                                std::getline(is, temp);
                                sscanf(temp.c_str(), "%*s %zu", &request.m_length);


                                std::cout << "Content-length: " << request.m_length << std::endl;

                                //read contents.
                                asio::async_read(*m_sock.get(), buf, asio::transfer_exactly(request.m_length),
                                [this](const system::error_code &ec, std::size_t bytes_transferred)
                                {
                                    if(ec.value() != 0)
                                    {
                                        std::cerr << "Error reading contents of message: " << ec.message() << std::endl;
                                        return;
                                    }else if(bytes_transferred != request.m_length)
                                    {

                                        std::cerr << "Failed to send all bytes:\nTotal bytes: " << request.m_length
                                                 << "\nBytes sent: " << bytes_transferred << std::endl;
                                        return;
                                    }

                                    std::istream is(&buf);
                                    is >> request.m_message;

                                    std::cout << "Message:\n" << request.m_message << std::endl;
                                    /* readComplete(request); */
                                });
                            });
                        });
                    });
        }

        /*
         * Process request.
         * precondition:
         *              request must have been fully parsed / read.
         */
        void readComplete(std::shared_ptr<Request> request)
        {
            std::cout << request->m_target << ": " << request->m_message << std::endl;

            /* readFromServer(); */
        }

    public:

        TCPClient(std::string author, std::size_t numThreads)
            :m_author(author),
            m_isClosed(false)
        {
            // keep threads running and listening
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

            // send a message to register with server? TODO: need to add logic to server for first time users/connections:
            m_sock->async_connect(ep,
                    [this](const boost::system::error_code &ec)
                    {
                        if(ec.value() != 0)
                        {
                            std::cerr << "Failed to connect to server: " << ec.message() << std::endl;
                        }

                        /* Request request("client", "server", "hello server"); */
                        /* boost::format fmt = boost::format(request.m_template) % request.m_author % request.m_target */
                        /*                                                       % request.m_length % request.m_message; */

                        /* // once connected send message to server. */
                        /* asio::async_write(*m_sock.get(), asio::buffer(fmt.str()), */
                        /*         [this](const system::error_code &ec, std::size_t bytes_transferred) */
                        /*         { */
                        /*             if(ec.value() != 0) */
                        /*             { */
                        /*                std::cerr << "Failed to respond to server-connection: " << ec.message() << std::endl; */
                        /*             } */

                        /*             // continuously listen for server. */
                        /*             readFromServer(); */
                        /*         }); */

                        std::cout << "Succesfully connected to server..." << std::endl;
                    });

        }

        void read()
        {
            readFromServer();
        }

        /*
         * Formats request before writing to server.
         */
        void serveRequest(const std::string &target, const std::string &message)
        {

            std::cout << "Serving request..." << std::endl;
            Request request("client", target, message);
            boost::format fmt = boost::format(request.m_template) % request.m_author % request.m_target
                                                                              % request.m_length % request.m_message;
            writeToServer(std::string(fmt.str()));
        }

        // stop waiting for async operations.
        void close()
        {
            m_work.reset(nullptr);
            m_isClosed.store(true);

            for(auto &thread: m_threads)
            {
                thread->join();
            }
        }
};
#endif // !TCP_CLIENT
