#pragma once
#include "Activty.hpp"

class Login: private Activity
{

    public:

        Login()
        {
            login_box = {screenWidth / 2.0f - 225, screenHeight / 2.0f - 225, 450, 450};
            username_box = {screenWidth / 2.0f - 175, login_box.y + 115, 350, 50};
            password_box = {username_box.x, username_box.y + 115, 350, 50};
            button_box = {password_box.x, password_box.y + 75, 350, 50};


            username_buffer.reserve(MAX_INPUT_CHAR);
            password_buffer.reserve(MAX_INPUT_CHAR);
        }

        void proccessLogin(Screen &currScreen)
        {

            // text-box logic:
            if(CheckCollisionPointRec(GetMousePosition(), username_box) && IsGestureDetected(GESTURE_TAP))
            {
                usernameTyping = true;
            }
            else if(!CheckCollisionPointRec(GetMousePosition(), username_box) && IsGestureDetected(GESTURE_TAP))
            {
                usernameTyping = false;
            }

            if(CheckCollisionPointRec(GetMousePosition(), password_box) && IsGestureDetected(GESTURE_TAP))
            {
                passwordTyping = true;
            }
            else if(!CheckCollisionPointRec(GetMousePosition(), password_box) && IsGestureDetected(GESTURE_TAP))
            {
                passwordTyping = false;
            }

            if(usernameTyping)
                typing(username_buffer, MAX_INPUT_CHAR);
            else if(passwordTyping)
                typing(password_buffer, MAX_INPUT_CHAR);
            else
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            // button logic:
            if(CheckCollisionPointRec(GetMousePosition(), button_box) && !IsGestureDetected(GESTURE_TAP))
                buttonPressed = true;
            else if(CheckCollisionPointRec(GetMousePosition(), button_box) && IsGestureDetected(GESTURE_TAP))
                currScreen = Screens::TITLE;
            else
                buttonPressed = false;

        }

        void drawLogin()
        {
            // Draw login box:
            DrawRectangleRec(login_box, softblack);

            // Draw text box and text inside text box:
            DrawRectangleRec(username_box, LIGHTGRAY);
            DrawText("name", (int) username_box.x, (int) username_box.y - 25, 25, softwhite);
            DrawText(username_buffer.c_str(), (int)username_box.x + 5, (int)username_box.y + 8, 40, BLACK);

            // draw red outline on textbox:
            if(usernameTyping)
                DrawRectangleLines((int) username_box.x, (int) username_box.y, (int) username_box.width, (int) username_box.height, RED);
            else
                DrawRectangleLines((int) username_box.x, (int) username_box.y, (int) username_box.width, (int) username_box.height, DARKGRAY);

            // Draw pasword box:
            DrawRectangleRec(password_box, LIGHTGRAY);
            DrawText("password", (int) password_box.x, (int) password_box.y - 25, 25, softwhite);
            DrawText(password_buffer.c_str(), (int) password_box.x + 5, (int) password_box.y + 8, 40, BLACK);

            if(passwordTyping)
                DrawRectangleLines((int) password_box.x, (int) password_box.y, (int) password_box.width, (int) password_box.height, RED);
            else
                DrawRectangleLines((int) password_box.x, (int) password_box.y, (int) password_box.width, (int) password_box.height, DARKGRAY);

            // Draw login button box:
            if(buttonPressed)
                DrawRectangleRounded(button_box, 0.5f, 0, {132, 109, 145, 255});
            else
                DrawRectangleRounded(button_box, 0.5f, 0, BACKGROUND);

            DrawText("Log In", (int)  button_box.x + 137, (int) button_box.y + 15, 20, softwhite);

        }

    private:

        const int MAX_INPUT_CHAR{16};

        Rectangle login_box;
        Rectangle username_box;
        Rectangle password_box;

        Rectangle button_box;

        bool usernameTyping{false}, passwordTyping{false};
        bool buttonPressed{false};

        std::string username_buffer;
        std::string password_buffer;
};
