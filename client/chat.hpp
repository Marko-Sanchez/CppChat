#pragma once
#include "Activty.hpp"
#include <vector>

class Chat: public Activity
{
    public:

        Chat()
        {
            chatScreen = {screenWidth / 3.0f, 25, screenWidth * 2.0f/3.0f - 25, screenHeight - 75};
            usersScreen = {25, 25, screenWidth / 3.0f - 50, screenHeight - 75};

            chatTextBox = {chatScreen.x, chatScreen.y + chatScreen.height + 5, chatScreen.width * 3.0f/4.0f, 40};
            chatButton = {chatScreen.x + chatScreen.width * 3.0f / 4.0f ,chatScreen.y + chatScreen.height + 5, chatScreen.width / 4.0f, 40};

            chat_buffer.reserve(MAX_INPUT_CHAR);
        }

        void processChat()
        {
            // for now if user clicks on box add something, later add button:
            if(CheckCollisionPointRec(GetMousePosition(), chatButton) && IsGestureDetected(GESTURE_TAP))
            {
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

        void drawChat()
        {
            DrawRectangleRec(chatScreen, GRAY);
            DrawRectangleRec(usersScreen, GRAY);

            DrawRectangleRounded(chatTextBox, 0.5f, 0, LIGHTGRAY);
            DrawText(chat_buffer.c_str(), chatTextBox.x + 5, chatTextBox.y + 5, 35, BLACK);

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
                DrawText(messages[messages.size() - i - 1].c_str(), chatScreen.x + 5, bottom - 30 * i, 30, LIME);
            }
        }

    private:

        const int MAX_INPUT_CHAR{16};

        Rectangle chatScreen;
        Rectangle usersScreen;

        Rectangle chatTextBox;
        Rectangle chatButton;

        std::string chat_buffer;
        std::vector<std::string> messages;

        bool chatTyping{false};
};
