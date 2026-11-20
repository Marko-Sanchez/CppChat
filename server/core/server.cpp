#include "server.hpp"

#include <iostream>

#include "worker.hpp"

namespace server::core
{
AsyncTCPServer::AsyncTCPServer():
    m_work(std::make_unique<asio::io_service::work>(m_ios))
{}

void AsyncTCPServer::Run(unsigned short port_num, std::size_t thread_pool_size)
{
    if(thread_pool_size == 0 || thread_pool_size > 2 * std::thread::hardware_concurrency())
    {
        thread_pool_size = 2;
    }

    m_isServerRunning = std::make_shared<std::atomic<bool>>(true);
    m_clients         = std::make_shared<std::list<clientInterface>>();
    m_writeQueue      = std::make_shared<std::queue<Request>>();

    m_acc = std::make_unique<Acceptor>(m_ios, port_num, m_clients, m_writeQueue, m_isServerRunning);
    m_acc->Start();

    for(std::size_t i{0}; i < thread_pool_size; ++i)
    {
        auto process = std::make_unique<std::thread>([this](){m_ios.run();});

        m_threadpool.push_back(std::move(process));
    }

    // Thread in charge of writing to clients.
    auto writethread = std::make_unique<std::thread>( [this](){Worker worker(m_writeQueue, m_clients, m_isServerRunning); });
    m_threadpool.push_back(std::move(writethread));


    // wait for user intput to stop server.
    std::string input;
    while (getline(std::cin, input))
    {
        if(input.length() <= 2)
        {
            this->Stop();
            break;
        }
        input.clear();
    }
}

void AsyncTCPServer::Stop()
{
    m_acc->Stop();
    m_ios.stop();

    for(auto& ptrThread: m_threadpool)
    {
        ptrThread->join();
    }
}
}// namespace server::core
