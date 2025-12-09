#include "GameOver.h"
#include "../../Game.h"
#include <SDL.h>
#include "../../Json.h"
#include <fstream>
#include <algorithm>
#include "../UIImage.h"
#include "../UIText.h"
#include "../../Renderer/Texture.h"

GameOver::GameOver(class Game* game, const std::string& fontName, GameScene level)
        :UIScreen(game, fontName)
        ,mlevel(level)
        ,mFadeState(FadeState::FadingIn)
        ,mFadeTimer(1.0f)
        ,mFadeRect(nullptr)
        ,mKillerImage(nullptr)
        ,mKillerTexture(nullptr)
        ,mAnimTimer(0.0f)
        ,mCurrentFrameIndex(0)
{

    Vector2 screenCenter(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT/2.0f);
    float leftColumnX = Game::WINDOW_WIDTH * 0.25f;
    float rightColumnX = Game::WINDOW_WIDTH * 0.75f;
    
    auto bg = AddImage("../Assets/Menus/GameOver.png", screenCenter, 1.0f, 0.0f, 50);
    
    bg->SetSize(Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)));

    // Dark overlay to make foreground elements pop
    auto darkOverlay = AddRect(screenCenter, Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)), 1.0f, 0.0f, 100);
    darkOverlay->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.7f));

    // Killer Info (Left Side)
    const auto& info = game->GetGameOverInfo();
    if (!info.killerName.empty())
    {
        auto text = AddText("Voce foi derrotado por:", Vector2(leftColumnX, screenCenter.y - 100.0f), 0.8f);
        text->SetTextColor(Vector3(1.0f, 0.0f, 0.0f));

        if (info.isBlock)
        {
            float scale = 2.0f;
            if (info.killerName == "Poison Mushroom") {
                scale = 1.0f;
            }
            mKillerImage = AddImage(info.killerSpritePath, Vector2(leftColumnX, screenCenter.y), scale, 0.0f, 150);
            if (info.useSrcRect)
            {
                mKillerImage->SetTextureRect(info.srcX, info.srcY, info.srcW, info.srcH);
            }
        }
        else if (info.isEnemy)
        {
            std::ifstream file(info.killerJsonPath);
            if (file.is_open())
            {
                nlohmann::json json;
                file >> json;
                
                std::vector<int> frameIndices;
                if (json.contains("meta") && json["meta"].contains("frameTags")) {
                    for (const auto& tag : json["meta"]["frameTags"]) {
                        if (tag["name"] == "idle") {
                            int from = tag["from"];
                            int to = tag["to"];
                            for (int i = from; i <= to; ++i) {
                                frameIndices.push_back(i);
                            }
                            break;
                        }
                    }
                }
                
                if (frameIndices.empty()) {
                    frameIndices.push_back(0);
                }

                if (json.contains("frames")) {
                    if (json["frames"].is_array()) {
                        for (int idx : frameIndices) {
                            if (idx >= 0 && idx < json["frames"].size()) {
                                auto& frame = json["frames"][idx]["frame"];
                                SDL_Rect r;
                                r.x = frame["x"];
                                r.y = frame["y"];
                                r.w = frame["w"];
                                r.h = frame["h"];
                                mIdleRects.push_back(r);
                            }
                        }
                    } else if (json["frames"].is_object()) {
                        std::vector<std::string> keys;
                        for (auto it = json["frames"].begin(); it != json["frames"].end(); ++it) {
                            keys.push_back(it.key());
                        }
                        std::sort(keys.begin(), keys.end());

                        for (int idx : frameIndices) {
                            if (idx >= 0 && idx < keys.size()) {
                                auto& frame = json["frames"][keys[idx]]["frame"];
                                SDL_Rect r;
                                r.x = frame["x"];
                                r.y = frame["y"];
                                r.w = frame["w"];
                                r.h = frame["h"];
                                mIdleRects.push_back(r);
                            }
                        }
                    }
                }
            }
            
            if (!mIdleRects.empty()) {
                mKillerImage = AddImage(info.killerSpritePath, Vector2(leftColumnX, screenCenter.y), 2.0f, 0.0f, 150);
                mKillerImage->SetTextureRect(mIdleRects[0].x, mIdleRects[0].y, mIdleRects[0].w, mIdleRects[0].h);
            }
        }
    }

    mFadeRect = AddRect(screenCenter, Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)), 1.0f, 0.0f, 200);
    mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    // Buttons (Right Side)
    auto restartBtn = AddButton("", [this]() {
        if (mFadeState != FadeState::None) return;
        mFadeState = FadeState::FadingOut;
        mFadeTimer = 0.0f;
        mNextAction = [this]() {
            Close();
            mGame->SetScene(mlevel);
        };
    }, Vector2(rightColumnX, screenCenter.y - 50.0f), 0.5f); 
    
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
    }, Vector2(rightColumnX, screenCenter.y + 50.0f), 0.5f);
    
    menuBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/back_normal.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/back_hover.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/back_pressed.png");

    auto exitBtn = AddButton("", [this]() {
        if (mFadeState != FadeState::None) return;
        mGame->Quit();
    }, Vector2(rightColumnX, screenCenter.y + 150.0f), 0.5f);
    
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

    if (!mIdleRects.empty() && mKillerImage)
    {
        mAnimTimer += deltaTime;
        if (mAnimTimer >= 0.1f)
        {
            mAnimTimer -= 0.1f;
            mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mIdleRects.size();
            const auto& r = mIdleRects[mCurrentFrameIndex];
            mKillerImage->SetTextureRect(r.x, r.y, r.w, r.h);
        }
    }

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