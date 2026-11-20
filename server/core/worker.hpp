#ifndef WORKER_H
#define WORKER_H

#include <iostream>
#include <print>
#include <thread>
#include <memory>
#include <queue>
#include <list>

#include "service.hpp"

namespace server::core
{
class Worker
{
    private:
        using clientInterface = std::pair<std::string, std::unique_ptr<Service>>;

        std::shared_ptr<std::queue<Request>> m_writeQueue;
        std::shared_ptr<std::list<clientInterface>> m_clientList;
        std::shared_ptr<std::atomic<bool>> m_serverRunning;

        /*
         * Waits for request to come in via the queue.
         */
        void work()
        {
            while(m_serverRunning->load())
            {
                // If queue is empty wait, sleep for a bit.
                if(m_writeQueue->empty())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                else
                {
                    std::println("Processing request...");
                    // pop client request from queue
                    // m_writeQueue->getLock();
                    if(!m_writeQueue->empty())
                    {
                        Request l_request = m_writeQueue->front();
                        m_writeQueue->pop();
                        // m_writeQueue->releaseLock();

                        // find client in client list
                        auto l_client = std::find_if(m_clientList->begin(), m_clientList->end(), [&l_request](const clientInterface &client)
                                {
                                    return client.first == l_request.m_target;
                                });

                        // if client is found, send message to it's socket.
                        if(l_client != m_clientList->end())
                        {
                            l_client->second->WriteToSelf(l_request.m_author, l_request.m_message);
                        }
                        else
                        {
                            std::println(std::cerr, "Client ({}) not found.", l_request.m_target);
                        }
                    }
                }// end else
            }
        }

    public:

        Worker(std::shared_ptr<std::queue<Request>> writeQueue, std::shared_ptr<std::list<clientInterface>> clientList, std::shared_ptr<std::atomic<bool>> serverRunning):
            m_writeQueue(writeQueue),
            m_clientList(clientList),
            m_serverRunning(serverRunning)
        {
            work();
        }
};
}// namespace core
#endif
