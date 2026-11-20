#ifndef SERVER_ACCEPTOR_H
#define SERVER_ACCEPTOR_H

#include <memory>
#include <list>

#include "service.hpp"

namespace server::core
{
/*
 * Handles the process of listening for new incoming connections.
 */
class Acceptor
{
    private:
        using clientInterface = std::pair<std::string, std::unique_ptr<Service>>;

        asio::io_service       &m_ios;
        asio::ip::tcp::acceptor m_acceptor;

        std::shared_ptr<std::atomic<bool>>          m_isServerRunning;
        std::shared_ptr<std::queue<Request>>        m_writeQueue;
        std::shared_ptr<std::list<clientInterface>> m_clients;

        /*
         * When client connects for the first time, parse there first-time request
         * for clients name.
         *
         * parameters:
         *              sock, shared pointer to tcp::socket belonging to newly connected client.
         * behavior:
         *              synchronously waits to read clients message.
         */
        void NewClient(std::shared_ptr<asio::ip::tcp::socket> sock);

        /*
         * Listens for client connections. Creates a socket and once client connects associates
         * socket with client.
         *
         * behavior:
         *          calls onAccept() to process new client connection.
         */
        void InitAccept();

        /*
         * when a connection is established check if an error occured, otherwise start handling client.
         *
         * behavior:
         *          handles client, then checks whether server has been stopped otherwise invokes initAccept().
         */
        void OnAccept(const system::error_code &ec, std::shared_ptr<asio::ip::tcp::socket> sock);

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
        Acceptor(asio::io_service &ios, unsigned short port_num, std::shared_ptr<std::list<clientInterface>> &clients, std::shared_ptr<std::queue<Request>> writeQueue, std::shared_ptr<std::atomic<bool>> isServerRunning);

        /*
         * Starts accepting connections.
         *
         * behavior:
         *          sets acceptor to handle 30 connections at once, ignores rest.
         */
        void Start();

        /* Stops server, sets atomic variable to false. */
        void Stop();

};
}// namespace server::core
#endif
