#include "service.hpp"
#include <iostream>
#include <print>

namespace server::core
{
void Service::ErrorCheck(const system::error_code &ec, std::size_t bytes_transferred)
{
    if(ec == asio::error::operation_aborted)
    {
        std::println(std::cerr, "Server has canceled operation...");
    }
    else if(ec == asio::error::eof && bytes_transferred == 0)
    {
        std::println(std::cerr, "Client has closed connection...");
    }
    else
    {
        std::println(std::cerr, "Error reading author name: {}", ec.message());
    }
}

Request Service::ParseHTTPHeader(asio::streambuf &bufferRequest)
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

std::string Service::ReadContents(asio::streambuf &bufferRequest)
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

void Service::ReadFromClient()
{
    asio::async_read_until(*m_sock.get(), m_buffer, "\r\n\r\n",
            [this](const system::error_code &ec, std::size_t bytes_transferred)
    {
                if(ec.value() != 0)
                {
                    this->ErrorCheck(ec, bytes_transferred);
                    return;
                }

                auto clientRequest = this->ParseHTTPHeader(m_buffer);

                // Read Contents of message.
                asio::async_read_until(*m_sock.get(), m_buffer, "\r\n\r\n",
                        [this,  clientRequest = std::move(clientRequest)](const system::error_code &ec, std::size_t bytes_transferred) mutable
                {
                        if(ec.value() != 0)
                        {
                            this->ErrorCheck(ec, bytes_transferred);
                            return;
                        }
                        clientRequest.m_message = this->ReadContents(m_buffer);

                        this->ReadComplete(clientRequest);
                });
    });
}

void Service::ReadComplete(Request readRequest)
{
    // m_writeQueue->lock();
    m_writeQueue->push(readRequest);
    // m_writeQueue->unlock();

    // continue reading.
    this->ReadFromClient();
}

void Service::Write(std::string &&sender, std::string &&message)
{
    Request l_request(sender, m_clientName, message);
    boost::format fmt = boost::format(l_request.messageTemplate) % l_request.m_author % l_request.m_target
                        % l_request.m_length % l_request.m_message;

    std::println("{}", fmt.str());
    asio::async_write(*m_sock.get(), asio::buffer(fmt.str()),
                     [this, fmt](const system::error_code &ec, std::size_t bytes_transferred)
                     {
                        if(ec.value() != 0)
                        {
                            std::println(std::cerr, "Error writing to client, Error: {}", ec.message());
                        }
                        else if(bytes_transferred != fmt.str().length())
                        {
                            std::println(std::cerr, "Failed to send all bytes:\nTotal bytes: {}\nBytes sent: {}"
                                     ,fmt.str().length()
                                     ,bytes_transferred);
                        }


                        std::println("Successfully sent client message, bytes transferred: ({})", bytes_transferred);
                     });
}

void Service::OnFinish()
{
    std::println("Finished processing request.");
}

Service::Service(std::string clientName, std::shared_ptr<asio::ip::tcp::socket> sock, std::shared_ptr<std::queue<Request>> writeQueue):
    m_clientName(clientName),
    m_sock(sock),
    m_writeQueue(writeQueue)
{}

void Service::StartHandling()
{
    std::println("Handling client...");
    this->ReadFromClient();
}

std::string Service::GetClientName() const
{
    return m_clientName;
}

void Service::WriteToSelf(std::string sender, std::string message)
{
    this->Write(std::move(sender), std::move(message));
}

void Service::Stop()
{
    m_sock->cancel();

    // shutdown connection to socket.
    m_sock->shutdown(asio::socket_base::shutdown_both);
}
}// namespace server::core
