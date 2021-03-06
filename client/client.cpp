#include <cstring>
#include <iostream>
#include <cstdio>
#include <future>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/shm.h>

#include "login.hpp"
#include "title.hpp"
#include "chat.hpp"

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

/* output logging to terminal */
auto logP = [](std::string&& msg){std::cout << color.pass << msg << color.end << std::endl;};
auto logE = [](std::string&& msg){std::cerr << color.error << msg << color.end << std::endl;};
auto logW = [](std::string&& msg){std::cout << color.warning << msg << color.end << std::endl;};

/*
 * listens for activity on server and pipe file descriptors. Adds
 * msg recieved from server onto container to be displayed in Chat gui.
 * @params: serverFD{const int} file descriptor to write to server.
 *          chat {Chat &} object to Chat scene.
 *          selfPipe{int []} pipe file descriptor for inter-process communication.
 *
 * @output: adds message from server to container to be displayed in GUI.
 */
void handler(const int serverFD, Chat &chat, const int selfPipe[])
{

    char buffer[BUFFER_SIZE] = {0};
    const int flags{0};

    // read server response:
    size_t bytes = recv(serverFD,  &buffer, BUFFER_SIZE, flags);
    chat.addMessage("server", std::string(buffer));

    memset(buffer, 0, bytes);

    fd_set readfd;

    // Pass server fd to chat object:
    chat.addServerFileD(serverFD);

    while(true)
    {

        FD_ZERO(&readfd);
        FD_SET(serverFD, &readfd);
        FD_SET(selfPipe[0], &readfd);

        int nfds{std::max(serverFD, selfPipe[0]) + 1};

        // listen for activity from server and self:
        int ready{select(nfds, &readfd, nullptr, nullptr, nullptr)};

        if(ready < 0)
        {
            logE("server error");
            break;
        }

        if(FD_ISSET(selfPipe[0], &readfd))
        {
            // send 0 bytes to server, indicating closure:
            send(serverFD, buffer, 0, flags);
            logW("closing connection to server");

            break;
        }

        if(FD_ISSET(serverFD, &readfd))
        {
            bytes = recv(serverFD, &buffer, BUFFER_SIZE, flags);
            std::string query{buffer};

            if(bytes <= 0)
            {
                logW("server has disconnected");
                break;
            }
            else if(bytes == 3 && (buffer[0] == '$' && buffer[2] == '$'))
            {
                logW("Updated who client is talking to");
                chat.contactStatus((buffer[1] == '1')?true:false);
            }
            else
            {
                auto found = query.find('$', 1);
                std::string name{query.substr(1, found - 1)};
                std::cout << "Message from: " << name << std::endl;

                chat.addMessage(name, std::string(&buffer[found + 1]));
            }
        }

        memset(buffer, 0, bytes);
    }

    close(selfPipe[0]);        // close read end of pipe:
    close(selfPipe[1]);        // close write end of pipe:
    close(serverFD);           // close socket to server:
}

int main(int argc, char* argv[])
{
    /* Setting up socket and connection to server */

    struct sockaddr_in serverAddr;

    int server_fd{socket(AF_INET, SOCK_STREAM, 0)};

    if(server_fd < 0)
    {
        logE("socket connection failed");
        return EXIT_FAILURE;
    }

    logP("socket connected");

    // setup sockets method of connection:
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server:
    if(connect(server_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)))
    {
        logE("connection to server failed");
        return EXIT_FAILURE;
    }

    logP("connected to server");

    int selfPipe[2];
    if(pipe(selfPipe) == -1)
    {
        logE("error establishing pipe-to-self");
        close(server_fd);
        return EXIT_FAILURE;
    }


    /***************** DRAW GRAPHICS *******************/

    InitWindow(screenWidth, screenHeight, "CPP Chat");

    // Initialize screens:
    Login login;
    Chat chat;
    Title title;

    // first entering title: used to first notify server of clients name.
    bool inTitle{true};

    // We first start of on the login screen:
    Screen currScreen = Screen::LOGIN;

    auto chatFuture = std::async(std::launch::async, handler, server_fd, std::ref(chat), selfPipe);

    SetTargetFPS(45);
    while(!WindowShouldClose())
    {
        switch (currScreen)
        {
            case Screen::LOGIN:
            {

                login.proccessLogin(currScreen);

            }break;

            case Screen::TITLE:
            {

                // when first entering title send username to server:
                if(inTitle)
                {

                    auto fut = std::async(std::launch::async,
                            [userName = login.getUserName(), &server_fd](){

                                // send user name to server:
                                send(server_fd, userName.data(), userName.length(), 0);
                            });

                    inTitle = false;
                }

                title.processTitle(currScreen);

            }break;

            case Screen::CHATTING:
            {

                chat.processChat();

            }break;

            default: break;
        }

        BeginDrawing();
            ClearBackground(BACKGROUND);

            switch (currScreen) {
                case Screen::LOGIN:
                {

                    login.drawLogin();

                }break;

                case Screen::TITLE:
                {

                    title.drawTitle();

                }break;

                case Screen::CHATTING:
                {

                    chat.drawChat();

                }break;

                default: break;
            }

        EndDrawing();

    }

    title.unload();
    chat.unload();
    login.unload();

    CloseWindow();

    // close connection to server and stop listening:
    write(selfPipe[1], "X" /* arbitary string*/, 1);
    close(selfPipe[1]);

    return EXIT_SUCCESS;
}
