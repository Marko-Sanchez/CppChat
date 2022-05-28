#include "login.hpp"
#include "title.hpp"
#include "chat.hpp"

int main(void)
{

    InitWindow(screenWidth, screenHeight, "CPP Chat");

    // Initialize screens:
    Login login;
    Chat chat;
    Title title;

    Screen currScreen = Screen::LOGIN;

    SetTargetFPS(45);
    while(!WindowShouldClose())
    {
        switch (currScreen)
        {
            case Screen::TITLE:
            {

                title.processTitle(currScreen);

            }break;

            case Screen::LOGIN:
            {

                login.proccessLogin(currScreen);

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
                case Screen::TITLE:
                {

                    title.drawTitle();

                }break;

                case Screen::LOGIN:
                {

                    login.drawLogin();

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
    return 0;
}
