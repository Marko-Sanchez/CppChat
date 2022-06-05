#pragma once
#include "Activty.hpp"
#include <cstdio>
#include <memory>
#include <raylib.h>
#include <string>
#include <utility>
#include <vector>
#include <list>
#include <future>
#include <map>
#include <sys/socket.h>

class Chat: public Activity
{
    public:

        /* Constructor, intitialize varaibles. */
        Chat()
        {
            chatScreen = {screenWidth / 3.0f, 25, screenWidth * 2.0f/3.0f - 25, screenHeight - 75};
            usersScreen = {25, 25, screenWidth / 3.0f - 50, screenHeight - 75};

            chatTextBox = {chatScreen.x, chatScreen.y + chatScreen.height + 5, chatScreen.width * 3.0f/4.0f, 40};
            chatButton = {chatScreen.x + chatScreen.width * 3.0f / 4.0f ,chatScreen.y + chatScreen.height + 5, chatScreen.width / 4.0f, 40};

            searchTextBox = {usersScreen.x, usersScreen.y + usersScreen.height + 5, usersScreen.width * 3.0f/4.0f, 40};
            searchButton = {usersScreen.x + usersScreen.width * 3.0f / 4.0f, usersScreen.y + usersScreen.height + 5, usersScreen.width / 4.0f, 40};

            font = LoadFont("raylib/examples/text/resources/fonts/pixelplay.png");
            textPosition = {chatTextBox.x + 5, chatTextBox.y + 5};
            spacing = 3.0f;

            chat_buffer.reserve(MAX_INPUT_CHAR);
            search_buffer.reserve(MAX_INPUT_CHAR);
            serverFD = -1;

            talkingTo = "server";
            addContact("server");
        }

        /*
         * Checks mouse position, gestures, and keys pressed to display
         * text onto GUI and message server.
         *
         * @output: fills 'chat_buffer' if user is typying in text box.
         */
        void processChat()
        {
            // if user presses button or enter key, add message to container:
            if(CheckCollisionPointRec(GetMousePosition(), chatButton) && IsGestureDetected(GESTURE_TAP) && chat_buffer.size() > 0)
            {
                // Send msg to server:
                std::string msgQuery = '$' + talkingTo + '$';
                msgQuery += chat_buffer;

                if(serverFD != -1)
                    send(serverFD, msgQuery.c_str(), msgQuery.length(), 0);

                messages[talkingTo].emplace_back(std::move(chat_buffer), Source::Local);
                chat_buffer.clear();
            }
            else if(chatTyping && IsKeyPressed(KEY_ENTER) && chat_buffer.size() > 0)
            {
                std::string msgQuery = '$' + talkingTo + '$';
                msgQuery += chat_buffer;

                if(serverFD != -1)
                    send(serverFD, msgQuery.c_str(), msgQuery.length(), 0);

                messages[talkingTo].emplace_back(std::move(chat_buffer), Source::Local);
                chat_buffer.clear();
            }

            // Does user want to type:
            if(CheckCollisionPointRec(GetMousePosition(), chatTextBox) && IsGestureDetected(GESTURE_TAP))
            {
                chatTyping = true;
            }
            else if(!CheckCollisionPointRec(GetMousePosition(), chatTextBox) && IsGestureDetected(GESTURE_TAP))
            {
                chatTyping = false;
            }

            if(chatTyping)
                typing(chat_buffer, MAX_INPUT_CHAR);

            // Does user want to search someone up
            if(CheckCollisionPointRec(GetMousePosition(), searchTextBox) && IsGestureDetected(GESTURE_TAP))
            {
                searchTyping = true;
            }else if(!CheckCollisionPointRec(GetMousePosition(), searchTextBox) && IsGestureDetected(GESTURE_TAP))
            {
                searchTyping = false;
            }

            if((CheckCollisionPointRec(GetMousePosition(), searchButton) && IsGestureDetected(GESTURE_TAP))
                || (searchTyping && IsKeyPressed(KEY_ENTER))
                && search_buffer.size() > 0)
            {

                // Query user on server:
                auto fut = std::async(std::launch::async, [this, name = search_buffer]() mutable{
                         std::string lookUp = "$" + name + "$";
                         send(serverFD, lookUp.c_str(), lookUp.length(), 0);
                        });
            }

            if (searchTyping)
                typing(search_buffer, MAX_INPUT_CHAR);

            if(!searchTyping && !chatTyping)
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            // update who user is talking to if user taps on friends 'Rectangle'.
            for(const auto& [name, rect]: friendList)
            {
                if(CheckCollisionPointRec(GetMousePosition(), rect) && IsGestureDetected(GESTURE_TAP))
                {
                    talkingTo = name;
                }
            }
        }

        /*
         * Draws chat screen objects such as user text, buttons, text box, and messages.
         */
        void drawChat()
        {
            DrawRectangleRec(chatScreen, GRAY);
            DrawRectangleRec(usersScreen, GRAY);

            // Draw chat text-box and button:
            DrawRectangleRounded(chatTextBox, 0.5f, 0, LIGHTGRAY);

            DrawTextEx(font, chat_buffer.c_str(), textPosition, 35, spacing, BLACK);

            if(chatTyping)
                DrawRectangleRoundedLines(chatTextBox, 0.6f , 0, 1, RED);
            else
                DrawRectangleRoundedLines(chatTextBox, 0.6f , 0, 1, DARKGRAY);

            DrawRectangleRounded(chatButton, 0.5f, 0, softblack);
            DrawText("send", chatButton.x + 70, chatButton.y + 10, 20, softwhite);

            // Draw search text-box and button:
            DrawRectangleRounded(searchTextBox, 0.5f, 0, LIGHTGRAY);

            DrawTextEx(font, search_buffer.c_str(), Vector2{searchTextBox.x + 5, searchTextBox.y + 5}, 35, spacing, BLACK);

            if(searchTyping)
                DrawRectangleRoundedLines(searchTextBox, 0.6f, 0, 1, RED);
            else
                DrawRectangleRoundedLines(searchTextBox, 0.6f, 0, 1, DARKGRAY);

            DrawRectangleRounded(searchButton, 0.5f, 0, softblack);
            DrawText("search", searchButton.x + 12, searchButton.y + 10, 19, softwhite);

            // Display text if user is searched for:
            if(userFound != 0 && userFound == 1)
            {
                DrawText("User added", screenWidth / 2 - 100, screenHeight / 2, 45, BLACK);

                if(++frameCounter > 120)
                {
                    frameCounter = 0;
                    userFound = 0;
                }
            }else if(userFound != 0 && userFound == -1)
            {
                DrawText("User not found", screenWidth / 2 - 100, screenHeight / 2, 45, RED);

                if(++frameCounter > 120)
                {
                    frameCounter = 0;
                    userFound = 0;
                }
            }

            // Draw messages boxes:
            auto bottom{chatScreen.y + chatScreen.height - 30};
            auto size = static_cast<int>(messages[talkingTo].size());

            for(int i = 0, j = size - 1; i < size; i++, j--)
            {
                auto &[msg, from] = messages[talkingTo][j];

                auto textLength = MeasureTextEx(font, msg.c_str(), 30, spacing);
                auto leftalign = chatScreen.x + chatScreen.width - (textLength.x + 10);

                // Draw rectangle and user text:
                if(from == Source::Local)
                {
                    DrawRectangleRounded(Rectangle{leftalign, bottom - 30 * i, textLength.x + 10, 29}, 0.5f, 0, BLUE);
                    DrawTextEx(font, msg.c_str(), {leftalign + 5, bottom - 30 * i}, 30, spacing, softwhite);
                }
                else
                {
                    DrawRectangleRounded(Rectangle{chatScreen.x, bottom - 30 * i, textLength.x + 10, 29}, 0.5f, 0, DARKGREEN);
                    DrawTextEx(font, msg.c_str(), {chatScreen.x + 5, bottom - 30 * i}, 30, spacing, softwhite);
                }

            }

            // Draw friends list:
            for(const auto &[name, rectptr]: friendList)
            {
                DrawRectangleRec(rectptr, softpurple);
                DrawRectangleLinesEx(rectptr,  1.0f, DARKGRAY);
                DrawText(name.c_str(), rectptr.x + 5, rectptr.y, 20, softwhite);
            }
        }

        /*
         * Adds message and who sent it to message container.
         * @param: name{std::string} Name of person who sent message.
         *         newMsg{std::string} message to add to container.
         *
         * @output: associates msg with who sent it.
        */
        void addMessage(std::string name, std::string &&newMsg) noexcept
        {
            messages[name].emplace_back(std::move(newMsg), Source::Remote);
        }

        /* Add a new contact to users friend list. */
        void addContact(std::string &&name) noexcept
        {
            messages.emplace(name, std::vector< std::pair<std::string, Source> >());

            friendList.emplace_back(std::move(name),
                    Rectangle{usersScreen.x, usersScreen.y + 45 * friendList.size(), usersScreen.width, 45}
                    );
        }

        /* Notifys if queried contact exist within server, adds contact to friends list. */
        void contactStatus(bool contactFound)
        {

            userFound = contactFound?1:-1;

            if(userFound == 1)
                addContact(std::move(search_buffer));

            search_buffer.clear();
        }

        /* Initialize server file descriptor. */
        void addServerFileD(int serverFD_) noexcept
        {
            serverFD = serverFD_;
        }

        /* Unload objects once no longer drawing. */
        void unload()
        {
            UnloadFont(font);
        }

    private:

        const int MAX_INPUT_CHAR{32};

        Rectangle chatScreen;
        Rectangle usersScreen;

        Rectangle searchTextBox;
        Rectangle searchButton;

        Rectangle chatTextBox;
        Rectangle chatButton;

        Vector2 textPosition;
        float spacing;
        Font font;

        enum class Source {Local, Remote};

        std::string chat_buffer;
        std::string search_buffer;
        std::string talkingTo;

        std::map<std::string, std::vector< std::pair<std::string, Source> >> messages;
        std::list< std::pair<std::string, Rectangle> > friendList;

        int serverFD;
        int userFound{0}, frameCounter{0};
        bool chatTyping{false};
        bool searchTyping{false};
};
