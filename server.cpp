#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <utility>

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
    socklen_t cli_len{sizeof(client)};

    fd_set socketset;
    const size_t max_connections{6};

    std::list< std::pair<int,std::string> > clients;      // client file descriptors:
    std::map<std::string, int> userTable;                 // client names, client fd:


    std::vector<uint8_t> buffer(BUFFER_SIZE, 0);
    const std::string welcomeMsg{"Welcome to the Server, "};
    int flags{0};

    while(true)
    {

        FD_ZERO(&socketset);
        FD_SET(server_fd, &socketset);

        // max file descriptor for select listen range:
        int maxFD{server_fd};

        // add all client sockets back to set:
        for(const auto& client: clients)
        {
            FD_SET(client.first, &socketset);
            maxFD = std::max(maxFD, client.first);
        }

        // listen for activity on all sockets in set range:
        int ready{select(maxFD + 1, &socketset, nullptr, nullptr, nullptr)};

        if(ready < 0)
        {
            std::cerr << color.error << "server error" << color.end << std::endl;
            break;
        }
        // check if activity was caused by server or client:
        else if(FD_ISSET(server_fd, &socketset))
        {

            // if we have reached maxed connections do not accept:
            if(clients.size() == max_connections)
            {
                std::cout << color.warning << "max connections reached" << color.end << std::endl;
                continue;
            }

            int incomingClient{accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&cli_len)};
            if(incomingClient < 0)
            {
                std::cerr << color.error << "unable to accept to client" << color.end << std::endl;
                continue;
            }

            std::cout << color.warning << "new connection, socket: " << incomingClient
                      << " ,there IP is: " << inet_ntoa(client.sin_addr) << color.end
                      << std::endl;

            // welcome user to server:

            auto bytes = recv(incomingClient, &buffer[0], BUFFER_SIZE, flags);

            std::string name{begin(buffer), begin(buffer) + bytes};
            std::string stemp{welcomeMsg + name};

            userTable[name] = incomingClient;

            std::vector<uint8_t> temp(cbegin(stemp), cend(stemp));
            auto len = static_cast<size_t>(temp.size());

            send(incomingClient, &temp[0], len, flags);

            // add client to container:
            clients.emplace_back(std::make_pair(incomingClient, name));

        }else
        {

            for(auto iter = begin(clients); iter != end(clients); iter++)
            {

                auto [client, name] = *iter;

                if(FD_ISSET( client, &socketset))
                {

                    ssize_t bytes = recv(client,  &buffer[0], BUFFER_SIZE, flags);
                    std::cout << "number of bytes read: " << bytes << std::endl;

                    std::string msg(cbegin(buffer), cbegin(buffer) + bytes);

                    // disconnect if no bytes were sent, else output message to terminal:
                    if(bytes <= 0)
                    {
                        std::cout << color.warning << "client has disconnected..." << color.end << std::endl;

                        close(client);
                        userTable.erase(name);
                        iter = clients.erase(iter--);

                    }else
                    {

                        std::cout << "client->" << msg << std::endl;

                        auto found = msg.find(':');

                        // send msg to other client, else echo back:
                        if(found != std::string::npos && userTable.find(msg.substr(0, found)) != end(userTable))
                        {
                            std::string reciever{msg.substr(0, found)};
                            auto clientFD = userTable[reciever];

                            msg = name + msg.substr(found);

                            std::vector<uint8_t> temp(cbegin(msg), cend(msg));
                            auto len = static_cast<size_t>(temp.size());

                            bytes = send(clientFD, &temp[0], len, flags);
                        }else
                        {
                            bytes = send(client, &buffer[0], bytes, flags);
                        }

                    }

                    std::fill(begin(buffer), end(buffer), 0);
                }

            }
        }
    }

    close(server_fd);

    return EXIT_SUCCESS;
}
