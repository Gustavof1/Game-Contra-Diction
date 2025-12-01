#include "Laser.h"
#include "../Game.h"
#include "../Components/Drawing/RectComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include <algorithm>

namespace {
    bool IntersectSegmentAABB(const Vector2& p0, const Vector2& p1, const Vector2& min, const Vector2& max)
    {
        Vector2 d = p1 - p0;
        float tmin = 0.0f;
        float tmax = 1.0f;

        // X axis
        if (Math::Abs(d.x) < 0.0001f) {
            if (p0.x < min.x || p0.x > max.x) return false;
        } else {
            float ood = 1.0f / d.x;
            float t1 = (min.x - p0.x) * ood;
            float t2 = (max.x - p0.x) * ood;
            if (t1 > t2) std::swap(t1, t2);
            tmin = Math::Max(tmin, t1);
            tmax = Math::Min(tmax, t2);
            if (tmin > tmax) return false;
        }

        // Y axis
        if (Math::Abs(d.y) < 0.0001f) {
            if (p0.y < min.y || p0.y > max.y) return false;
        } else {
            float ood = 1.0f / d.y;
            float t1 = (min.y - p0.y) * ood;
            float t2 = (max.y - p0.y) * ood;
            if (t1 > t2) std::swap(t1, t2);
            tmin = Math::Max(tmin, t1);
            tmax = Math::Min(tmax, t2);
            if (tmin > tmax) return false;
        }

        return true;
    }
}

Laser::Laser(Game* game)
    : Actor(game)
{
    mRect = new RectComponent(this, 100, 100, RendererMode::TRIANGLES, 200);
    mRect->SetColor(Vector3(1.0f, 0.0f, 0.0f)); // Red laser
    mRect->SetVisible(false);
    
    // Collider for damage? 
    // For a continuous beam, we might want to do raycasting or a long collider.
    // For now, let's just make it visual and maybe a long collider.
    mCollider = new AABBColliderComponent(this, 0, 0, 1, 1, ColliderLayer::PlayerProjectile, true);
    mCollider->SetEnabled(false);
}

void Laser::SetActive(bool active)
{
    mRect->SetVisible(active);
    mCollider->SetEnabled(active);
    if (!active) {
        // Move away to avoid collisions
        SetPosition(Vector2(-1000.0f, -1000.0f));
    }
}

void Laser::UpdateBeam(const Vector2& start, const Vector2& direction)
{
    if (!mRect->IsVisible()) return;

    // Simple implementation: Fixed length beam for now, or raycast to find end.
    // Let's do a fixed length long beam.
    float length = 800.0f; // Screen width approx
    
    // Center of the beam segment
    Vector2 center = start + direction * (length / 2.0f);
    SetPosition(center);
    SetRotation(Math::Atan2(direction.y, direction.x));
    
    mRect->SetWidth(static_cast<int>(length));
    mRect->SetHeight(4); // Thin beam

    // Update collider
    mCollider->SetSize(length, 4);
    mCollider->SetOffset(Vector2(0, 0));

    // Raycast against enemies
    Vector2 end = start + direction * length;
    auto colliders = GetGame()->GetColliders();
    for (auto collider : colliders) {
        if (collider->GetLayer() == ColliderLayer::Enemy && collider->IsEnabled()) {
            if (IntersectSegmentAABB(start, end, collider->GetMin(), collider->GetMax())) {
                collider->GetOwner()->Kill();
            }
        }
    }
}
