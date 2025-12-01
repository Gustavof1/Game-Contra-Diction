//
// Created by Lucas N. Ferreira on 30/09/23.
//

#include "Spawner.h"
#include "../Game.h"
#include "Spaceman.h"
#include "Policeman.h"
#include "AlienKid.h"
#include "AlienMan.h"
#include "AlienWoman.h"
#include "RobotFlyer.h"
#include "RobotTurret.h"

Spawner::Spawner(Game* game, float spawnDistance, SpawnerType type)
        :Actor(game)
        ,mSpawnDistance(spawnDistance)
        ,mType(type)
{

}

void Spawner::OnUpdate(float deltaTime)
{
    // Check distance to player to spawn enemy
    const Spaceman* player = GetGame()->GetPlayer();
    if (player) {
        float dist = fabs(player->GetPosition().x - mPosition.x);

        if (dist < mSpawnDistance) {
            Actor* spawnedActor = nullptr;
            switch (mType) {
                case SpawnerType::Policeman:
                    spawnedActor = new Policeman(GetGame(), 100.0f);
                    break;
                case SpawnerType::AlienKid:
                    spawnedActor = new AlienKid(GetGame());
                    break;
                case SpawnerType::AlienMan:
                    spawnedActor = new AlienMan(GetGame());
                    break;
                case SpawnerType::AlienWoman:
                    spawnedActor = new AlienWoman(GetGame());
                    break;
                case SpawnerType::RobotTurret:
                    spawnedActor = new RobotTurret(GetGame());
                    break;
                case SpawnerType::RobotFlyer:
                {
                    int count = 4 + (rand() % 2);
                    for(int i=0; i<count; ++i) {
                        auto* flyer = new RobotFlyer(GetGame());
                        // Random offset around spawner position
                        float offX = (rand() % 60) - 30.0f;
                        float offY = (rand() % 60) - 30.0f;
                        Vector2 spawnPos = mPosition + Vector2(offX, offY);
                        
                        flyer->SetPosition(spawnPos);
                        flyer->SetArrival(mPosition); // They all arrive at the spawner's location
                    }
                    spawnedActor = nullptr; // Handled above
                    break;
                }
            }

            if (spawnedActor) {
                spawnedActor->SetPosition(mPosition);

                if (mType == SpawnerType::RobotFlyer) {
                    // This block is now unreachable for RobotFlyer because spawnedActor is null
                    // But kept for safety if logic changes
                    auto* flyer = dynamic_cast<RobotFlyer*>(spawnedActor);
                    if (flyer) {
                        flyer->SetArrival(mPosition);
                    }
                }
            }
            mState = ActorState::Destroy;
        }
    }
}