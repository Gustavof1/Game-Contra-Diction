#include "Hazard.h"

#include "../Components/Physics/AABBColliderComponent.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"

Hazard::Hazard(Game* game, int width, int height)
    :Actor(game)
{
    // Make it bigger (30%)
    float w = width * 1.3f;
    float h = height * 1.3f;

    // Criamos o colisor do tamanho exato passado pelo Tiled
    // offset (0,0) porque vamos posicionar o Actor no centro do retângulo
    AABBColliderComponent* col = new AABBColliderComponent(
        this,
        0, 0,           // Offset X, Y (Centralizado no Actor)
        static_cast<int>(w), static_cast<int>(h),  // Tamanho dinâmico
        ColliderLayer::Hazard,
        true,           // isTrigger = true (Player atravessa)
        true,           // isStatic = true
        10
    );

    // Highlight
    auto* sc = new SpriteComponent(this, 150);
    
    // Heuristic: Wide hazards are spikes, tall/square hazards are poisonous mushrooms
    if (width > height) {
        sc->SetTexture(game->GetRenderer()->GetTexture("../Assets/Sprites/Cenario/spikes.png"));
    } else {
        sc->SetTexture(game->GetRenderer()->GetTexture("../Assets/Sprites/Cenario/mushroompoison.png"));
    }

    sc->SetColor(Vector3(1.0f, 0.5f, 0.5f));
    // sc->SetAlpha(0.4f);
    
    // Calculate scale for sprite (assuming 32x32 texture base, but we should check texture size if possible)
    // Since we don't know texture size here easily without querying texture, 
    // and GetTexture returns a Texture*, we can use it.
    
    auto* tex = sc->GetTexture();
    if (tex) {
        float scaleX = w / static_cast<float>(tex->GetWidth());
        float scaleY = h / static_cast<float>(tex->GetHeight());
        SetScale(Vector2(scaleX, scaleY));
    } else {
        // Fallback if texture fails
        float scaleX = w / static_cast<float>(Game::TILE_SIZE);
        float scaleY = h / static_cast<float>(Game::TILE_SIZE);
        SetScale(Vector2(scaleX, scaleY));
    }
}
