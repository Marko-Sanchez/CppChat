#include <raylib.h>
#include <string>

constexpr int screenWidth{1200};
constexpr int screenHeight{750};
constexpr int MAX_INPUT_CHAR{12};

/* Custom Colors */
constexpr Color BACKGROUND{67, 49, 92, 255};
constexpr Color softblack({20, 22, 23, 225});
constexpr Color softwhite({184, 192, 161, 255});

using Screen = enum class Screens:int{LOGIN = 0, TITLE, ACTIVITY};

/*
 * Grab user input when typing in textbox.
 *
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

int main(void)
{

    InitWindow(screenWidth, screenHeight, "CPP Chat");

    // Load font:
    Font font = LoadFont("raylib/examples/text/resources/fonts/romulus.png");
    Vector2 titlePosition = { screenWidth / 2.0f - 100,  screenHeight / 2.0f};
    float spacing{3};

    // Load image:
    Texture2D texture = LoadTexture("resources/img/kind-Mountain.png");

    // Login page stuff:
    Rectangle login_box = {screenWidth / 2.0f - 225, screenHeight / 2.0f - 225, 450, 450};
    Rectangle username_box = {screenWidth / 2.0f - 175, login_box.y + 115, 350, 50};
    Rectangle password_box = {username_box.x, username_box.y + 115, 350, 50};
    Rectangle button_box = {password_box.x, password_box.y + 75, 350, 50};

    std::string username_buffer;
    username_buffer.reserve(MAX_INPUT_CHAR);

    std::string password_buffer;
    password_buffer.reserve(MAX_INPUT_CHAR);

    bool usernameTyping{false}, passwordTyping{false};
    bool buttonPressed{false};

    Screen currScreen = Screens::LOGIN;
    int framesCounter{0};

    SetTargetFPS(45);
    while(!WindowShouldClose())
    {
        switch (currScreen)
        {
            case Screens::TITLE:
            {
                framesCounter++;

                if(framesCounter > 300)
                {
                    currScreen = Screens::LOGIN;
                    framesCounter = 0;
                }
            }break;

            default: break;
        }

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

        BeginDrawing();
            ClearBackground(BACKGROUND);

            switch (currScreen) {
                case Screens::TITLE:
                {
                    DrawTexture(texture, screenWidth / 2 - texture.width / 2, screenHeight / 2 - texture.height / 2, LIGHTGRAY);
                    DrawTextEx(font, "KIND", titlePosition, 64, spacing, BLACK);
                }break;

                case Screens::LOGIN:
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


                }break;

                default: break;
            }

        EndDrawing();

    }

    // Unload stuff:
    UnloadTexture(texture);
    UnloadFont(font);

    // Close window and OpenGL context
    CloseWindow();
    return 0;
}
