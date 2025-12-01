#include "LoadingScreen.h"
#include "../../Game.h"
#include "../../Renderer/Renderer.h"

LoadingScreen::LoadingScreen(Game* game)
    :UIScreen(game, "../Assets/Fonts/ALS_Micro_Bold.ttf")
{
    // Add "LOADING..." text in the center
    AddText("LOADING...", Vector2(0.0f, 0.0f), 1.0f, 0.0f, 60);
}

LoadingScreen::~LoadingScreen()
{
}
