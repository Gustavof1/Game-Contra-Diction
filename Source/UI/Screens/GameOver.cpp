#include "GameOver.h"
#include "../../Game.h"
#include <SDL.h>

GameOver::GameOver(class Game* game, const std::string& fontName, GameScene level)
        :UIScreen(game, fontName)
        ,mlevel(level)
        ,mFadeState(FadeState::FadingIn)
        ,mFadeTimer(1.0f)
        ,mFadeRect(nullptr)
{

    Vector2 screenCenter(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT/2.0f);
    
    auto bg = AddImage("../Assets/Menus/GameOver.png", screenCenter, 1.0f);
    
    bg->SetSize(Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)));

    mFadeRect = AddRect(screenCenter, Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)), 1.0f, 0.0f, 200);
    mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    auto restartBtn = AddButton("", [this]() {
        if (mFadeState != FadeState::None) return;
        mFadeState = FadeState::FadingOut;
        mFadeTimer = 0.0f;
        mNextAction = [this]() {
            Close();
            mGame->SetScene(mlevel);
        };
    }, Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT * 0.65f), 0.5f); 
    
    restartBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/retry_normal.png",
                            "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/retry_hover.png",
                            "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/retry_pressed.png");

    auto menuBtn = AddButton("", [this]() {
        if (mFadeState != FadeState::None) return;
        mFadeState = FadeState::FadingOut;
        mFadeTimer = 0.0f;
        mNextAction = [this]() {
            Close();
            mGame->SetScene(GameScene::MainMenu);
        };
    }, Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT * 0.75f), 0.5f);
    
    menuBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/back_normal.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/back_hover.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/back_pressed.png");

    auto exitBtn = AddButton("", [this]() {
        if (mFadeState != FadeState::None) return;
        mGame->Quit();
    }, Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT * 0.85f), 0.5f);
    
    exitBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_normal.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_hover.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_pressed.png");
}

GameOver::~GameOver()
{
}

void GameOver::HandleKeyPress(int key)
{

    if (mButtons.empty()) return;

    if (key == SDLK_DOWN || key == SDLK_s) 
    {
        // Tira o destaque do atual
        if (mSelectedButtonIndex != -1) {
            mButtons[mSelectedButtonIndex]->SetHighlighted(false);
        }
        
        // Avança o índice
        mSelectedButtonIndex++;
        if (mSelectedButtonIndex >= static_cast<int>(mButtons.size())) {
            mSelectedButtonIndex = 0;
        }
        
        // Destaca o novo
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
}

void GameOver::Update(float deltaTime)
{
    UIScreen::Update(deltaTime);

    if (mFadeState == FadeState::FadingIn)
    {
        mFadeTimer -= deltaTime;
        if (mFadeTimer <= 0.0f)
        {
            mFadeTimer = 0.0f;
            mFadeState = FadeState::None;
        }
        mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, mFadeTimer));
    }
    else if (mFadeState == FadeState::FadingOut)
    {
        mFadeTimer += deltaTime;
        if (mFadeTimer >= 1.0f)
        {
            mFadeTimer = 1.0f;
            if (mNextAction)
            {
                mNextAction();
            }
        }
        mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, mFadeTimer));
    }
}