#include "SpacemanArm.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Game.h"

SpacemanArm::SpacemanArm(Game* game)
    : Actor(game)
    , mAnimator(nullptr)
    , mIsBent(false)
    , mFacingRight(true)
    , mWeaponType(WeaponType::Pistol)
{
    // Load arm spritesheet
    mAnimator = new AnimatorComponent(
        this,
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_arm_spritesheet.png",
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_arm_spritesheet.json",
        0, 0, // Size will be set per animation
        110 
    );
    mAnimator->SetUseFrameAspect(true);

    // Indices based on sorted keys in JSON:
    // 0: alien_bend-0 (Right)
    // 1: alien_bend-1 (Left)
    // 2: alien_straight-0 (Right)
    // 3: alien_straight-1 (Left)
    // 4: pistol_bend-0 (Right)
    // 5: pistol_bend-1 (Left)
    // 6: pistol_straight-0 (Right)
    // 7: pistol_straight-1 (Left)

    mAnimator->AddAnimation("PistolStraightRight", {6});
    mAnimator->AddAnimation("PistolStraightLeft", {7});
    mAnimator->AddAnimation("PistolBentRight", {4});
    mAnimator->AddAnimation("PistolBentLeft", {5});
    
    mAnimator->AddAnimation("AlienStraightRight", {2});
    mAnimator->AddAnimation("AlienStraightLeft", {3});
    mAnimator->AddAnimation("AlienBentRight", {0});
    mAnimator->AddAnimation("AlienBentLeft", {1});

    SetWeaponType(WeaponType::Pistol);
}

void SpacemanArm::SetFacingRight(bool facingRight)
{
    if (mFacingRight != facingRight) {
        mFacingRight = facingRight;
        UpdateAnimation();
    }
}

void SpacemanArm::SetBent(bool bent)
{
    mIsBent = bent;
    UpdateAnimation();
}

void SpacemanArm::SetWeaponType(WeaponType type)
{
    mWeaponType = type;
    UpdateAnimation();
}

void SpacemanArm::UpdateAnimation()
{
    if (!mAnimator) return;

    std::string animName;
    int w = 0, h = 0;
    std::string suffix = mFacingRight ? "Right" : "Left";
    float scale = 0.5f;

    if (mWeaponType == WeaponType::Pistol) {
        if (mIsBent) {
            animName = "PistolBent" + suffix;
            w = 70 * scale; h = 44 * scale;
        } else {
            animName = "PistolStraight" + suffix;
            w = 96 * scale; h = 25 * scale;
        }
    } else {
        if (mIsBent) {
            animName = "AlienBent" + suffix;
            w = 70 * scale; h = 44 * scale;
        } else {
            animName = "AlienStraight" + suffix;
            w = 96 * scale; h = 31 * scale;
        }
    }

    mAnimator->SetAnimation(animName);
    mAnimator->SetSize(w, h);
}

bool SpacemanArm::IsBent() const
{
    return mIsBent;
}

void SpacemanArm::SetVisible(bool visible)
{
    if (mAnimator) {
        mAnimator->SetVisible(visible);
    }
}
