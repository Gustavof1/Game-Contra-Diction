#include "Vegetation.h"
#include "../../../Source/Components/Drawing/SpriteComponent.h"

Vegetation::Vegetation(Game* game)
    :Actor(game)
{
    // O construtor está vazio pois configuramos o sprite depois
    // Mas se quiser, pode criar o componente aqui e guardar num ponteiro membro
}

void Vegetation::SetSpriteData(Texture* texture, int srcX, int srcY, int w, int h)
{
    // Cria o componente visual atrelado a este Actor
    // (O Actor gerencia a memória do componente, geralmente)
    SpriteComponent* sc = new SpriteComponent(this, 150); // DrawOrder 150 (exemplo: desenha na frente do chão)
    
    sc->SetTexture(texture);
    
    // A mágica acontece aqui: usamos sua função existente para definir o recorte
    sc->SetTextureRect(srcX, srcY, w, h);
}