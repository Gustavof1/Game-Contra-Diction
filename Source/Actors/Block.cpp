//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Block.h"
#include "Mushroom.h"
#include "Coin.h"
#include "../Game.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

Block::Block(Game* game, const std::string &texturePath, bool hasMushroom)
        :Actor(game)
        ,mTexturePath(texturePath)
        ,mIsBumping(false)
        ,mBumpSpeed(0.0f)
        ,mIsUsed(false)
        ,mHasMushroom(hasMushroom)
        ,mFrameIndex(0)
        ,mOriginalSrcX(0)
        ,mOriginalSrcY(0)
        ,mSize(Game::TILE_SIZE)
{
    // Extract filename from texture path
    std::string filename = texturePath;
    size_t pos = filename.find_last_of("/\\");
    if (pos != std::string::npos) {
        filename = filename.substr(pos + 1);
    }

    // Map texture filename to frame index in the sprite sheet
    int frameIndex = 0;
    if (filename == "BlockA.png") frameIndex = 0;
    else if (filename == "BlockB.png") frameIndex = 1;
    else if (filename == "BlockC.png") frameIndex = 2;
    else if (filename == "BlockD.png") frameIndex = 3;
    else if (filename == "BlockE.png") frameIndex = 4;
    else if (filename == "BlockF.png") frameIndex = 5;
    else if (filename == "BlockG.png") frameIndex = 6;
    else if (filename == "BlockH.png") frameIndex = 7;
    else if (filename == "BlockI.png") frameIndex = 8;
    else frameIndex = 0; // fallback

    // Store frame index
    mFrameIndex = frameIndex;

    // Create animator component
    auto* ac = new AnimatorComponent(this, "../Assets/Sprites/Blocks/Assets.png", "../Assets/Sprites/Blocks/Blocks.json", Game::TILE_SIZE, Game::TILE_SIZE, 99);
    
    ac->AddAnimation("idle", { frameIndex });
    ac->AddAnimation("used", { 4 });
    ac->SetAnimation("idle");
    ac->SetAnimFPS(1.0f); 

    // Create AABB collider
    new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE, Game::TILE_SIZE, ColliderLayer::Blocks, false, true);
}

Block::Block(Game* game, Texture* texture, int srcX, int srcY, int size, bool isCollidable)
    :Actor(game)
    ,mIsBumping(false)
    ,mBumpSpeed(0.0f)
    ,mIsUsed(false)
    ,mHasMushroom(false)
    ,mFrameIndex(0)
    ,mOriginalSrcX(srcX)
    ,mOriginalSrcY(srcY)
    ,mSize(size)
{
    // --- 1. VISUAL ---
    // Cria o componente de sprite.
    // DrawOrder 100 garante que fique na frente do Background (se o BG for < 100)
    mSprite = new SpriteComponent(this, 100);

    // Define a textura gigante (Tileset)
    mSprite->SetTexture(texture);

    // Define QUAL pedaço dessa textura é este bloco específico
    // (O srcX e srcY que calculamos no Game.cpp)
    mSprite->SetTextureRect(srcX, srcY, size, size);

    // Store original position
    mOriginalPos = GetPosition();

    // --- 2. FÍSICA ---
    // Cria o colisor.
    // ColliderLayer::Blocks é crucial para o Player saber que isso é chão.
    if (isCollidable) {
        new AABBColliderComponent(this, 0, 0, size, size, ColliderLayer::Blocks);
    }
}

void Block::OnUpdate(float deltaTime)
{
    if (mIsBumping) { // Handle bumping motion
        mPosition.y += mBumpSpeed * deltaTime;

        if (mBumpSpeed < 0 && mPosition.y <= mOriginalPos.y - mBumpHeight) {
            mPosition.y = mOriginalPos.y - mBumpHeight;
            mBumpSpeed *= -1.0f;
        }
        else if (mBumpSpeed > 0 && mPosition.y >= mOriginalPos.y) { // Return to original position
            mPosition = mOriginalPos;
            mIsBumping = false;
        }
    }
}

void Block::Bump()
{
    
    if (mIsBumping || mIsUsed) {
        return;
    }

    // Start bumping motion
    mIsBumping = true;
    mOriginalPos = mPosition;
    mBumpSpeed = -200.0f;

    // Check for any enemy on top of the block and kill it
    auto* blockCollider = GetComponent<AABBColliderComponent>();
    if (blockCollider) {
        Vector2 blockTopLeft = blockCollider->GetMin();

        for (auto collider : GetGame()->GetColliders()) {
            if (collider->GetLayer() == ColliderLayer::Enemy) {
                Vector2 enemyBottomLeft = Vector2(collider->GetMin().x, collider->GetMax().y);

                // Check for horizontal overlap
                bool horizontalOverlap = blockCollider->GetMin().x < collider->GetMax().x &&
                                         blockCollider->GetMax().x > collider->GetMin().x;

                // Check if the enemy's bottom is touching the block's top (with a small tolerance)
                bool verticalTouch = fabs(enemyBottomLeft.y - blockTopLeft.y) < 5.0f;

                if (horizontalOverlap && verticalTouch) {
                    collider->GetOwner()->Kill();
                }
            }
        }
    }

    // Generate item or coin based on block type
    if (mFrameIndex == 2) {
        mIsUsed = true;

        // Change animation to "used"
        auto* ac = GetComponent<AnimatorComponent>();
        if (ac) {
            ac->SetAnimation("used");
        }

        if (mHasMushroom) {
            // Generate a mushroom
            auto* mushroom = new Mushroom(GetGame());
            mushroom->SetPosition(Vector2(mPosition.x, mPosition.y));
        } else {
            // If no mushroom, generate a coin
            GetGame()->AddCoin();
            auto* coin = new Coin(GetGame(), true);
            coin->SetPosition(Vector2(mPosition.x, mPosition.y));
        }
    }
}

void Block::SetFlipData(float rotation, const Vector2& scale) {
    SetRotation(rotation);
    if (mSprite) {
        mSprite->SetFlipScale(scale);
    }
}