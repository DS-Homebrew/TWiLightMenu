#ifndef TWLMENU_GIFT_H
#define TWLMENU_GIFT_H

#include <string>

// "New software" gifts for the 3DS theme.
//
// An app that has never been opened is drawn as a wrapped gift. Selecting it
// and tapping it (or pressing A) plays an unwrap animation instead of
// launching. The app is then remembered in a list on the card, so it only
// ever shows as a gift once. With no list yet, every app starts out wrapped.

// Length of the unwrap animation, in frames (60 per second)
#define GIFT_ANIM_LENGTH 44

// Per box slot of the current page
extern bool giftWrapped[40];
// 0 = idle, 1..GIFT_ANIM_LENGTH = unwrapping
extern int giftAnimFrame[40];

// Reads the list of opened apps from the card (only done once per run)
void giftLoadSeenList();

// Call when a slot is filled with an entry; name is relative to the current folder
void giftUpdateSlot(int slot, bool isDirectory, const std::string &name);

// Clears the gift state of every slot (call before refilling a page)
void giftClearSlots();

// True if the slot should currently be drawn as a gift
bool giftActive(int slot);

// Sprite sheet frame (0 = closed .. 3 = burst) and horizontal shake for a slot
int giftSpriteFrame(int slot);
int giftShakeX(int slot);

// Remembers the app as opened and saves the list. Call once the animation is done.
void giftFinishUnwrap(int slot, const std::string &name);

#endif
