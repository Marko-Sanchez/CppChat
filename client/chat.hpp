#pragma once
#include "Activty.hpp"
#include <memory>
#include <raylib.h>
#include <utility>
#include <vector>
#include <list>
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
            _serverfd = -1;//for testing

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
            if(CheckCollisionPointRec(GetMousePosition(), chatButton) && IsGestureDetected(GESTURE_TAP))
            {
                // Send msg to server:
                auto length = chat_buffer.length();
                if(_serverfd != -1)
                    send(_serverfd, chat_buffer.c_str(), length, 0);

                messages.emplace_back(std::move(chat_buffer), Source::Local);
                chat_buffer.clear();
            }
            else if(chatTyping && IsKeyPressed(KEY_ENTER))
            {
                auto length = chat_buffer.length();
                if(_serverfd != -1)
                    send(_serverfd, chat_buffer.c_str(), length, 0);

                messages.emplace_back(std::move(chat_buffer), Source::Local);
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
            else
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            // Does user want to search someone up
            if(CheckCollisionPointRec(GetMousePosition(), searchTextBox) && IsGestureDetected(GESTURE_TAP))
            {
                searchTyping = true;
            }else if(!CheckCollisionPointRec(GetMousePosition(), searchTextBox) && IsGestureDetected(GESTURE_TAP))
            {
                searchTyping = false;
            }

            if(CheckCollisionPointRec(GetMousePosition(), searchButton) && IsGestureDetected(GESTURE_TAP))
            {
                // TODO: Query user on server:
                printf("Testing send button\n");
                search_buffer.clear();
            }

            if (searchTyping)
                typing(search_buffer, MAX_INPUT_CHAR);
            else
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            // TODO: Add collision detection for user list
            // when pressed that user will be contacted.
            // when searching for user query server for the users name.
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

            // Draw messages boxes:
            auto bottom{chatScreen.y + chatScreen.height - 30};
            for(int i = 0, j = messages.size() - 1; i < messages.size(); i++, j--)
            {
                auto &[msg, from] = messages[j];

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
            int i = 0;
            for(const auto &[name, rectptr]: friendList)
            {
                DrawRectangleRec(rectptr, softpurple);
                DrawRectangleLinesEx(rectptr,  1.0f, DARKGRAY);
                DrawText(name.c_str(), rectptr.x + 5, rectptr.y + 45 * i, 20, softwhite);

                ++i;
            }
        }

        /* Add string and message source to container to be displayed in GUI. */
        void addMessage(std::string &&newMsg) noexcept
        {
            messages.emplace_back(std::move(newMsg), Source::Remote);
        }

        /* Add a new contact to users friend list. */
        void addContact(std::string &&name) noexcept
        {
            friendList.emplace_back(std::move(name),
                    Rectangle{usersScreen.x, usersScreen.y + 45 * friendList.size(), usersScreen.width, 45}
                    );
        }

        /* Required to be able to communicate to server. */
        void addServerFileD(int serverFD) noexcept
        {
            _serverfd = serverFD;
        }

        /* Unload objects once no longer drawing. */
        void unload()
        {
            UnloadFont(font);
        }

    private:

        // TODO: Create a box for adding users:
        const int MAX_INPUT_CHAR{16};

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
        /* std::unordered_map<std::string, std::vector<std::string, SOURCE>> messages; */
        std::vector<std::pair<std::string, Source>> messages;
        std::list< std::pair<std::string, Rectangle> > friendList;

        int _serverfd;
        bool chatTyping{false};
        bool searchTyping{false};
};
