#pragma once
#include "Actor.h"

class VineWarning : public Actor
{
public:
    VineWarning(class Game* game);
    void OnUpdate(float deltaTime) override;
private:
    float mLifeTime;
};
