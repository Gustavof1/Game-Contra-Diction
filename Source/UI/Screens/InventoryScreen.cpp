#include "InventoryScreen.h"
#include "../../Game.h"
#include "../../Renderer/Renderer.h"
#include "../../Actors/Spaceman.h"
#include <SDL.h>

InventoryScreen::InventoryScreen(Game* game)
    : UIScreen(game, "../Assets/Fonts/ALS_Micro_Bold.ttf")
    , mIsDragging(false)
    , mDraggedSlotIndex(-1)
    , mDragImage(nullptr)
    , mWasMouseDown(false)
    , mLastClickTime(0.0f)
    , mLastClickedSlotIndex(-1)
    , mHeadSlotPos(810.0f, 90.0f)
    , mHandSlotPos(720.0f, 397.0f)
{
    mGame->SetState(GameState::Paused);
    mGame->GetAudio()->PauseAllSounds();
    SetupUI();
}

InventoryScreen::~InventoryScreen()
{
    mGame->GetAudio()->ResumeAllSounds();
    mGame->SetState(GameState::Gameplay);
}

void InventoryScreen::SetupUI()
{
    // Background
    UIRect* bg = AddRect(Vector2(Game::WINDOW_WIDTH/2, Game::WINDOW_HEIGHT/2), Vector2(Game::WINDOW_WIDTH, Game::WINDOW_HEIGHT));
    bg->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.8f));
    // Note: UIRect doesn't support alpha in SetColor, but we can rely on global alpha or just have opaque background.
    
    // Grid
    float startX = 200.0f;
    float startY = 200.0f;
    float slotSize = 80.0f;
    float padding = 10.0f;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Slot slot;
            slot.pos = Vector2(startX + j * (slotSize + padding), startY + i * (slotSize + padding));
            slot.item = {ItemType::None, ""};
            
            // Slot background
            UIRect* r = AddRect(slot.pos, Vector2(slotSize, slotSize), 1.0f, 0.0f, 90);
            r->SetColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
            slot.image = nullptr; // We use rect
            
            slot.itemImage = nullptr;
            mGridSlots.push_back(slot);
        }
    }

    // Populate items
    mGridSlots[0].item = {ItemType::Pistol, "../Assets/HUD/ITEM_pistol.png"};
    mGridSlots[1].item = {ItemType::AlienPistol, "../Assets/HUD/ITEM_alien_pistol.png"};
    mGridSlots[2].item = {ItemType::Flashlight, "../Assets/HUD/ITEM_flashlights.PNG"};
    mGridSlots[3].item = {ItemType::Headphones, "../Assets/HUD/ITEM_headphones.PNG"};

    // Character
    Vector2 charPos(800.0f, 300.0f);
    mCharacterImage = AddImage("../Assets/HUD/HUD_character.png", charPos, 0.3f);
    
    // Head Slot
    mHeadSlot.pos = mHeadSlotPos; 
    mHeadSlot.item = {ItemType::None, ""};
    mHeadSlot.itemImage = nullptr;
    mHeadSlot.image = nullptr;
    UIRect* headRect = AddRect(mHeadSlot.pos, Vector2(60, 60), 1.0f, 0.0f, 110);
    headRect->SetColor(Vector4(1.0f, 0.0f, 0.0f, 0.5f)); // Bright Red for visibility
    
    // Hand Slot
    mHandSlot.pos = mHandSlotPos; 
    mHandSlot.item = {ItemType::None, ""};
    mHandSlot.itemImage = nullptr;
    mHandSlot.image = nullptr;
    UIRect* handRect = AddRect(mHandSlot.pos, Vector2(60, 60), 1.0f, 0.0f, 110);
    handRect->SetColor(Vector4(0.0f, 1.0f, 0.0f, 0.5f)); // Bright Green for visibility

    // Sync with current player state
    const Spaceman* player = mGame->GetPlayer();
    if (player) {
        ItemType head = player->GetHeadItem();
        ItemType hand = player->GetHandItem();
        
        if (head != ItemType::None) {
             for (auto& slot : mGridSlots) {
                 if (slot.item.type == head) {
                     mHeadSlot.item = slot.item;
                     mHeadSlot.itemImage = AddImage(slot.item.iconPath, mHeadSlot.pos, 0.2f); // Smaller headphones
                     slot.item = {ItemType::None, ""};
                     break;
                 }
             }
        }
        
        if (hand != ItemType::None) {
             for (auto& slot : mGridSlots) {
                 if (slot.item.type == hand) {
                     mHandSlot.item = slot.item;
                     float scale = 0.3f;
                     if (hand == ItemType::Pistol) scale = 0.7f;
                     else if (hand == ItemType::AlienPistol) scale = 0.5f;
                     mHandSlot.itemImage = AddImage(slot.item.iconPath, mHandSlot.pos, scale);
                     slot.item = {ItemType::None, ""};
                     break;
                 }
             }
        }
    }

    // Create images for remaining grid items
    for (auto& slot : mGridSlots) {
        if (slot.item.type != ItemType::None) {
            float scale = 0.3f;
            if (slot.item.type == ItemType::Headphones) scale = 0.2f; // Smaller headphones
            if (slot.item.type == ItemType::Pistol) scale = 1.5f;
            else if (slot.item.type == ItemType::AlienPistol) scale = 0.5f;
            slot.itemImage = AddImage(slot.item.iconPath, slot.pos, scale);
        }
    }
}

void InventoryScreen::HandleKeyPress(int key)
{
    if (key == SDLK_TAB) {
        Close();
    }
}

void InventoryScreen::Update(float deltaTime)
{
    UIScreen::Update(deltaTime);
    
    int mouseX, mouseY;
    Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);
    Vector2 mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));

    bool isMouseDown = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT));
    bool justPressed = isMouseDown && !mWasMouseDown;
    mWasMouseDown = isMouseDown;

    if (justPressed) {
        int clickedSlot = -1;
        // Check Grid
        for (int i = 0; i < mGridSlots.size(); ++i) {
            Vector2 diff = mousePos - mGridSlots[i].pos;
            if (Math::Abs(diff.x) < 40 && Math::Abs(diff.y) < 40) {
                clickedSlot = i;
                break;
            }
        }
        // Check Head
        if (clickedSlot == -1) {
            Vector2 diff = mousePos - mHeadSlot.pos;
            if (Math::Abs(diff.x) < 30 && Math::Abs(diff.y) < 30) {
                clickedSlot = 9;
            }
        }
        // Check Hand
        if (clickedSlot == -1) {
            Vector2 diff = mousePos - mHandSlot.pos;
            if (Math::Abs(diff.x) < 30 && Math::Abs(diff.y) < 30) {
                clickedSlot = 10;
            }
        }

        if (clickedSlot != -1) {
            float time = SDL_GetTicks() / 1000.0f;
            if (clickedSlot == mLastClickedSlotIndex && time - mLastClickTime < 0.5f) {
                HandleDoubleClick(clickedSlot);
                mLastClickedSlotIndex = -1;
                return; // Skip drag logic
            } else {
                mLastClickedSlotIndex = clickedSlot;
                mLastClickTime = time;
            }
        }
    }

    if (!mIsDragging && isMouseDown) {
        // Check clicks
        for (int i = 0; i < mGridSlots.size(); ++i) {
            if (mGridSlots[i].item.type != ItemType::None) {
                Vector2 diff = mousePos - mGridSlots[i].pos;
                if (Math::Abs(diff.x) < 40 && Math::Abs(diff.y) < 40) {
                    mIsDragging = true;
                    mDraggedSlotIndex = i;
                    mDragItem = mGridSlots[i].item;
                    mDragImage = mGridSlots[i].itemImage;
                    mDragOffset = mGridSlots[i].pos - mousePos;
                    break;
                }
            }
        }
        
        if (!mIsDragging) {
            // Check head slot (index 9)
            Vector2 diff = mousePos - mHeadSlot.pos;
            if (Math::Abs(diff.x) < 30 && Math::Abs(diff.y) < 30 && mHeadSlot.item.type != ItemType::None) {
                mIsDragging = true;
                mDraggedSlotIndex = 9;
                mDragItem = mHeadSlot.item;
                mDragImage = mHeadSlot.itemImage;
                mDragOffset = mHeadSlot.pos - mousePos;
            }
        }
        
        if (!mIsDragging) {
            // Check hand slot (index 10)
            Vector2 diff = mousePos - mHandSlot.pos;
            if (Math::Abs(diff.x) < 30 && Math::Abs(diff.y) < 30 && mHandSlot.item.type != ItemType::None) {
                mIsDragging = true;
                mDraggedSlotIndex = 10;
                mDragItem = mHandSlot.item;
                mDragImage = mHandSlot.itemImage;
                mDragOffset = mHandSlot.pos - mousePos;
            }
        }
    }
    else if (mIsDragging && !isMouseDown) {
        // Released
        mIsDragging = false;
        
        // Check drop target
        bool dropped = false;
        
        // Check Head Slot
        Vector2 diffHead = mousePos - mHeadSlot.pos;
        if (Math::Abs(diffHead.x) < 30 && Math::Abs(diffHead.y) < 30) {
            if (mDragItem.type == ItemType::Headphones) {
                // Swap or Place
                InventoryItem temp = mHeadSlot.item;
                UIImage* tempImg = mHeadSlot.itemImage;
                
                mHeadSlot.item = mDragItem;
                mHeadSlot.itemImage = mDragImage;
                mHeadSlot.itemImage->SetOffset(mHeadSlot.pos);
                mHeadSlot.itemImage->SetScale(0.2f); // Smaller headphones
                
                // Return old item to source slot
                if (mDraggedSlotIndex < 9) {
                    mGridSlots[mDraggedSlotIndex].item = temp;
                    mGridSlots[mDraggedSlotIndex].itemImage = tempImg;
                    if (tempImg) tempImg->SetOffset(mGridSlots[mDraggedSlotIndex].pos);
                } else if (mDraggedSlotIndex == 10) {
                     mHandSlot.item = temp;
                     mHandSlot.itemImage = tempImg;
                     if (tempImg) tempImg->SetOffset(mHandSlot.pos);
                }
                
                // Update Player
                if (auto* player = mGame->GetPlayer()) {
                    player->EquipHead(ItemType::Headphones);
                }
                dropped = true;
            }
        }
        
        // Check Hand Slot
        Vector2 diffHand = mousePos - mHandSlot.pos;
        if (!dropped && Math::Abs(diffHand.x) < 45 && Math::Abs(diffHand.y) < 45) {
            if (mDragItem.type == ItemType::Pistol || mDragItem.type == ItemType::AlienPistol || mDragItem.type == ItemType::Flashlight) {
                 InventoryItem temp = mHandSlot.item;
                 UIImage* tempImg = mHandSlot.itemImage;
                 
                 mHandSlot.item = mDragItem;
                 mHandSlot.itemImage = mDragImage;
                 mHandSlot.itemImage->SetOffset(mHandSlot.pos);
                 float scale = 0.3f;
                 if (mDragItem.type == ItemType::Pistol) scale = 0.7f;
                 else if (mDragItem.type == ItemType::AlienPistol) scale = 0.5f;
                 mHandSlot.itemImage->SetScale(scale);
                 
                 if (mDraggedSlotIndex < 9) {
                    mGridSlots[mDraggedSlotIndex].item = temp;
                    mGridSlots[mDraggedSlotIndex].itemImage = tempImg;
                    if (tempImg) tempImg->SetOffset(mGridSlots[mDraggedSlotIndex].pos);
                } else if (mDraggedSlotIndex == 9) {
                     mHeadSlot.item = temp;
                     mHeadSlot.itemImage = tempImg;
                     if (tempImg) tempImg->SetOffset(mHeadSlot.pos);
                }
                
                if (auto* player = mGame->GetPlayer()) {
                    player->EquipHand(mDragItem.type);
                }
                dropped = true;
            }
        }
        
        // Check Grid Slots
        if (!dropped) {
            for (int i = 0; i < mGridSlots.size(); ++i) {
                Vector2 diff = mousePos - mGridSlots[i].pos;
                if (Math::Abs(diff.x) < 40 && Math::Abs(diff.y) < 40) {
                    // Swap
                    InventoryItem temp = mGridSlots[i].item;
                    UIImage* tempImg = mGridSlots[i].itemImage;
                    
                    mGridSlots[i].item = mDragItem;
                    mGridSlots[i].itemImage = mDragImage;
                    mGridSlots[i].itemImage->SetOffset(mGridSlots[i].pos);
                    
                    float scale = 0.3f;
                    if (mDragItem.type == ItemType::Headphones) scale = 0.2f;
                    if (mDragItem.type == ItemType::Pistol) scale = 0.7f;
                    else if (mDragItem.type == ItemType::AlienPistol) scale = 0.5f;
                    mGridSlots[i].itemImage->SetScale(scale);
                    
                    if (mDraggedSlotIndex < 9) {
                        mGridSlots[mDraggedSlotIndex].item = temp;
                        mGridSlots[mDraggedSlotIndex].itemImage = tempImg;
                        if (tempImg) tempImg->SetOffset(mGridSlots[mDraggedSlotIndex].pos);
                    } else if (mDraggedSlotIndex == 9) {
                        mHeadSlot.item = temp;
                        mHeadSlot.itemImage = tempImg;
                        if (tempImg) tempImg->SetOffset(mHeadSlot.pos);
                        // Unequip head
                        if (auto* player = mGame->GetPlayer()) {
                            player->EquipHead(ItemType::None);
                        }
                    } else if (mDraggedSlotIndex == 10) {
                        mHandSlot.item = temp;
                        mHandSlot.itemImage = tempImg;
                        if (tempImg) tempImg->SetOffset(mHandSlot.pos);
                        // Unequip hand
                        if (auto* player = mGame->GetPlayer()) {
                            player->EquipHand(ItemType::None);
                        }
                    }
                    dropped = true;
                    break;
                }
            }
        }
        
        if (!dropped) {
            // Return to original position
            if (mDraggedSlotIndex < 9) {
                mDragImage->SetOffset(mGridSlots[mDraggedSlotIndex].pos);
            } else if (mDraggedSlotIndex == 9) {
                mDragImage->SetOffset(mHeadSlot.pos);
            } else if (mDraggedSlotIndex == 10) {
                mDragImage->SetOffset(mHandSlot.pos);
            }
        }
        
        mDragImage = nullptr;
        mDraggedSlotIndex = -1;
    }
    
    if (mIsDragging && mDragImage) {
        mDragImage->SetOffset(mousePos + mDragOffset);
    }
}

void InventoryScreen::HandleDoubleClick(int slotIndex)
{
    // Grid Slot -> Equip
    if (slotIndex < 9) {
        InventoryItem& item = mGridSlots[slotIndex].item;
        if (item.type == ItemType::None) return;

        if (item.type == ItemType::Headphones) {
            // Swap with Head Slot
            InventoryItem temp = mHeadSlot.item;
            UIImage* tempImg = mHeadSlot.itemImage;

            mHeadSlot.item = item;
            mHeadSlot.itemImage = mGridSlots[slotIndex].itemImage;
            mHeadSlot.itemImage->SetOffset(mHeadSlot.pos);
            mHeadSlot.itemImage->SetScale(0.2f);

            mGridSlots[slotIndex].item = temp;
            mGridSlots[slotIndex].itemImage = tempImg;
            if (tempImg) tempImg->SetOffset(mGridSlots[slotIndex].pos);

            if (auto* player = mGame->GetPlayer()) {
                player->EquipHead(ItemType::Headphones);
            }
        }
        else if (item.type == ItemType::Pistol || item.type == ItemType::AlienPistol || item.type == ItemType::Flashlight) {
            // Swap with Hand Slot
            InventoryItem temp = mHandSlot.item;
            UIImage* tempImg = mHandSlot.itemImage;

            mHandSlot.item = item;
            mHandSlot.itemImage = mGridSlots[slotIndex].itemImage;
            mHandSlot.itemImage->SetOffset(mHandSlot.pos);
            float scale = 0.3f;
            if (item.type == ItemType::Pistol) scale = 0.7f;
            else if (item.type == ItemType::AlienPistol) scale = 0.5f;
            mHandSlot.itemImage->SetScale(scale);

            mGridSlots[slotIndex].item = temp;
            mGridSlots[slotIndex].itemImage = tempImg;
            if (tempImg) tempImg->SetOffset(mGridSlots[slotIndex].pos);

            if (auto* player = mGame->GetPlayer()) {
                player->EquipHand(item.type);
            }
        }
    }
    // Head Slot -> Unequip
    else if (slotIndex == 9) {
        if (mHeadSlot.item.type == ItemType::None) return;
        
        // Find first empty grid slot
        for (int i = 0; i < mGridSlots.size(); ++i) {
            if (mGridSlots[i].item.type == ItemType::None) {
                mGridSlots[i].item = mHeadSlot.item;
                mGridSlots[i].itemImage = mHeadSlot.itemImage;
                mGridSlots[i].itemImage->SetOffset(mGridSlots[i].pos);
                mGridSlots[i].itemImage->SetScale(0.2f); // Assuming headphones

                mHeadSlot.item = {ItemType::None, ""};
                mHeadSlot.itemImage = nullptr;

                if (auto* player = mGame->GetPlayer()) {
                    player->EquipHead(ItemType::None);
                }
                break;
            }
        }
    }
    // Hand Slot -> Unequip
    else if (slotIndex == 10) {
        if (mHandSlot.item.type == ItemType::None) return;

        // Find first empty grid slot
        for (int i = 0; i < mGridSlots.size(); ++i) {
            if (mGridSlots[i].item.type == ItemType::None) {
                mGridSlots[i].item = mHandSlot.item;
                mGridSlots[i].itemImage = mHandSlot.itemImage;
                mGridSlots[i].itemImage->SetOffset(mGridSlots[i].pos);
                
                float scale = 0.3f;
                if (mGridSlots[i].item.type == ItemType::Pistol) scale = 0.7f;
                else if (mGridSlots[i].item.type == ItemType::AlienPistol) scale = 0.5f;
                mGridSlots[i].itemImage->SetScale(scale);

                mHandSlot.item = {ItemType::None, ""};
                mHandSlot.itemImage = nullptr;

                if (auto* player = mGame->GetPlayer()) {
                    player->EquipHand(ItemType::None);
                }
                break;
            }
        }
    }
}
