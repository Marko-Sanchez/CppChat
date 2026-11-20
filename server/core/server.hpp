#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>

#include <boost/asio/io_service.hpp>
#include <memory>
#include <utility>
#include <list>
#include <atomic>
#include <queue>

#include "service.hpp"
#include "acceptor.h"

namespace server::core
{
class AsyncTCPServer
{

    private:

        using clientInterface = std::pair<std::string, std::unique_ptr<Service>>;

        asio::io_service                          m_ios;
        std::unique_ptr<Acceptor>                 m_acc;
        std::unique_ptr<asio::io_service::work>   m_work;
        std::vector<std::unique_ptr<std::thread>> m_threadpool;

        // variables shared between threads.
        std::shared_ptr<std::atomic<bool>>          m_isServerRunning;
        std::shared_ptr<std::queue<Request>>        m_writeQueue;
        std::shared_ptr<std::list<clientInterface>> m_clients;

    public:

        /* Constructor. */
        AsyncTCPServer();

        /*
         * Starts accepting connections and creates worker threads to handle request.
         *
         * param:
         *      port_num port on which server will listen on.
         *      thread_pool_size how many threads to create.
         *
         *  behavior:
         *      calls Acceptor.start() which will invoke a asynchronous function that will listen
         *      for connections.
         */
        void Run(unsigned short port_num, std::size_t thread_pool_size);

        /*
         * Stops server and joins threads.
         *
         * behavior:
         *          calls Acceptor to stop, stops io_service object, and joins threads.
         */
        void Stop();
};
}// namespace server::core
#endif
