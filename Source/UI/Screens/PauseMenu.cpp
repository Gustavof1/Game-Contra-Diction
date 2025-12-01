//
// Created by Vinicius Trindade on 12/11/25.
//

#include "PauseMenu.h"
#include "../../Game.h"

PauseMenu::PauseMenu(class Game* game, const std::string& fontName, GameScene level)
        :UIScreen(game, fontName)
        ,mlevel(level)
        ,mResuming(false)
{
    mGame->SetState(GameState::Paused);

    mGame->GetAudio()->PauseAllSounds();

    Vector2 screenSize(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT));
    Vector2 screenCenter(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT / 2.0f);
    
    auto* bgRect = AddRect(screenCenter, screenSize);
    bgRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.7f)); 

    // Title
    auto title = AddText("PAUSED", Vector2(screenCenter.x, screenCenter.y - 150.0f), 1.5f);
    title->SetTextColor(Vector3(1.0f, 0.6f, 0.0f)); // Orange

    auto resumeBtn = AddButton("", [this]() {
        mResuming = true;
        Close();
    }, Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT * 0.6f), 0.5f);

    resumeBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/play_normal.png",
                           "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/play_hover.png",
                           "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/play_pressed.png");

    auto restartBtn = AddButton("", [this]() {
        Close();
        mGame->SetScene(mlevel);
    }, Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT * 0.7f), 0.5f); 
    
    restartBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/retry_normal.png",
                            "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/retry_hover.png",
                            "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/retry_pressed.png");

    auto menuBtn = AddButton("", [this]() {
        Close();
        mGame->SetScene(GameScene::MainMenu);
    }, Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT * 0.8f), 0.5f);
    
    menuBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/back_normal.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/back_hover.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/back_pressed.png");

    auto exitBtn = AddButton("", [this]() {
        mGame->Quit();
    }, Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT * 0.9f), 0.5f);
    
    exitBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_normal.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_hover.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_pressed.png");
}

PauseMenu::~PauseMenu()
{
    if (mResuming) {
        mGame->GetAudio()->ResumeAllSounds();
    }
    else {
        mGame->GetAudio()->StopAllSounds();
    }
    mGame->SetState(GameState::Gameplay);
}

void PauseMenu::HandleKeyPress(int key)
{
    UIScreen::HandleKeyPress(key);

    if (key == SDLK_DOWN || key == SDLK_s) {
        mButtons[mSelectedButtonIndex]->SetHighlighted(false);
        mSelectedButtonIndex = (mSelectedButtonIndex + 1) % mButtons.size();
        mButtons[mSelectedButtonIndex]->SetHighlighted(true);
    }
    else if (key == SDLK_UP || key == SDLK_w) {
        mButtons[mSelectedButtonIndex]->SetHighlighted(false);
        mSelectedButtonIndex--;
        if (mSelectedButtonIndex < 0) {
            mSelectedButtonIndex = mButtons.size() - 1;
        }
        mButtons[mSelectedButtonIndex]->SetHighlighted(true);
    }
    else if (key == SDLK_RETURN) { 
        if (mSelectedButtonIndex != -1) {
            mButtons[mSelectedButtonIndex]->OnClick();
        }
    }
    else if (key == SDLK_ESCAPE)
    {
        mResuming = true;
        Close();
    }
}