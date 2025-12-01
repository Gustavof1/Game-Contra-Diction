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
}

void ParallaxActor::OnUpdate(float deltaTime)
{
    Vector2 cameraPos = GetGame()->GetCameraPos();
    
    // 1. Keep the actor centered on the screen
    Vector2 screenCenter(GetGame()->WINDOW_WIDTH / 2.0f, GetGame()->WINDOW_HEIGHT / 2.0f);
    SetPosition(cameraPos + screenCenter);

    // 2. Scroll the texture to create parallax effect
    // We use SetTextureRect to shift the UVs
    // Offset = CameraX * Factor
    // We need to convert pixels to texture coordinates?
    // SpriteComponent::SetTextureRect takes pixels (x, y, w, h)
    
    if (mSpriteComponent) {
        int texWidth = mSpriteComponent->GetTexWidth();
        int texHeight = mSpriteComponent->GetTexHeight();
        
        // Calculate offset in pixels
        // If Factor is 1.0, it moves with camera (static on screen) -> Offset 0?
        // No, if Factor is 1.0 (distant), it should stay static relative to world?
        // Wait.
        // If I want it to look like it's at infinity (Factor 1.0), it should NOT scroll.
        // If I want it to look close (Factor 0.0), it should scroll at same speed as world.
        
        // Let's re-evaluate Factor.
        // Previous logic: Pos = Init + Cam * Factor.
        // If Factor=1, Pos moves with Cam. Relative pos is constant. Static on screen.
        // If Factor=0, Pos is constant. Relative pos moves opposite to Cam. Moves with world.
        
        // Here, the Actor is STATIC ON SCREEN (Pos = Cam + Center).
        // So by default, it moves with Camera (Factor 1 behavior).
        // To make it move with world (Factor 0 behavior), we need to SCROLL the texture.
        // If Camera moves +100.
        // Factor 1 (Infinity): Texture should NOT scroll.
        // Factor 0 (Close): Texture should scroll +100 pixels (to look like it stayed behind).
        
        // So Scroll = Camera * (1.0 - Factor).
        
        float scrollX = cameraPos.x * (1.0f - mParallaxFactor);
        
        // Wrap scrollX to avoid large numbers
        if (texWidth > 0) {
            int iScrollX = static_cast<int>(scrollX);
            iScrollX = iScrollX % texWidth;
            mSpriteComponent->SetTextureRect(iScrollX, 0, texWidth, texHeight);
        }
    }
}
