#pragma once
#include "Activty.hpp"
#include <vector>
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

            font = LoadFont("raylib/examples/text/resources/fonts/setback.png");
            textPosition = {chatTextBox.x + 5, chatTextBox.y + 5};
            spacing = 3.0f;

            chat_buffer.reserve(MAX_INPUT_CHAR);
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
                send(_serverfd, chat_buffer.c_str(), length, 0);

                messages.emplace_back(std::move(chat_buffer));
                chat_buffer.clear();
            }
            else if(chatTyping && IsKeyPressed(KEY_ENTER))
            {
                auto length = chat_buffer.length();
                send(_serverfd, chat_buffer.c_str(), length, 0);

                messages.emplace_back(std::move(chat_buffer));
                chat_buffer.clear();
            }

            //Does user want to type:
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

        }

        /*
         * Draws chat screen objects such as user text, buttons, text box, and messages.
         */
        void drawChat()
        {
            DrawRectangleRec(chatScreen, GRAY);
            DrawRectangleRec(usersScreen, GRAY);

            DrawRectangleRounded(chatTextBox, 0.5f, 0, LIGHTGRAY);

            DrawTextEx(font, chat_buffer.c_str(), textPosition, 35, spacing, BLACK);

            if(chatTyping)
                DrawRectangleRoundedLines(chatTextBox, 0.6f , 0, 1, RED);
            else
                DrawRectangleRoundedLines(chatTextBox, 0.6f , 0, 1, DARKGRAY);

            DrawRectangleRounded(chatButton, 0.5f, 0, softblack);
            DrawText("send", chatButton.x + 70, chatButton.y + 10, 20, softwhite);

            auto bottom{chatScreen.y + chatScreen.height - 30};
            for(int i = 0; i < messages.size(); i++)
            {
                // Draw rectangle:
                if(i % 2 == 0)
                    DrawRectangle(chatScreen.x, bottom - 30 * i, chatScreen.width, 30, LIGHTGRAY);
                else
                    DrawRectangle(chatScreen.x, bottom - 30 * i, chatScreen.width, 30, DARKGRAY);

                // add user text to rectangle:
                DrawTextEx(font, messages[messages.size() - i - 1].c_str(), {chatScreen.x + 5, bottom - 30 * i}, 30, spacing, LIME);
            }
        }

        /* Add string to container to be displayed in GUI. */
        void addMessage(std::string &&newMsg)
        {
            messages.emplace_back(std::move(newMsg));
        }

        /* Required to be able to communicate to server. */
        void addServerFileD(int serverFD)
        {
            _serverfd = serverFD;
        }

        /* Unload objects once no longer drawing. */
        void unload()
        {
            UnloadFont(font);
        }

    private:

        const int MAX_INPUT_CHAR{16};

        Rectangle chatScreen;
        Rectangle usersScreen;

        Rectangle chatTextBox;
        Rectangle chatButton;

        Vector2 textPosition;
        float spacing;
        Font font;

        std::string chat_buffer;
        std::vector<std::string> messages;

        int _serverfd;
        bool chatTyping{false};
};
