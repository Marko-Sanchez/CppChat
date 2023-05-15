#ifndef CHAT_SERVER
#define CHAT_SERVER

#include <boost/asio.hpp>

#include <boost/asio/io_service.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <list>

#include "service.hpp"
#include "Database.hpp"

using namespace boost;

/*
 * Handles the process of listening for new incoming connections.
 */
class Acceptor
{
    private:
        asio::io_service &m_ios;
        asio::ip::tcp::acceptor m_acceptor;
        std::atomic<bool> m_isStopped;      // TODO: shared_ptr to be given to service to know when to shutdown?
        std::list<std::pair<std::string, std::unique_ptr<Service>>> clients;

        /*
         * When client connects for the first time, parse there first-time request
         * for clients name.
         *
         * parameters:
         *              sock, shared pointer to tcp::socket belonging to newly connected client.
         * behavior:
         *              synchronously waits to read clients message.
         */
        void newClient(std::shared_ptr<asio::ip::tcp::socket> sock)
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

            auto l_service = std::make_unique<Service>(clientName, sock);
            l_service->startHandling();

            clients.emplace_back(std::make_pair(clientName, std::move(l_service)));

            std::cout << "Added user " << clientName << " to list..." << std::endl;
        }

        /*
         * Listens for client connections. Creates a socket and once client connects associates
         * socket with client.
         *
         * behavior:
         *          calls onAccept() to process new client connection.
         */
        void initAccept()
        {
            auto sock = std::make_shared<asio::ip::tcp::socket>(asio::ip::tcp::socket(m_ios));

            m_acceptor.async_accept(*sock.get(),
                    [this, sock](const system::error_code &ec)
                    {
                        if(ec.value() != 0)
                        {
                            std::cerr << "Error connecting to client..." << std::endl;
                        }
                        else
                        {
                            std::cout << "Connection to client successful..." << std::endl;
                            onAccept(ec, sock);
                        }
                    });
        }

        /*
         * when a connection is established check if an error occured, otherwise start handling client.
         *
         * behavior:
         *          handles client, then checks whether server has been stopped otherwise invokes initAccept().
         */
        void onAccept(const system::error_code &ec, std::shared_ptr<asio::ip::tcp::socket> sock)
        {
            if(ec.value() == 0)
            {
                // TODO: unique pointer and add to database
                /* std::unique_ptr<Service> t = std::make_unique<Service>(sock); */
                /* (new Service(sock)) -> startHandling(); */
                newClient(sock);
            }
            else
            {
                std::cout << "Error occured accepting request! Error code =" << ec.value()
                    << ". Message: " << ec.message() << std::endl;
            }

            // server has not been closed continue processing.
            if(!m_isStopped.load())
            {
                initAccept();
            }
            else
            {
                m_acceptor.close();
            }
        }

    public:

        /*
         * Constructor binds asio::acceptor with port to listen on.
         *
         * parameters:
         *              ios, object representing machine network resources.
         *              port_num, port to listen on.
         * behavior:
         *          initializes io_service object and binds asio::acceptor object to port and system resources.
         */
        Acceptor(asio::io_service &ios, unsigned short port_num):
            m_ios(ios),
            m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)),
            m_isStopped(false)
        {}

        /*
         * Starts accepting connections.
         *
         * behavior:
         *          sets acceptor to handle 30 connections at once, ignores rest.
         */
        void start()
        {
            std::cout << "Starting server..." << std::endl;

            m_acceptor.listen(30);
            initAccept();
        }

        /* Stops server, sets atomic variable to false. */
        void stop()
        {
            std::cout << "Stopping server..." << std::endl;
            m_isStopped.store(true);

            // TODO: iterate through list and stop clients:
            for(auto &client: clients)
            {
                // check that pointer exist
                if(client.second)
                    client.second->stop();
            }
        }
};

class AsyncTCPServer
{

    private:

        asio::io_service m_ios;
        std::unique_ptr<asio::io_service::work> m_work;
        std::unique_ptr<Acceptor> acc;
        std::vector<std::unique_ptr<std::thread>> m_threadpool;

    public:

        /* Constructor. */
        AsyncTCPServer()
        {
            m_work.reset(new asio::io_service::work(m_ios));
        }

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
        void start(unsigned short port_num, std::size_t thread_pool_size)
        {
            if(thread_pool_size == 0 || thread_pool_size > 2 * std::thread::hardware_concurrency())
                thread_pool_size = 2;


            acc = std::make_unique<Acceptor>(m_ios, port_num);
            acc->start();

            for(unsigned int i{0}; i < thread_pool_size; ++i)
            {
                auto process = std::make_unique<std::thread>([this](){m_ios.run();});

                m_threadpool.push_back(std::move(process));
            }
        }

        /*
         * Stops server and joins threads.
         *
         * behavior:
         *          calls Acceptor to stop, stops io_service object, and joins threads.
         */
        void stop()
        {

            acc->stop();
            m_ios.stop();

            for(auto& ptrThread: m_threadpool)
            {
                ptrThread->join();
            }
        }
};
#endif // !CHAT_SERVER
