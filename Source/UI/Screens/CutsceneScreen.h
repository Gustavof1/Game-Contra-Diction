#pragma once
#include "UIScreen.h"
#include "../../Game.h"

class CutsceneScreen : public UIScreen
{
public:
    CutsceneScreen(class Game* game, GameScene nextScene);
    ~CutsceneScreen();

    void Update(float deltaTime) override;
    void HandleKeyPress(int key) override;

private:
    enum class CutsceneState
    {
        Frame1,
        FadeOut1,
        FadeIn2,
        Frame2,
        FadeOut2,
        FadeIn3,
        Frame3,
        FadeOut3,
        Finished
    };

    CutsceneState mState;
    GameScene mNextScene;
    float mTimer;
    float mFadeTimer;
    const float FADE_DURATION = 1.0f;
    const float DISPLAY_DURATION = 5.0f;

    class UIImage* mImage1;
    class UIImage* mImage2;
    class UIImage* mImage3;
    
    class UIText* mText;
    class UIRect* mFadeRect;
};
