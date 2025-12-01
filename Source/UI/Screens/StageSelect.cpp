#include "StageSelect.h"
#include "MainMenu.h"
#include "CutsceneScreen.h"
#include "../../Game.h"
#include <SDL.h>

StageSelect::StageSelect(Game* game, const std::string& fontName)
    : UIScreen(game, fontName)
{
    Vector2 screenCenter(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT / 2.0f);

    // Background
    auto bg = AddImage("../Assets/Menus/menu.png", screenCenter, 1.0f);
    bg->SetSize(Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)));
    bg->SetColor(Vector4(0.2f, 0.2f, 0.2f, 1.0f));

    // Title
    auto title = AddText("SELECT STAGE", Vector2(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT * 0.15f), 1.5f);
    title->SetTextColor(Vector3(1.0f, 0.6f, 0.0f));

    // Stage 1 Button
    auto stage1Btn = AddButton("STAGE 1", [this]() {
        Close();
        new CutsceneScreen(mGame, GameScene::Level1);
    }, Vector2(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT * 0.30f), 0.8f);
    
    // Stage 2 Button
    auto stage2Btn = AddButton("STAGE 2", [this]() {
        Close();
        mGame->SetScene(GameScene::Level2);
    }, Vector2(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT * 0.45f), 0.8f);

    // Stage 3 Button
    auto stage3Btn = AddButton("STAGE 3", [this]() {
        Close();
        mGame->SetScene(GameScene::Level3);
    }, Vector2(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT * 0.60f), 0.8f);

    // Stage 4 Button
    auto stage4Btn = AddButton("STAGE 4 (TEST)", [this]() {
        Close();
        mGame->SetScene(GameScene::TestLevel);
    }, Vector2(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT * 0.75f), 0.8f);

    // Back Button
    auto backBtn = AddButton("BACK", [this]() {
        Close();
        new MainMenu(mGame, "../Assets/Fonts/ALS_Micro_Bold.ttf");
    }, Vector2(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT * 0.90f), 0.8f);
}

StageSelect::~StageSelect()
{
}

void StageSelect::HandleKeyPress(int key)
{
    if (mButtons.empty()) return;

    if (key == SDLK_DOWN || key == SDLK_s) 
    {
        if (mSelectedButtonIndex != -1) {
            mButtons[mSelectedButtonIndex]->SetHighlighted(false);
        }
        
        mSelectedButtonIndex++;
        if (mSelectedButtonIndex >= static_cast<int>(mButtons.size())) {
            mSelectedButtonIndex = 0;
        }
        
        mButtons[mSelectedButtonIndex]->SetHighlighted(true);
    }
    else if (key == SDLK_UP || key == SDLK_w) 
    {
        if (mSelectedButtonIndex != -1) {
            mButtons[mSelectedButtonIndex]->SetHighlighted(false);
        }

        mSelectedButtonIndex--;
        if (mSelectedButtonIndex < 0) {
            mSelectedButtonIndex = static_cast<int>(mButtons.size()) - 1;
        }

        mButtons[mSelectedButtonIndex]->SetHighlighted(true);
    }
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) 
    {
        if (mSelectedButtonIndex != -1) {
            mButtons[mSelectedButtonIndex]->OnClick();
        }
    }
    else if (key == SDLK_ESCAPE)
    {
        // Back to Main Menu on Escape
        Close();
        new MainMenu(mGame, "../Assets/Fonts/ALS_Micro_Bold.ttf");
    }
}
