#pragma once
#include "Actor.h"
#include <string>

class ParallaxActor : public Actor
{
public:
    ParallaxActor(class Game* game, const std::string& texturePath, float parallaxFactor, float width, float height, int drawOrder);
    ~ParallaxActor();
    
    void OnUpdate(float deltaTime) override;
    void UpdateLayout(float zoom);
private:
    float mParallaxFactor;
    float mWidth;
    float mHeight;
    class SpriteComponent* mSpriteComponent;
};
