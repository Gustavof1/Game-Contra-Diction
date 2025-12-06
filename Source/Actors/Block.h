//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once

#include "Actor.h"

class Block : public Actor
{
public:
    Block(class Game* game, const std::string &texturePath, bool hasMushroom = false);
    Block(class Game* game, class Texture* texture, int srcX, int srcY, int size, bool isCollidable = true);

    void OnUpdate(float deltaTime) override;
    void Bump();
    const std::string& GetTexturePath() const { return mTexturePath; }
    void SetTexturePath(const std::string& path) { mTexturePath = path; }
    void SetFlipData(float rotation, const Vector2& scale);

    int GetOriginalSrcX() const { return mOriginalSrcX; }
    int GetOriginalSrcY() const { return mOriginalSrcY; }
    int GetSize() const { return mSize; }

private:
    std::string mTexturePath;
    Vector2 mOriginalPos;
    bool mIsBumping;
    float mBumpSpeed;
    const float mBumpHeight = 8.0f;
    bool mIsUsed;
    bool mHasMushroom;
    int mFrameIndex;

    class SpriteComponent* mSprite;
    class AABBColliderComponent* mCollider;

    int mOriginalSrcX;
    int mOriginalSrcY;
    int mSize;
};
