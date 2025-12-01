//
// Created by Lucas N. Ferreira on 30/09/23.
//

#pragma once

#include "Actor.h"

enum class SpawnerType {
    Policeman,
    AlienKid,
    AlienMan,
    AlienWoman,
    RobotTurret,
    RobotFlyer
};

class Spawner : public Actor
{
public:
    explicit Spawner(Game* game, float spawnDistance, SpawnerType type = SpawnerType::Policeman);

    void OnUpdate(float deltaTime) override;
private:
    float mSpawnDistance;
    SpawnerType mType;
};
