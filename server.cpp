#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <algorithm>

#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

constexpr uint16_t PORT{8080};
constexpr ssize_t BUFFER_SIZE{1024};

/* Contianer for colored text values to output onto terminal */
const struct Colors{

    const std::string pass{"\033[0;32m"};
    const std::string sMsg{"\033[0;36m"};
    const std::string warning{"\033[0;33m"};
    const std::string error{"\033[0;31m"};

    const std::string end{"\033[0m"};

}color;

int main(int argc, char* argv[])
{
    /* setting up socket information */

    struct sockaddr_in address;

    // Connect using: domain: IPV4, comunication tye: TCP, and using internet protocol (0)
    int server_fd{socket(AF_INET, SOCK_STREAM, 0)};

    if(server_fd == 0)
    {
        std::cerr << color.error << "socket connection failed" << color.end << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << color.pass << "socket connected" << color.end << std::endl;

    // Attach socket to port:
    int option{1};
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
    {
        std::cerr << color.error << "error in setting socket options" << color.end << std::endl;
        return EXIT_FAILURE;
    }

    // INADDR_ANY: bind socket to all local interfaces, using IPV4 family:
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)))
    {
        std::cerr << color.error << "binding socket failed" << color.end << std::endl;
        return EXIT_FAILURE;
    }

    if(listen(server_fd, 5) < 0)
    {
        std::cerr << color.error << "unable to start listening" << color.end << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << color.pass << "listening..." << color.end << std::endl;

    /* server starts listening and processing client connections */

    // client address container:
    struct sockaddr_in client;
    socklen_t len{sizeof(client)};

    // monitor multiple file descriptors:
    fd_set socketset;
    int max_connections{6};

    // client file descriptor container:
    std::vector<int> clients(max_connections, 0);
    int currentConnections{0};

    char welcomeMsg[] = "Welcome to the Server!";
    char response[] = "Message received\n";
    char buffer[BUFFER_SIZE] = {0};

    int testCounter{1};
    while(true)
    {

        FD_ZERO(&socketset);
        FD_SET(server_fd, &socketset);

        // max file descriptor for select listen range:
        int maxFD{server_fd};

        // add all client sockets back to set:
        for(const auto& cli: clients)
        {
            if(cli != 0)
            {
                FD_SET(cli, &socketset);
                maxFD = std::max(maxFD, cli);
            }

        }

        // listen for activity on all sockets in set range:
        int ready{select(maxFD + 1, &socketset, nullptr, nullptr, nullptr)};

        // check if activity was caused by server or client:
        if(FD_ISSET(server_fd, &socketset))
        {

            // if we have reached maxed connections do not accept:
            if(currentConnections == max_connections)
            {
                std::cout << color.warning << "max connections reached" << color.end << std::endl;
                continue;
            }

            int incomingClient{accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&len)};
            if(incomingClient < 0)
            {
                std::cerr << color.error << "unable to accept to client" << color.end << std::endl;
                continue;
            }

            std::cout << color.warning << "new connection, socket: " << incomingClient
                      << " ,there IP is: " << inet_ntoa(client.sin_addr) << color.end
                      << std::endl;

            ssize_t bytes = send(incomingClient, welcomeMsg, sizeof(welcomeMsg), 0);

            // add new client to set and container:
            FD_SET(incomingClient, &socketset);

            auto iter = find(begin(clients), end(clients), 0);
            *iter = incomingClient;

            currentConnections++;

        }else
        {

            for(auto & client: clients)
            {
                if(FD_ISSET( client, &socketset))
                {
                    ssize_t bytes = recv(client,  buffer, sizeof(buffer), 0);
                    std::cout << "number of bytes read: " << bytes << std::endl;

                    // dicsonect if no bytes were sent, else output message to terminal:
                    if(bytes <= 0)
                    {
                        std::cout << color.warning << "client no longer wants to talk" << color.end << std::endl;

                        // close client connection and remove from container:
                        close(client);
                        client = 0;
                        currentConnections--;

                    }else
                    {
                        // output client response:
                        std::cout << "client: " << buffer << std::endl;
                        testCounter++;

                        // welcome new client to the server: pusedo reply testing
                        if(testCounter == 3)
                        {
                            bytes = send(client, welcomeMsg, sizeof(welcomeMsg), 0);
                            std::cout << "Msg sent: " << bytes << std::endl;
                        }
                        testCounter = testCounter % 3;

                    }

                    // clear buffer:
                    memset(buffer, 0, BUFFER_SIZE);

                }
            }
        }

        // no clients connected, stop listening:
        if(currentConnections == 0)break;
    }

    close(server_fd);

    return EXIT_SUCCESS;
}
