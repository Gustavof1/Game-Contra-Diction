#include "HUD.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"
#include "../Actors/Spaceman.h"
#include "../Inventory.h"

HUD::HUD(Game* game)
    : mGame(game)
{
    Renderer* r = mGame->GetRenderer();
    mTexPistol = r->GetTexture("../Assets/HUD/ITEM_pistol.png");
    mTexAlienPistol = r->GetTexture("../Assets/HUD/ITEM_alien_pistol.png");
    mTexFlashlight = r->GetTexture("../Assets/HUD/ITEM_flashlights.PNG");
    mTexHeadphones = r->GetTexture("../Assets/HUD/ITEM_headphones.PNG");
}

HUD::~HUD()
{
}

void HUD::Draw(Renderer* renderer)
{
    const Spaceman* player = mGame->GetPlayer();
    if (!player) return;

    Vector2 cameraPos = mGame->GetCameraPos();

    // Draw 2 slots in top-left corner
    // Slot size
    Vector2 slotSize(60.0f, 60.0f);
    float padding = 10.0f;
    float startX = 20.0f;
    float startY = 20.0f;

    // Slot 1: Head
    Vector2 screenPos1(startX + slotSize.x/2, startY + slotSize.y/2);
    Vector2 pos1 = cameraPos + screenPos1;
    
    renderer->DrawRect(pos1, slotSize, 0.0f, Vector3(0.5f, 0.5f, 0.5f), cameraPos, RendererMode::TRIANGLES, 0.5f);
    
    ItemType head = player->GetHeadItem();
    if (head == ItemType::Headphones) {
        renderer->DrawTexture(pos1, slotSize, 0.0f, Vector3::One, mTexHeadphones, Vector4::UnitRect, cameraPos, Vector2(1.0f, 1.0f), 0.2f); // Smaller
    }

    // Slot 2: Hand
    Vector2 screenPos2(startX + slotSize.x + padding + slotSize.x/2, startY + slotSize.y/2);
    Vector2 pos2 = cameraPos + screenPos2;
    
    renderer->DrawRect(pos2, slotSize, 0.0f, Vector3(0.5f, 0.5f, 0.5f), cameraPos, RendererMode::TRIANGLES, 0.5f);

    ItemType hand = player->GetHandItem();
    Texture* handTex = nullptr;
    float scale = 0.3f;

    if (hand == ItemType::Pistol) {
        handTex = mTexPistol;
        scale = 0.4f; // Bigger
    } else if (hand == ItemType::AlienPistol) {
        handTex = mTexAlienPistol;
        scale = 0.4f; // Bigger
    } else if (hand == ItemType::Flashlight) {
        handTex = mTexFlashlight;
        scale = 0.3f;
    }

    if (handTex) {
        renderer->DrawTexture(pos2, slotSize, 0.0f, Vector3::One, handTex, Vector4::UnitRect, cameraPos, Vector2(1.0f, 1.0f), scale);
    }
}
