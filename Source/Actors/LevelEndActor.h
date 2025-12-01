#pragma once
#include "Actor.h"
#include <string>

class LevelEndActor : public Actor
{
public:
    LevelEndActor(class Game* game, const std::string& shipTexture);
    
    void OnUpdate(float deltaTime) override;
    float GetTimer() const { return mTimer; }

private:
    class SpriteComponent* mSprite;
    float mTimer;
    bool mIsAnimating;
    
    // Para centralizar
    Vector2 mStartPos;
    Vector2 mEndPos; // Centro da tela (em coordenadas de mundo + camera)
};