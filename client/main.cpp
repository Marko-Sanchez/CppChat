#include "asynctcpclient.hpp"
#include <iostream>
#include <thread>

int main (int argc, char *argv[])
{
    try
    {
        TCPClient client("marko", 2);

        client.connect("127.0.0.1", 8080);

        std::this_thread::sleep_for(std::chrono::seconds(5));

        client.serveRequest("server", "Hello Server");

        std::this_thread::sleep_for(std::chrono::seconds(5));

        client.close();
    }
    catch (boost::system::system_error &ec)
    {

        std::cout << "Error occured accepting request! Error code =" << ec.code()
            << ". Message: " << ec.what() << std::endl;
    }
    return 0;
}
