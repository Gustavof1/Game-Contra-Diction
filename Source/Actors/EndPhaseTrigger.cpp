#include "EndPhaseTrigger.h"
#include "../Game.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "LevelEndActor.h"
#include "Spaceman.h" // Para pausar o player

EndPhaseTrigger::EndPhaseTrigger(Game* game, float w, float h)
    :Actor(game)
{
    auto* box = new AABBColliderComponent(this, 0, 0, w, h, ColliderLayer::Blocks, true, true);

    box->SetCollisionCallback([this](AABBColliderComponent* other) {
        if (other->GetLayer() == ColliderLayer::Player) {
            // EVITAR DUPLA ATIVAÇÃO
            if (GetState() == ActorState::Destroy) return;

            // 1. Criar a animação da nave

            std::string texPath;

            switch (GetGame()->GetCurrentScene()) {
                case GameScene::Level1:
                    texPath = "../Assets/Sprites/Jeep.png";
                    break;
                case GameScene::Level2:
                    texPath = "../Assets/Sprites/ParteNave2.png";
                    break;
                case GameScene::Level3:
                    texPath = "../Assets/Sprites/ParteNave3.png";
                    break;
                default:
                    texPath = "../Assets/Sprites/ParteNave1.png";
                    break;
            }

            auto* endSeq = new LevelEndActor(GetGame(), texPath); // Use a imagem certa
            endSeq->SetPosition(GetPosition());

            // 2. Pausar ou Esconder o Player
            auto* player = GetGame()->GetPlayer();
            if (player) {
                player->SetState(ActorState::Paused); // Para de mover
                // Opcional: player->SetVisible(false); // Se ele "entrou" na nave
            }

            // 3. Destruir este gatilho para não ativar de novo
            SetState(ActorState::Destroy);
        }
    });
}