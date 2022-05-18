#include "login.hpp"
#include "title.hpp"
#include "chat.hpp"

int main(void)
{

    InitWindow(screenWidth, screenHeight, "CPP Chat");

    Login login;
    Chat chat;
    Title title;

    Screen currScreen = Screens::LOGIN;

    SetTargetFPS(45);
    while(!WindowShouldClose())
    {
        switch (currScreen)
        {
            case Screens::TITLE:
            {

                title.processTitle(currScreen);

            }break;

            case Screens::LOGIN:
            {

                login.proccessLogin(currScreen);

            }break;

            case Screens::ACTIVITY:
            {

                chat.processChat();

            }break;
            default: break;
        }

        BeginDrawing();
            ClearBackground(BACKGROUND);

            switch (currScreen) {
                case Screens::TITLE:
                {

                    title.drawTitle();

                }break;

                case Screens::LOGIN:
                {

                    login.drawLogin();

                }break;

                case Screens::ACTIVITY:
                {

                    chat.drawChat();

                }break;

                default: break;
            }

        EndDrawing();

    }

    title.unload();

    CloseWindow();
    return 0;
}
