//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "AABBColliderComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include "RigidBodyComponent.h"

AABBColliderComponent::AABBColliderComponent(class Actor* owner, int dx, int dy, int w, int h,
        ColliderLayer layer, bool isTrigger, bool isStatic, int updateOrder)
        :Component(owner, updateOrder)
        ,mOffset(Vector2((float)dx, (float)dy))
        ,mIsStatic(isStatic)
        ,mIsTrigger(isTrigger)
        ,mWidth(w)
        ,mHeight(h)
        ,mLayer(layer)
{
    GetGame()->AddCollider(this);
}

AABBColliderComponent::~AABBColliderComponent()
{
    GetGame()->RemoveCollider(this);
}

Vector2 AABBColliderComponent::GetMin() const
{
    Vector2 center = mOwner->GetPosition() + mOffset;
    float halfWidth = mWidth / 2.0f;
    float halfHeight = mHeight / 2.0f;
    return Vector2(center.x - halfWidth, center.y - halfHeight);
}

Vector2 AABBColliderComponent::GetMax() const
{
    Vector2 center = mOwner->GetPosition() + mOffset;
    float halfWidth = mWidth / 2.0f;
    float halfHeight = mHeight / 2.0f;
    return Vector2(center.x + halfWidth, center.y + halfHeight);
}

bool AABBColliderComponent::Intersect(const AABBColliderComponent& b) const
{
    Vector2 aMin = this->GetMin();
    Vector2 aMax = this->GetMax();
    Vector2 bMin = b.GetMin();
    Vector2 bMax = b.GetMax();

    bool overlapX = (aMax.x >= bMin.x) && (aMin.x <= bMax.x);
    bool overlapY = (aMax.y >= bMin.y) && (aMin.y <= bMax.y);

    return overlapX && overlapY;
}

float AABBColliderComponent::GetMinVerticalOverlap(AABBColliderComponent* b) const
{
    Vector2 aMin = this->GetMin();
    Vector2 aMax = this->GetMax();
    Vector2 bMin = b->GetMin();
    Vector2 bMax = b->GetMax();

    float dy1 = bMax.y - aMin.y;
    float dy2 = aMax.y - bMin.y;

    if (dy1 < dy2) {
        return -dy1;
    }
    else {
        return dy2;
    }
}

float AABBColliderComponent::GetMinHorizontalOverlap(AABBColliderComponent* b) const
{
    Vector2 aMin = this->GetMin();
    Vector2 aMax = this->GetMax();
    Vector2 bMin = b->GetMin();
    Vector2 bMax = b->GetMax();

    float dx1 = bMax.x - aMin.x;
    float dx2 = aMax.x - bMin.x;
    
    if (dx1 < dx2) {
        return -dx1;
    }
    else {
        return dx2;
    }
}

float AABBColliderComponent::DetectHorizontalCollision(RigidBodyComponent *rigidBody)
{
    if (mIsStatic || !IsEnabled()) return 0.0f;

    auto colliders = GetGame()->GetColliders();

    for (auto collider : colliders) {
        if (collider == this || !collider->IsEnabled()) {
            continue;
        }

        if (this->Intersect(*collider)) {
            float minXOverlap = GetMinHorizontalOverlap(collider);
            float minYOverlap = GetMinVerticalOverlap(collider);

            // Fix for "seam" issue: If falling and hitting floor, prefer vertical resolution
            float yBias = 1.0f;
            if (rigidBody->GetVelocity().y > 0.0f && minYOverlap > 0.0f) {
                yBias = 0.01f; // Strong bias to treat Y overlap as small
            }

            if (fabs(minXOverlap) < fabs(minYOverlap) * yBias) {
                if (!collider->IsTrigger()) {
                    ResolveHorizontalCollisions(rigidBody, minXOverlap);
                }
                mOwner->OnHorizontalCollision(minXOverlap, collider);
            }
        }
    }

    return 0.0f;
}

float AABBColliderComponent::DetectVertialCollision(RigidBodyComponent *rigidBody)
{
    if (mIsStatic || !IsEnabled()) return 0.0f;

    auto colliders = GetGame()->GetColliders();

    for (auto collider : colliders) {
        if (collider == this || !collider->IsEnabled()) {
            continue;
        }

        if (this->Intersect(*collider)) {
            float minXOverlap = GetMinHorizontalOverlap(collider);
            float minYOverlap = GetMinVerticalOverlap(collider);

            // Fix for "seam" issue: If falling and hitting floor, prefer vertical resolution
            float yBias = 1.0f;
            if (rigidBody->GetVelocity().y > 0.0f && minYOverlap > 0.0f) {
                yBias = 0.01f;
            }

            if (fabs(minYOverlap) * yBias <= fabs(minXOverlap)) {
                if (!collider->IsTrigger()) {
                    ResolveVerticalCollisions(rigidBody, minYOverlap);
                }
                mOwner->OnVerticalCollision(minYOverlap, collider);
            }
        }
    }

    return 0.0f;
}

void AABBColliderComponent::ResolveHorizontalCollisions(RigidBodyComponent *rigidBody, const float minXOverlap)
{
    // Clamp overlap to avoid teleportation
    float overlap = minXOverlap;
    float maxOverlap = 32.0f; // Game::TILE_SIZE
    if (overlap > maxOverlap) overlap = maxOverlap;
    if (overlap < -maxOverlap) overlap = -maxOverlap;

    // Adjust position
    Vector2 pos = mOwner->GetPosition();
    pos.x -= overlap;
    mOwner->SetPosition(pos);

    // Zero out horizontal velocity
    Vector2 vel = rigidBody->GetVelocity();
    vel.x = 0.0f;
    rigidBody->SetVelocity(vel);
}

void AABBColliderComponent::ResolveVerticalCollisions(RigidBodyComponent *rigidBody, const float minYOverlap)
{
    // Clamp overlap to avoid teleportation
    float overlap = minYOverlap;
    float maxOverlap = 32.0f; // Game::TILE_SIZE
    if (overlap > maxOverlap) overlap = maxOverlap;
    if (overlap < -maxOverlap) overlap = -maxOverlap;

    // Adjust position
    Vector2 pos = mOwner->GetPosition();
    pos.y -= overlap;
    mOwner->SetPosition(pos);

    // Zero out vertical velocity
    Vector2 vel = rigidBody->GetVelocity();
    vel.y = 0.0f;
    rigidBody->SetVelocity(vel);

    if (minYOverlap > 0) {
        mOwner->SetOnGround();
    }
}

void AABBColliderComponent::SetSize(float width, float height)
{
    mWidth = width;
    mHeight = height;
}

void AABBColliderComponent::SetOffset(const Vector2& offset)
{
    mOffset = offset;
}

void AABBColliderComponent::DebugDraw(class Renderer *renderer)
{
    renderer->DrawRect(mOwner->GetPosition() + mOffset,Vector2((float)mWidth, (float)mHeight), mOwner->GetRotation(),
                       Color::Green, mOwner->GetGame()->GetCameraPos(), RendererMode::LINES);
}

void AABBColliderComponent::Update(float deltaTime)
{
    // Só executamos essa lógica se for um TRIGGER e tiver um callback configurado
    if (mIsTrigger && mCollisionCallback)
    {
        // Pega todos os colisores do jogo
        auto colliders = GetGame()->GetColliders();

        for (auto collider : colliders)
        {
            // Não colidir consigo mesmo nem com triggers desativados
            if (collider == this || !collider->IsEnabled()) {
                continue;
            }

            // Se houver interseção
            if (Intersect(*collider))
            {
                // CHAMA O CALLBACK!
                mCollisionCallback(collider);
            }
        }
    }
}