#pragma once
#include "UIScreen.h"

class MainMenu : public UIScreen
{
public:
    MainMenu(class Game* game, const std::string& fontName);
    ~MainMenu();

    void Update(float deltaTime) override;
    void HandleKeyPress(int key) override;

private:
    bool mIsFadingOut;
    float mFadeTimer;
    const float FADE_DURATION = 1.0f;
    class UIRect* mFadeRect;

    std::vector<int> mCheatCode;
    int mCheatCodeIndex;
};
