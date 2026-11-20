#include "acceptor.h"

#include <iostream>
#include <print>

namespace server::core
{
void Acceptor::NewClient(std::shared_ptr<asio::ip::tcp::socket> sock)
{
    asio::streambuf l_buffer;
    auto bytes_read = asio::read_until(*sock.get(), l_buffer, "\r\n\r\n");
    l_buffer.commit(bytes_read);

    std::istream is(&l_buffer);
    std::string clientName;
    std::getline(is,clientName);

    // parse "Author: <name>\r"
    clientName = clientName.substr(clientName.find(' ') + 1);
    clientName.pop_back();

    auto l_service = std::make_unique<Service>(clientName, sock, m_writeQueue);
    l_service->StartHandling();

    m_clients->emplace_back(std::make_pair(clientName, std::move(l_service)));

    std::println("Added User: {} to list...", clientName);
}

void Acceptor::InitAccept()
{
    auto sock = std::make_shared<asio::ip::tcp::socket>(asio::ip::tcp::socket(m_ios));

    m_acceptor.async_accept(*sock.get(),
            [this, sock](const system::error_code &ec)
            {
                if(ec.value() != 0)
                {
                    std::println(std::cerr, "Error connecting to client...");
                }
                else
                {
                    std::println("Connection to client successful...");
                    this->OnAccept(ec, sock);
                }
            });
}

void Acceptor::OnAccept(const system::error_code &ec, std::shared_ptr<asio::ip::tcp::socket> sock)
{
    if(ec.value() == 0)
    {
        // TODO: unique pointer and add to database
        this->NewClient(sock);
    }
    else
    {
        std::println("Error occured accepting request! Message: {}\nError code = {}", ec.message(), ec.value());
    }

    // server has not been closed continue processing.
    if(m_isServerRunning->load())
    {
        this->InitAccept();
    }
    else
    {
        m_acceptor.close();
    }
}

Acceptor::Acceptor(asio::io_service &ios, unsigned short port_num, std::shared_ptr<std::list<clientInterface>> &clients, std::shared_ptr<std::queue<Request>> writeQueue, std::shared_ptr<std::atomic<bool>> isServerRunning):
    m_ios(ios),
    m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)),
    m_clients(clients),
    m_writeQueue(writeQueue),
    m_isServerRunning(isServerRunning)
{}

void Acceptor::Start()
{
    std::println("Starting server...");

    m_acceptor.listen(30);
    this->InitAccept();
}

void Acceptor::Stop()
{
    std::println("Stopping server...");
    m_isServerRunning->store(false);

    // TODO: m_client, might need a mutex lock since threads are accessing it.
    for(auto &client: *m_clients.get())
    {
        // check that pointer exist
        if(client.second)
        {
            client.second->Stop();
        }
    }
}
}// namespace server::core
