#ifndef CHAT_SERVER
#define CHAT_SERVER

#include <boost/asio.hpp>

#include <boost/asio/io_service.hpp>
#include <iostream>
#include <memory>
#include <utility>

#include "service.hpp"
#include "Database.hpp"

using namespace boost;

class Acceptor
{
    private:
        asio::io_service &m_ios;
        asio::ip::tcp::acceptor m_acceptor;
        std::atomic<bool> m_isStopped;      // TODO: shared_ptr to be given to service to know when to shutdown?

        std::shared_ptr<Database> m_database;

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
                            std::cout << "Connection to client succesfull..." << std::endl;
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
                (new Service(sock)) -> startHandling();
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

        Acceptor(asio::io_service &ios, unsigned short port_num):
            m_ios(ios),
            m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)),
            m_isStopped(false)
        {}

        void start()
        {
            std::cout << "Starting server..." << std::endl;

            m_database = std::make_shared<Database>(Database());

            m_acceptor.listen(30);
            initAccept();
        }

        void stop()
        {
            std::cout << "Stopping server..." << std::endl;
            m_isStopped.store(true);
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

        AsyncTCPServer()
        {
            m_work.reset(new asio::io_service::work(m_ios));
        }

        /*
         * param:
         *      port_num port on which server will listen on.
         *      thread_pool_size how many threads to create.
         *
         * behavior:
         *      starts accepting connections and creates worker threads to handle request.
         */
        void start(unsigned short port_num, std::size_t thread_pool_size)
        {
            if(thread_pool_size == 0 || thread_pool_size > 2 * std::thread::hardware_concurrency())
                thread_pool_size = 2;


            acc = std::make_unique<Acceptor>(m_ios, port_num);
            acc->start();

            for(unsigned int i{0}; i < thread_pool_size; ++i)
            {
                /* auto process = std::make_unique<std::thread>(std::thread([this](){m_ios.run();})); */
                auto process = std::make_unique<std::thread>([this](){m_ios.run();});

                m_threadpool.push_back(std::move(process));
            }
        }

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
