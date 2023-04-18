#include "server.hpp"

#include <boost/asio.hpp>
#include <thread>
#include <iostream>

int main (int argc, char *argv[])
{
    const std::string ip_address{"127.0.0.1"};
    unsigned short port_num{8080};

    try
    {
        AsyncTCPServer server;
        server.start(8080, 2);

        std::this_thread::sleep_for(std::chrono::seconds(10));

        server.stop();
    }catch(boost::system::system_error &ec)
    {
        std::cout << "Error occured accepting request! Error code =" << ec.code()
            << ". Message: " << ec.what() << std::endl;
    }

    return 0;
}
