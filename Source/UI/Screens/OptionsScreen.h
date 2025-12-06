#pragma once
#include "UIScreen.h"

class OptionsScreen : public UIScreen
{
public:
    OptionsScreen(class Game* game, const std::string& fontName);
    ~OptionsScreen();

    void Update(float deltaTime) override;
    void HandleMouseClick(const Vector2& mousePos) override;
    void HandleMouseMove(const Vector2& mousePos) override;

private:
    void UpdateKnobPositions();

    class UIRect* mSFXBar;
    class UIRect* mSFXKnob;
    class UIRect* mMusicBar;
    class UIRect* mMusicKnob;

    float mSFXVolumeVal;
    float mMusicVolumeVal;
    
    // Cooldown for playing SFX sample
    float mSFXSampleCooldown;
};
