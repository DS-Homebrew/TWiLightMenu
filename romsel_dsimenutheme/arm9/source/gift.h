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

// True if the slot should currently be drawn as a gift. This does not depend on the
// platform: NDS, GBA, GB, GBC, NES, SNES and every other launchable file are all
// wrapped until opened. Only folders and the system Settings entry are skipped.
bool giftActive(int slot);

// True while the app being carried in move mode is still wrapped. Remembered when the
// app is picked up, because the slot it came from can be refilled by a page flip.
extern bool giftMovingApp;

// Sprite sheet frame (0 = closed .. 3 = burst) and horizontal shake for a slot
int giftSpriteFrame(int slot);
int giftShakeX(int slot);

// Remembers the app as opened and saves the list. Call once the animation is done.
void giftFinishUnwrap(int slot, const std::string &name);

// ---- Spinning 3D gift on the top screen (3DS theme) ----
// While a wrapped gift is selected, a short turntable animation of a gift box plays where
// the box art normally goes. Frames are stored in nitro:/graphics/giftspin.bin, one after
// the other: per frame W*H RGB555 pixels (little endian) followed by W*H alpha bytes.
#define GIFT_SPIN_W 112
#define GIFT_SPIN_H 112
#define GIFT_SPIN_FRAMES 24			// 7.5 degrees apart, covering a half turn (the gift looks the same after that)
#define GIFT_SPIN_TICKS 4			// Screen frames per animation frame
#define GIFT_SPIN_FRAME_BYTES (GIFT_SPIN_W * GIFT_SPIN_H * 3)
#ifndef GIFT_SPIN_PATH
#define GIFT_SPIN_PATH "nitro:/graphics/giftspin.bin"
#endif

// Call once per screen frame. wanted = a wrapped gift is selected right now.
// Starts, advances or stops the animation. Returns true while it is on screen,
// in which case the caller should skip drawing normal box art.
bool giftSpinUpdate(bool wanted);

// Draws one frame (w x h) over bg into dst. dst and bg are 256 pixels wide.
// Transparent pixels show bg, partly transparent ones are blended with it.
void giftComposite(unsigned short *dst, const unsigned short *bg, const unsigned char *frame,
		int w, int h, const unsigned short *colorTable);

#endif
