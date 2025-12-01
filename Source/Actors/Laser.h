#pragma once
#include "Actor.h"

class Laser : public Actor
{
public:
    Laser(class Game* game);
    void UpdateBeam(const Vector2& start, const Vector2& direction);
    void SetActive(bool active);

private:
    class RectComponent* mRect;
    class AABBColliderComponent* mCollider;
};
