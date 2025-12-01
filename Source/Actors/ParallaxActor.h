#pragma once
#include "Actor.h"
#include <string>

class ParallaxActor : public Actor
{
public:
    ParallaxActor(class Game* game, const std::string& texturePath, float parallaxFactor, float width, float height, int drawOrder);
    void OnUpdate(float deltaTime) override;
private:
    float mParallaxFactor;
    float mWidth;
    float mHeight;
    class SpriteComponent* mSpriteComponent;
};
