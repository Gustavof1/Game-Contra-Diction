#include "MainMenu.h"
#include "CutsceneScreen.h"
#include "StageSelect.h"
#include "OptionsScreen.h"
#include "../../Game.h"
#include <SDL.h>
#include <algorithm>

MainMenu::MainMenu(class Game* game, const std::string& fontName)
        :UIScreen(game, fontName)
        ,mIsFadingOut(false)
        ,mFadeTimer(0.0f)
        ,mCheatCodeIndex(0)
{
    // Konami Code: Up, Up, Down, Down, Left, Right, Left, Right
    mCheatCode = { SDLK_UP, SDLK_UP, SDLK_DOWN, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_LEFT, SDLK_RIGHT };

    Vector2 screenCenter(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT/2.0f);
    // 1. Background
    // Ajuste o caminho da imagem conforme sua pasta Assets
    auto bg = AddImage("../Assets/Menus/menu.png", screenCenter, 1.0f);
    
    // --- FORÇA O TAMANHO DA TELA ---
    // Isso garante que a imagem cubra tudo, igual fazia com o Actor
    bg->SetSize(Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)));
    bg->SetColor(Vector4(0.2f, 0.2f, 0.2f, 1.0f)); // Darker background

    // 2. Título
    // auto title = AddText("CONTRA-DICTION", Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT * 0.15f), 1.5f);
    // title->SetTextColor(Vector3(1.0f, 0.6f, 0.0f)); // Orange title

    // 3. Botão Start
    // Posição: Esquerda
    auto startBtn = AddButton("", [this]() {
        mIsFadingOut = true;
    }, Vector2(Game::WINDOW_WIDTH * 0.25f, Game::WINDOW_HEIGHT * 0.60f), 0.5f); 
    
    startBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/start_normal.png",
                          "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/start_hover.png",
                          "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/start_pressed.png");

    // 3.5 Botão Options
    // Posição: Esquerda
     auto optionsBtn = AddButton("", [this]() {
        Close();
        new OptionsScreen(mGame, "../Assets/Fonts/ALS_Micro_Bold.ttf");
    }, Vector2(Game::WINDOW_WIDTH * 0.25f, Game::WINDOW_HEIGHT * 0.70f), 0.5f);
    
    optionsBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Large/options_normal.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Large/options_hover.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Large/options_pressed.png");

    // 4. Botão Quit
    // Posição: Esquerda
    auto exitBtn = AddButton("", [this]() {
        mGame->Quit();
    }, Vector2(Game::WINDOW_WIDTH * 0.25f, Game::WINDOW_HEIGHT * 0.80f), 0.5f);
    
    exitBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_normal.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_hover.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_pressed.png");

    // 5. Spaceman Image
    auto spaceman = AddImage("../Assets/Menus/MainMenuSpaceman.png", Vector2(Game::WINDOW_WIDTH * 0.55f, Game::WINDOW_HEIGHT * 0.75f), 0.2f);

    // Fade Rect
    mFadeRect = AddRect(screenCenter, Vector2(Game::WINDOW_WIDTH, Game::WINDOW_HEIGHT), 1.0f, 0.0f, 200);
    mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
}

MainMenu::~MainMenu()
{
}

void MainMenu::Update(float deltaTime)
{
    UIScreen::Update(deltaTime);

    if (mIsFadingOut)
    {
        mFadeTimer += deltaTime;
        float alpha = std::min(mFadeTimer / FADE_DURATION, 1.0f);
        mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, alpha));

        if (mFadeTimer >= FADE_DURATION)
        {
            Close();
            new StageSelect(mGame, "../Assets/Fonts/ALS_Micro_Bold.ttf");
        }
    }
}

void MainMenu::HandleKeyPress(int key)
{
    if (mIsFadingOut) return;

    // Cheat Code Logic
    if (key == mCheatCode[mCheatCodeIndex])
    {
        mCheatCodeIndex++;
        if (mCheatCodeIndex >= static_cast<int>(mCheatCode.size()))
        {
            // Cheat Activated!
            mGame->SetImmortal(true);
            mGame->GetAudio()->PlaySound("PowerUp.wav"); // Or any other sound
            mCheatCodeIndex = 0;
            SDL_Log("Cheat Code Activated: Immortality ON");
        }
    }
    else
    {
        // Reset if wrong key, but check if it matches the first key
        if (key == mCheatCode[0])
        {
            mCheatCodeIndex = 1;
        }
        else
        {
            mCheatCodeIndex = 0;
        }
    }

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
