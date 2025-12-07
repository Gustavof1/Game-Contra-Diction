#include "GasCloud.h"
#include "../Game.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Random.h"
#include "PolicemanBullet.h"
#include "EnemyLaser.h"
#include "PlayerBullet.h"
#include <SDL_image.h>

static Texture* sGasTexture = nullptr;

GasCloud::GasCloud(Game* game, const Vector2& direction)
    : Actor(game)
    , mDirection(direction)
    , mLifeTime(1.0f)
    , mSpeed(400.0f)
{
    mSprite = new SpriteComponent(this, 150);

    if (!sGasTexture)
    {
        // Load textures
        SDL_Surface* spriteSurf = IMG_Load("../Assets/Sprites/gas_sprite.png");
        SDL_Surface* alphaSurf = IMG_Load("../Assets/Sprites/gas_alpha.png");

        if (spriteSurf && alphaSurf)
        {
            // Convert sprite to RGBA32
            SDL_Surface* finalSurf = SDL_ConvertSurfaceFormat(spriteSurf, SDL_PIXELFORMAT_RGBA32, 0);
            
            // Convert alpha to RGBA32
            SDL_Surface* tempAlpha = SDL_ConvertSurfaceFormat(alphaSurf, SDL_PIXELFORMAT_RGBA32, 0);
            
            SDL_Surface* alphaSurfRGBA = nullptr;

            if (finalSurf && tempAlpha)
            {
                // Handle dimension mismatch by scaling alpha to match sprite
                if (finalSurf->w != tempAlpha->w || finalSurf->h != tempAlpha->h)
                {
                    alphaSurfRGBA = SDL_CreateRGBSurfaceWithFormat(0, finalSurf->w, finalSurf->h, 32, SDL_PIXELFORMAT_RGBA32);
                    if (alphaSurfRGBA)
                    {
                        SDL_SetSurfaceBlendMode(tempAlpha, SDL_BLENDMODE_NONE);
                        SDL_BlitScaled(tempAlpha, nullptr, alphaSurfRGBA, nullptr);
                    }
                    SDL_FreeSurface(tempAlpha);
                }
                else
                {
                    alphaSurfRGBA = tempAlpha;
                }

                if (alphaSurfRGBA)
                {
                    SDL_LockSurface(finalSurf);
                    SDL_LockSurface(alphaSurfRGBA);

                    int width = finalSurf->w;
                    int height = finalSurf->h;

                    for (int y = 0; y < height; ++y)
                    {
                        Uint32* pixels = (Uint32*)((Uint8*)finalSurf->pixels + y * finalSurf->pitch);
                        Uint32* alphaPixels = (Uint32*)((Uint8*)alphaSurfRGBA->pixels + y * alphaSurfRGBA->pitch);

                        for (int x = 0; x < width; ++x)
                        {
                            Uint8 r, g, b, a;
                            SDL_GetRGBA(pixels[x], finalSurf->format, &r, &g, &b, &a);
                            
                            Uint8 ar, ag, ab, aa;
                            SDL_GetRGBA(alphaPixels[x], alphaSurfRGBA->format, &ar, &ag, &ab, &aa);

                            // Use red channel of alpha texture as alpha (White = 255 = Opaque)
                            a = ar; 

                            pixels[x] = SDL_MapRGBA(finalSurf->format, r, g, b, a);
                        }
                    }

                    SDL_UnlockSurface(alphaSurfRGBA);
                    SDL_UnlockSurface(finalSurf);

                    sGasTexture = new Texture();
                    sGasTexture->CreateFromSurface(finalSurf);
                    
                    SDL_FreeSurface(alphaSurfRGBA);
                }
            }

            if (finalSurf) SDL_FreeSurface(finalSurf);
        }
        
        if (spriteSurf) SDL_FreeSurface(spriteSurf);
        if (alphaSurf) SDL_FreeSurface(alphaSurf);
    }

    if (sGasTexture)
    {
        mSprite->SetTexture(sGasTexture);
    }

    // Set 30% opacity
    mSprite->SetAlpha(0.3f);
    // Set Green Color
    mSprite->SetColor(Vector3(0.0f, 1.0f, 0.0f));

    // Random rotation
    SetRotation(Random::GetFloatRange(0.0f, Math::Pi * 2.0f));
    
    // Random scale variation (smaller particles)
    float scale = Random::GetFloatRange(0.1f, 0.3f);
    SetScale(Vector2(scale, scale));

    // RigidBody for movement and collision detection
    auto* rb = new RigidBodyComponent(this, 1.0f, 0.0f, false);
    rb->SetVelocity(mDirection * mSpeed);

    // Collider for precise collision (Trigger to pass through enemies)
    mCollider = new AABBColliderComponent(this, 0, 0, 20, 20, ColliderLayer::PlayerProjectile, true);
}

void GasCloud::OnUpdate(float deltaTime)
{
    mLifeTime -= deltaTime;
    if (mLifeTime <= 0.0f)
    {
        SetState(ActorState::Destroy);
        return;
    }

    // Fade out based on lifetime
    // Start fading when lifetime is below 0.5s
    if (mLifeTime < 0.5f)
    {
        float alpha = (mLifeTime / 0.5f) * 0.3f; // 0.3f is the max alpha
        mSprite->SetAlpha(alpha);
    }
    
    // Rotate slowly
    SetRotation(GetRotation() + deltaTime);
}

void GasCloud::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        // Ignore bullets
        if (dynamic_cast<PolicemanBullet*>(other->GetOwner()) || 
            dynamic_cast<EnemyLaser*>(other->GetOwner()))
        {
            return;
        }

        Actor* enemy = other->GetOwner();
        
        // Check if we already hit this enemy
        bool alreadyHit = false;
        for (auto* hit : mHitActors)
        {
            if (hit == enemy)
            {
                alreadyHit = true;
                break;
            }
        }

        if (!alreadyHit)
        {
            enemy->HitByGas();
            mHitActors.push_back(enemy);
        }
    }
    else if (other->GetLayer() == ColliderLayer::Blocks)
    {
        SetState(ActorState::Destroy);
    }
}

void GasCloud::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    OnHorizontalCollision(minOverlap, other);
}
