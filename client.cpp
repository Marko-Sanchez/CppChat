#include <cstring>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <future>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/shm.h>

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

/*
 * function to be passed to std::async(), to be processed asynchronously.
 * Reads user input from terminal, then sends message to server.
 * @params:     serverFD{const int} file descriptor to write to server.
 *              pfdWrite{const int} file descriptor to alert self to close.
 *
 * @output: sends message to server.
 */
void clientInput(const int serverFD, const int pfdWrite)
{
    char buffer[BUFFER_SIZE] = {0};
    int flags{0};
    ssize_t bytes{0};

    std::cout << "> ";

    while(std::cin.getline(buffer, BUFFER_SIZE))
    {

        // write message to server:
        auto length = static_cast<ssize_t>(strlen(buffer));
        bytes = send(serverFD, buffer, length, flags);

        if(bytes <= 0)
        {
            write(pfdWrite, buffer, 0); // write to self, to close connection:
            close(pfdWrite);            // close write end of pipe:
            break;
        }

        std::cout << "> ";
    }

}

/*
 * listens for activity on server and pipe file descriptors. If server
 * outputs message onto terminal, else begins shutting down process.
 * Launches thread to listen for user input on terminal.
 * @params:     serverFD{const int} file descriptor to write to server.
 *
 * @output: message from server onto terminal.
 */
void chat(const int serverFD)
{

    char buffer[BUFFER_SIZE] = {0};
    int flags{0};
    ssize_t bytes{0};

    // output server response:
    bytes = recv(serverFD,  &buffer, sizeof(buffer), flags);
    std::cout << color.sMsg << buffer << color.end << std::endl;

    fd_set socketset;
    pid_t childpid;
    int pipefd[2];

    // establish pipe to write to self:
    if(pipe(pipefd) == -1)
    {
        std::cerr << color.error << "error establishing pipe-to-self" << color.end << std::endl;

        close(serverFD);
        return;
    }

    // task for processsing user input:
    auto ft = std::async(std::launch::async, clientInput, serverFD, pipefd[1]);

    int nfds{std::max(serverFD, pipefd[0]) + 1};

    // make shared memory:
    int memID = shmget(IPC_PRIVATE, sizeof(bool), IPC_CREAT|0666);
    bool *ptr = static_cast<bool*>(shmat(memID, nullptr, 0));
    *ptr = true;

    while(*ptr)
    {

        FD_ZERO(&socketset);
        FD_SET(serverFD, &socketset);
        FD_SET(pipefd[0], &socketset);


        // listen for activity from server and self:
        int ready{select(nfds, &socketset, nullptr, nullptr, nullptr)};

        if(ready <= 0)
        {
            std::cerr << color.error << "server error" << color.end << std::endl;
            break;
        }
        else if(FD_ISSET(pipefd[0], &socketset))
        {
            send(serverFD, buffer, 0, flags);// send 0 bytes to server, indicating closure:

            std::cout << color.warning << "closing connection to server" << color.end << std::endl;
            break;
        }
        else if(FD_ISSET(serverFD, &socketset))
        {
            // output server response, in a child process:
            if((*ptr == true) && (childpid = fork()) == 0)
            {
                bytes = recv(serverFD, &buffer, BUFFER_SIZE, flags);
                if(bytes <= 0)
                {
                    *ptr = false;
                    std::cout << color.warning << "server has disconnected" << color.end << std::endl;
                }else
                {
                    std::cout << "Server: " << buffer << std::endl;
                    std::cout << "> ";
                    std::flush(std::cout);
                }

                close(serverFD);
                exit (0);
            }
        }
        else{std::cout << "something else" << std::endl;}

        memset(buffer, 0, bytes);
    }

    close(pipefd[0]);        // close read end of pipe:
    close(pipefd[1]);        // close write end of pipe:
    close(serverFD);         // close socket to server:

    shmdt(ptr);             // detach connection to shared memory:
    shmctl(memID, IPC_RMID, nullptr);// destroy shared memory:
}

int main(int argc, char* argv[])
{

    /* Setting up socket and connection to server */

    struct sockaddr_in serverAddr;

    int server_fd{socket(AF_INET, SOCK_STREAM, 0)};

    if(server_fd < 0)
    {
        std::cerr << color.error << "socket connection failed" << color.end << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << color.pass << "socket connected" << color.end << std::endl;

    // setup sockets method of connection:
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server: Can use this same method to connect to another client, once serversneds back client info for us
    if(connect(server_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)))
    {
        std::cerr << color.error << "connection to server failed" << color.end << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << color.pass << "connected to server" << color.end << std::endl;

    chat(server_fd);

    return EXIT_SUCCESS;
}
