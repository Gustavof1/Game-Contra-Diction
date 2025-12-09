#pragma once
#include "DrawComponent.h"
#include "../../Renderer/Texture.h"
#include "../../Renderer/Font.h"

class TutorialDrawComponent : public DrawComponent
{
public:
    TutorialDrawComponent(class Actor* owner);
    ~TutorialDrawComponent();

    void Draw(class Renderer* renderer) override;
    void Update(float deltaTime) override;
    void ProcessInput(const uint8_t* keyState) override;

private:
    Texture* mTexW;
    Texture* mTexA;
    Texture* mTexD;
    Texture* mTexS;
    Texture* mTexSpace;
    Texture* mTexUp;
    Texture* mTexDown;
    Texture* mTexLeft;
    Texture* mTexRight;
    Texture* mTexCtrl;
    Texture* mTexMouseLeft;
    Texture* mTexTab;
    Texture* mTexShift;

    Texture* mTexJump;
    Texture* mTexMove;
    Texture* mTexCrouch;
    Texture* mTexShoot;
    Texture* mTexDance;
    Texture* mTexInventory;
    Texture* mTexInventoryRed;
    Texture* mTexRun;
    Texture* mTexDoubleJump;

    Font* mFont;

    bool mShowJump;
    bool mShowDoubleJump;
    bool mShowMove;
    bool mShowCrouch;
    bool mShowShoot;
    bool mShowDance;
    bool mShowInventory;
    bool mShowRun;
    
    bool mJumpKeyReleased;

    bool mPulseInventory;
    float mPulseTimer;
    float mPulseDurationTimer;
};
