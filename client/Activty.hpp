#pragma once
#include <raylib.h>
#include <string>

using Screen = enum class Screens:int{LOGIN = 0, TITLE, CHATTING};

constexpr int screenWidth{1200};
constexpr int screenHeight{750};

/* Custom Colors */
constexpr Color BACKGROUND({67, 49, 92, 255});
constexpr Color softblack({20, 22, 23, 225});
constexpr Color softwhite({184, 192, 161, 255});


class Activity
{
    public:

        /*
         * Grab user input when typing in textbox.
         * @params: buffer {std::string &} contains text for specfic text-box.
         *          MAX_INPUT {const int} max amount of characters for text-box.
         *
         * @ouput: buffer should contain string user typed in text-box.
         */
        void typing(std::string &buffer, const int MAX_INPUT)
        {

            SetMouseCursor(MOUSE_CURSOR_IBEAM);
            int key = GetCharPressed();

            while(key > 0)
            {
                if(key >= 32 && key <= 132 && buffer.length() < MAX_INPUT)
                {
                    buffer.push_back(static_cast<char>(key));
                }

                key = GetCharPressed();
            }

            // check if user tried to delete some letters:
            if(IsKeyPressed(KEY_BACKSPACE) && buffer.length() > 0)
            {
                buffer.pop_back();
            }

        }

};
