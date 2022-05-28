#pragma once
#include "Activty.hpp"
#include <raylib.h>

class Login: private Activity
{

    public:

        /* Constructor, initialize variables */
        Login()
        {
            login_box = {screenWidth / 2.0f - 225, screenHeight / 2.0f - 225, 450, 450};
            username_box = {screenWidth / 2.0f - 175, login_box.y + 115, 350, 50};
            password_box = {username_box.x, username_box.y + 115, 350, 50};
            button_box = {password_box.x, password_box.y + 75, 350, 50};

            font = LoadFont("raylib/examples/text/resources/fonts/pixelplay.png");
            spacing = 3.0f;

            username_buffer.reserve(MAX_INPUT_CHAR);
            password_buffer.reserve(MAX_INPUT_CHAR);
        }

        /*
         * Checks if client is selecting a text-box to type in.
         * @params: currScreen { Screen &} current screen being displayed.
         *
         * @outputs: logic for login screen to be drawn.
         */
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
                currScreen = Screen::TITLE;
            else
                buttonPressed = false;

        }

        /* Draw login screen, background, text box, and button. */
        void drawLogin()
        {
            // Draw login box:
            DrawRectangleRec(login_box, softblack);

            // Draw text box and user text inside text box:
            DrawRectangleRec(username_box, LIGHTGRAY);
            DrawText("name", (int) username_box.x, (int) username_box.y - 25, 25, softwhite);
            DrawTextEx(font, username_buffer.c_str(), Vector2{ username_box.x + 5, username_box.y + 8 }, 40, spacing, BLACK);

            // draw red outline on textbox:
            if(usernameTyping)
                DrawRectangleLinesEx(username_box, 1, RED);
            else
                DrawRectangleLinesEx(username_box, 1, DARKGRAY);

            // Draw password box:
            DrawRectangleRec(password_box, LIGHTGRAY);
            DrawText("password", (int) password_box.x, (int) password_box.y - 25, 25, softwhite);
            DrawTextEx(font, password_buffer.c_str(), Vector2{ password_box.x + 5, password_box.y + 8 }, 40, spacing, BLACK);

            if(passwordTyping)
                DrawRectangleLinesEx(password_box, 1, RED);
            else
                DrawRectangleLinesEx(password_box, 1, DARKGRAY);

            // Draw login button box:
            if(buttonPressed)
                DrawRectangleRounded(button_box, 0.5f, 0, softpurple);
            else
                DrawRectangleRounded(button_box, 0.5f, 0, BACKGROUND);

            DrawText("Log In", (int)  button_box.x + 137, (int) button_box.y + 15, 20, softwhite);

        }

        /* return users name */
        std::string getUserName()
        {
            return username_buffer;
        }

        /* Unload objects after done drawing. */
        void unload()
        {
            UnloadFont(font);
        }

    private:

        const int MAX_INPUT_CHAR{16};

        Rectangle login_box;
        Rectangle username_box;
        Rectangle password_box;

        Rectangle button_box;

        float spacing;
        Font font;

        bool usernameTyping{false}, passwordTyping{false};
        bool buttonPressed{false};

        std::string username_buffer;
        std::string password_buffer;
};
