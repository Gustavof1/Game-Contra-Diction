#include "Hazard.h"

#include "../Components/Physics/AABBColliderComponent.h"

Hazard::Hazard(Game* game, int width, int height)
    :Actor(game)
{
    // Criamos o colisor do tamanho exato passado pelo Tiled
    // offset (0,0) porque vamos posicionar o Actor no centro do retângulo
    AABBColliderComponent* col = new AABBColliderComponent(
        this,
        0, 0,           // Offset X, Y (Centralizado no Actor)
        width, height,  // Tamanho dinâmico
        ColliderLayer::Hazard,
        true,           // isTrigger = true (Player atravessa)
        true,           // isStatic = true
        10
    );
}
