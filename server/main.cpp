#include "server.hpp"

#include <boost/asio.hpp>
#include <string>
#include <thread>
#include <iostream>

int main (int argc, char *argv[])
{
    unsigned short port_num{8080};

    try
    {
        AsyncTCPServer server;
        server.start(8080, 2);

        std::string input;
        while (getline(std::cin, input))
        {
            if(input.length() <= 2)
            {
                server.stop();
                break;
            }
            input.clear();
        }
    }
    catch(boost::system::system_error &ec)
    {
        std::cout << "Error occured accepting request! Error code =" << ec.code()
            << ". Message: " << ec.what() << std::endl;
    }

    return 0;
}
