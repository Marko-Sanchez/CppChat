#pragma once
#include "Activty.hpp"

class Title: public Activity
{

    public:

        /* Constructor intitialize variables, load Fonts and textures. */
        Title()
        {
            font = LoadFont("raylib/examples/text/resources/fonts/jupiter_crash.png");
            texture = LoadTexture("resources/img/kind-Mountain.png");

            titlePosition = { screenWidth / 2.0f - 75,  screenHeight / 2.0f};
            spacing = 3.0f;
            framesCounter = 0;
        }

        /*
         * Waits in title for 3 seconds, before switching scene to chat screen.
         * @params: currScreen { Screen &} current screen being displayed.
         *
         * @output: changes scene after certain amount of time.
         */
        void processTitle(Screen &currScreen)
        {
            if(++framesCounter > 300)
            {
                currScreen = Screens::CHATTING;
                framesCounter = 0;
            }
        }

        /* Draws background Image and Text. */
        void drawTitle()
        {
            DrawTexture(texture, screenWidth / 2 - texture.width / 2, screenHeight / 2 - texture.height / 2, LIGHTGRAY);
            DrawTextEx(font, "KIND", titlePosition, 94, spacing, BLACK);
        }

         /* Unload objects after done drawing. */
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
