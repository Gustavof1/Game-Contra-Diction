#pragma once
#include "UIScreen.h"
#include "../../Game.h"
#include <functional>

class GameOver : public UIScreen
{
public:
    GameOver(class Game* game, const std::string& fontName, GameScene level);
    ~GameOver();

    void HandleKeyPress(int key) override;
    void Update(float deltaTime) override;
    
private:
    GameScene mlevel;
    
    enum class FadeState {
        None,
        FadingIn,
        FadingOut
    };

    FadeState mFadeState;
    float mFadeTimer;
    class UIRect* mFadeRect;
    std::function<void()> mNextAction;

    // Killer info
    class UIImage* mKillerImage;
    class Texture* mKillerTexture;
    std::vector<SDL_Rect> mIdleRects;
    float mAnimTimer;
    int mCurrentFrameIndex;
};