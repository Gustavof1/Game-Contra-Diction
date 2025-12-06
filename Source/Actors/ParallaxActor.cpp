#include "ParallaxActor.h"
#include "../Game.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Renderer/Texture.h"

ParallaxActor::ParallaxActor(Game* game, const std::string& texturePath, float parallaxFactor, float width, float height, int drawOrder)
    : Actor(game)
    , mParallaxFactor(parallaxFactor)
    , mWidth(width)
    , mHeight(height)
{
    mSpriteComponent = new SpriteComponent(this, drawOrder);
    auto* texture = game->GetRenderer()->GetTexture(texturePath);
    mSpriteComponent->SetTexture(texture);
    
    // Set size to screen size (passed as width/height)
    // We want the background to cover the screen
    SetScale(Vector2(width / texture->GetWidth(), height / texture->GetHeight()));
    
    // Initial position is center of the screen (relative to camera)
    // But since we update position in OnUpdate, we just need to initialize variables
    
    game->AddParallaxActor(this);
}

ParallaxActor::~ParallaxActor()
{
    GetGame()->RemoveParallaxActor(this);
}

void ParallaxActor::OnUpdate(float deltaTime)
{
    // Logic moved to UpdateLayout to handle zoom correctly even when paused
}

void ParallaxActor::UpdateLayout(float zoom)
{
    Vector2 cameraPos = GetGame()->GetCameraPos();
    
    // Viewport size in world units
    float viewportW = GetGame()->WINDOW_WIDTH / zoom;
    float viewportH = GetGame()->WINDOW_HEIGHT / zoom;
    
    // 1. Keep the actor centered on the screen
    Vector2 screenCenter(viewportW / 2.0f, viewportH / 2.0f);
    SetPosition(cameraPos + screenCenter);
    
    // Scale to fill viewport
    if (mSpriteComponent) {
        float texW = static_cast<float>(mSpriteComponent->GetTexWidth());
        float texH = static_cast<float>(mSpriteComponent->GetTexHeight());
        
        if (texW > 0 && texH > 0) {
            SetScale(Vector2(viewportW / texW, viewportH / texH));
        }

        // 2. Scroll the texture to create parallax effect
        float scrollX = cameraPos.x * (1.0f - mParallaxFactor);
        
        // Wrap scrollX to avoid large numbers
        int iScrollX = static_cast<int>(scrollX);
        iScrollX = iScrollX % static_cast<int>(texW);
        mSpriteComponent->SetTextureRect(iScrollX, 0, static_cast<int>(texW), static_cast<int>(texH));
    }
}
