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
    const std::string messageTemplate{"Author: %s\nTarget: %s\nContent-length: %d\n%s"};
    std::string m_author;
    std::string m_target;
    std::string m_message;

    std::size_t m_length;
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

                        }

                        std::istream is(&m_buffer);
                        std::getline(is, request.m_author);
                        request.m_author = request.m_author.substr(request.m_author.find(' '));

                        // read target name
                        asio::async_read_until(*m_sock.get(), m_buffer, '\n',
                                [this](const system::error_code &ec, std::size_t bytes_transferred){

                            if(ec.value() != 0)
                            {

                            }

                            std::istream is(&m_buffer);
                            std::getline(is, request.m_author);
                            request.m_target = request.m_target.substr(request.m_target.find(' '));

                            // read content-length
                            asio::async_read_until(*m_sock.get(), m_buffer, '\n',
                                    [this](const system::error_code &ec, std::size_t bytes_transferred)
                                    {
                                        if(ec.value() != 0)
                                        {

                                        }

                                        std::istream is(&m_buffer);
                                        std::string temp;

                                        getline(is, temp);
                                        sscanf(temp.c_str(), "%*s %ld", &request.m_length);


                                        // read contents
                                        asio::async_read(*m_sock.get(), m_buffer, asio::transfer_exactly(request.m_length),
                                                [this](const system::error_code &ec, std::size_t bytes_transferred)
                                                {
                                                    if(ec.value() != 0)
                                                    {

                                                    }

                                                    // TODO: check if bytes bytes_transferred equals content length ?
                                                    std::istream is(&m_buffer);
                                                    is >> request.m_message;

                                                    readComplete(ec, bytes_transferred);
                                                });
                                    });
                                });
                    });
        }

        /*
         * write to the clients target recipient.
         *
         * TODO: implement client to test
         */
        void writeToTarget()
        {
            std::cout << request.m_message << std::endl;

            asio::streambuf buf;
            std::ostream os(&buf);
            os << "Author: " << request.m_author << '\n'
               << "Target: " << request.m_target << '\n'
               << "Content-length: " << request.m_length << '\n'
               << request.m_message << '\n';

            boost::format fmt = boost::format(request.messageTemplate) % request.m_author % request.m_target
                                % request.m_length % request.m_message;
            std::cout << fmt.str() << std::endl;

            // asio::async_write(*m_sock.get()/*this would be the other clients socket, placeholder for now*/,
                             /* asio::buffer(fmt.str()), */
                             /* [this](const system::error_code &ec, std::size_t bytes_transferred) */
                             /* { */

/*                              }); */

            onFinish();
        }

        void onFinish()
        {
            delete this;
        }

    public:

        Service(std::shared_ptr<asio::ip::tcp::socket> sock):
            m_sock(sock)
        {}

        void startHandling()
        {
            // TODO: function for first time client connection to store there name and also adds them to database
            readFromClient();
        }
};

#endif // !SERVER_SERVICE
