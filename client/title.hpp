#pragma once
#include "Activty.hpp"

class Title: public Activity
{

    public:

        Title()
        {
            font = LoadFont("raylib/examples/text/resources/fonts/dejavu.fnt");
            texture = LoadTexture("resources/img/kind-Mountain.png");

            titlePosition = { screenWidth / 2.0f - 100,  screenHeight / 2.0f};
            spacing = 3.0f;
            framesCounter = 0;
        }

        void processTitle(Screen &currScreen)
        {
            if(++framesCounter > 300)
            {
                currScreen = Screens::ACTIVITY;
                framesCounter = 0;
            }
        }

        void drawTitle()
        {
            DrawTexture(texture, screenWidth / 2 - texture.width / 2, screenHeight / 2 - texture.height / 2, LIGHTGRAY);
            DrawTextEx(font, "KIND", titlePosition, 64, spacing, BLACK);
        }

        void unload()
        {
            UnloadTexture(texture);
            UnloadFont(font);
        }

    private:

        Vector2 titlePosition;
        Texture2D texture;
        Font font;

        float spacing;
        int framesCounter;
};
