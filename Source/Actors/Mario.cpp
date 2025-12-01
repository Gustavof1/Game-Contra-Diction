//
// Created by Lucas N. Ferreira on 03/08/23.
#include "Mario.h"

// Placeholder implementation kept to satisfy legacy includes; real player lives in Spaceman.
            Kill();
        }
    }

    // Update animations based on state
    ManageAnimations();
}

void Mario::ManageAnimations()
{
    auto* ac = GetComponent<AnimatorComponent>();
    if (!ac) {
        return;
    }

    // Priority animations: stomp > turn > normal states
    if (mStompTimer > 0.0f) {
        ac->SetAnimation("stomp");
        return;
    }
    if (mTurnTimer > 0.0f) {
        ac->SetAnimation("turn");
        return;
    }
// Legacy placeholder. Spaceman is now the active player implementation.
        else {
            // Fallback if any component is not found
            Kill();
        }
    }

    // If colliding with a collectable, check if we should power up
    if (other->GetLayer() == ColliderLayer::Collectable) {
        if (dynamic_cast<Mushroom*>(other->GetOwner())) {
            PowerUp();
            other->GetOwner()->Kill();
        }
        else if (dynamic_cast<Coin*>(other->GetOwner())) {
            GetGame()->AddCoin();
            other->GetOwner()->SetState(ActorState::Destroy);
        }
    }
}

void Mario::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    auto* rbc = GetComponent<RigidBodyComponent>();
    if (!rbc) {
        return;
    }

    // Check for ground collision
    if (minOverlap > 0.0f) {
        mIsOnGround = true;

        if (other->GetLayer() == ColliderLayer::Enemy) {
            Goomba* goomba = dynamic_cast<Goomba*>(other->GetOwner());
            if (goomba) {
                goomba->setStomped(true);
            }
            other->GetOwner()->Kill();
            
            Vector2 vel = rbc->GetVelocity();
            vel.y = mJumpSpeed * 0.6f; 
            rbc->SetVelocity(vel);
            SetOffGround();

            // show stomp animation briefly
            auto* ac = GetComponent<AnimatorComponent>();
            if (ac) {
                ac->SetAnimation("stomp");
                mStompTimer = 0.40f;
            }
        }
    }
    // Check for hitting blocks/enemies from below
    else if (minOverlap < 0.0f) {
        if (other->GetLayer() == ColliderLayer::Blocks) {
            Block* block = dynamic_cast<Block*>(other->GetOwner());
            if (block) {
                std::string tex = block->GetTexturePath();
                
                // If it's a brick block ("BlockB.png")
                if (tex == "BlockB.png") {
                    block->Bump();
                    if (mIsSuper) {
                        // Super Mario breaks the block
                        block->SetState(ActorState::Destroy);
                    }
                }
                // If it's a question block ("BlockC.png"), it always bumps
                else if (tex == "BlockC.png") {
                    block->Bump();
                }
            }
        }
    }
    else if (other->GetLayer() == ColliderLayer::Enemy) {
        Kill();
    }

    // If colliding with a collectable
    if (other->GetLayer() == ColliderLayer::Collectable) {
        if (dynamic_cast<Mushroom*>(other->GetOwner())) {
            PowerUp();
            other->GetOwner()->Kill();
        }
        else if (dynamic_cast<Coin*>(other->GetOwner())) {
            GetGame()->AddCoin();
            other->GetOwner()->SetState(ActorState::Destroy);
        }
    }
}

void Mario::PowerUp()
{
    if (mIsSuper) return;
    mIsSuper = true;

    // Reset vertical velocity
    auto* rbc = GetComponent<RigidBodyComponent>();
    if (rbc) {
        Vector2 vel = rbc->GetVelocity();
        vel.y = -150.0f;
        rbc->SetVelocity(vel);
    }

    // Update sprite sheet for super Mario
    auto* ac = GetComponent<AnimatorComponent>();
    if (ac) {
        
        ac->SetSpriteSheet("../Assets/Sprites/SuperMario/SuperMario.png", "../Assets/Sprites/SuperMario/SuperMario.json");

        ac->AddAnimation("dead", {0});    // its not a super mario animation, but we keep it for consistency
        ac->AddAnimation("idle", {0});
        ac->AddAnimation("jump", {2});
        ac->AddAnimation("run", {6, 7, 8});
        ac->AddAnimation("run", {3, 4, 5});
        ac->AddAnimation("stomp", {0, 2});  
        ac->AddAnimation("turn", {0});
        ac->SetAnimFPS(10.0f);
        ac->SetAnimation("idle");

        ac->SetSize(Game::TILE_SIZE, Game::TILE_SIZE * 2.0f);
        ac->SetOffset(Vector2(0.0f, -Game::TILE_SIZE / 2.0f));
    }

    // Update collider size and offset
    auto* cc = GetComponent<AABBColliderComponent>();
    if (cc) {
        cc->SetSize(Game::TILE_SIZE, Game::TILE_SIZE * 2.0f);
        cc->SetOffset(Vector2(0.0f, -Game::TILE_SIZE / 2.0f));
    }
}

void Mario::PowerDown()
{
    if (!mIsSuper) return;
    mIsSuper = false;

    // Set invincibility timer
    mInvincibilityTimer = 1.5f;

    // Update animations based on state
    auto* ac = GetComponent<AnimatorComponent>();
    if (ac) {
        
        ac->SetSpriteSheet("../Assets/Sprites/Mario/Mario.png", "../Assets/Sprites/Mario/Mario.json");

        ac->AddAnimation("dead", {0});
        ac->AddAnimation("idle", {1});
        ac->AddAnimation("jump", {2});
        ac->AddAnimation("run", {3, 4, 5});
        ac->AddAnimation("stomp", {6, 7});  
        ac->AddAnimation("turn", {8});
        ac->SetAnimFPS(10.0f);
        ac->SetAnimation("idle");

        ac->SetSize(Game::TILE_SIZE, Game::TILE_SIZE);
        ac->SetOffset(Vector2::Zero);
    }

    // Update collider size and offset
    auto* cc = GetComponent<AABBColliderComponent>();
    if (cc) {
        cc->SetSize(Game::TILE_SIZE, Game::TILE_SIZE);
        cc->SetOffset(Vector2::Zero);
    }
}