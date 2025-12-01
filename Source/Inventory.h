#pragma once
#include <string>

enum class ItemType {
    None,
    Headphones,
    Flashlight,
    Pistol,
    AlienPistol
};

struct InventoryItem {
    ItemType type;
    std::string iconPath;
};
