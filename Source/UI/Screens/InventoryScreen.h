#pragma once
#include "UIScreen.h"
#include "../../Inventory.h"
#include <vector>

class InventoryScreen : public UIScreen
{
public:
    InventoryScreen(class Game* game);
    ~InventoryScreen();

    void Update(float deltaTime) override;
    void HandleKeyPress(int key) override;

private:
    void SetupUI();
    void HandleDoubleClick(int slotIndex);

    // Grid slots (3x3)
    struct Slot {
        Vector2 pos;
        InventoryItem item;
        class UIImage* image; // The background of the slot
        class UIImage* itemImage; // The item icon
    };
    std::vector<Slot> mGridSlots;

    // Character slots
    Slot mHeadSlot;
    Slot mHandSlot;

    // Dragging
    bool mIsDragging;
    int mDraggedSlotIndex; // -1 for none, 0-8 for grid, 9 for head, 10 for hand
    Vector2 mDragOffset;
    class UIImage* mDragImage;
    InventoryItem mDragItem;

    class UIImage* mCharacterImage;

    // Double Click Logic
    bool mWasMouseDown;
    float mLastClickTime;
    int mLastClickedSlotIndex;

    // Configurable Positions
    Vector2 mHeadSlotPos;
    Vector2 mHandSlotPos;
};
