#pragma once
#include "Actor.h"

enum class WeaponType
{
    Pistol,
    AlienGun
};

class SpacemanArm : public Actor
{
public:
    SpacemanArm(class Game* game);
    void SetBent(bool bent);
    bool IsBent() const;
    
    void SetWeaponType(WeaponType type);
    WeaponType GetWeaponType() const { return mWeaponType; }

    void SetFacingRight(bool facingRight);
    bool IsFacingRight() const { return mFacingRight; }

    void SetVisible(bool visible);

private:
    void UpdateAnimation();

    class AnimatorComponent* mAnimator;
    bool mIsBent;
    bool mFacingRight;
    WeaponType mWeaponType;
};
