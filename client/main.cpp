#include "asynctcpclient.hpp"
#include <iostream>
#include <thread>

int main (int argc, char *argv[])
{
    try
    {
        std::cout << "Enter client name: ";

        std::string clientName {"marko"};
        getline(std::cin, clientName);

        TCPClient client(clientName, 2);

        client.connect("127.0.0.1", 8080);

        // Loop waiting for user input:
        std::string input;
        while(getline(std::cin, input))
        {
            if(input.length() <= 2)
            {
                client.close();
                break;
            }

            std::cout << "Who do you want to send message to: ";
            std::string target;
            getline(std::cin, target);

            client.serveRequest(target, input);
            input.clear();
        }
    }
    catch (boost::system::system_error &ec)
    {

        std::cout << "Error occured accepting request! Error code =" << ec.code()
            << ". Message: " << ec.what() << std::endl;
    }
    return 0;
}
