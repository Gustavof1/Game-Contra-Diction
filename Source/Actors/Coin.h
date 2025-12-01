//
// Created by Vinicius Trindade on 23/10/25.
//

#pragma once
#include "Actor.h"

class Coin : public Actor
{
public:
    Coin(class Game* game, bool isAnimatedEffect = false);
    void OnUpdate(float deltaTime) override;
private:
    Vector2 mSpawnStartPosition;
    bool mIsAnimatedEffect;
    bool mIsSpawning;
    bool mIsFalling;
    float mLifeSpan;
};