#pragma once
#include "Drawing/DrawComponent.h"
#include "../Math.h"

class HealthBarComponent : public DrawComponent
{
public:
    HealthBarComponent(class Actor* owner, int maxHP, int* currentHP);
    void Draw(class Renderer* renderer) override;

private:
    int mMaxHP;
    int* mCurrentHP;
};
