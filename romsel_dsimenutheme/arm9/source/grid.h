#ifndef TWLMENU_GRID_H
#define TWLMENU_GRID_H

#include <string_view>

// "Multiple rows" view for the 3DS theme.
//
// Instead of one scrolling row of boxes, the bottom screen shows a grid of
// GRID_COLS x GRID_ROWS boxes (like the 3DS HOME Menu). Left/Right step through the
// apps, Up/Down jump a row, and the grid scrolls a row at a time. Tapping a box
// selects it, tapping the selected box opens it. It is switched on in the settings
// ("Multiple rows"). With it off nothing here is used and the menu behaves as before.

#define GRID_COLS 3
#define GRID_ROWS 2
#define GRID_VISIBLE (GRID_COLS * GRID_ROWS)	// Must stay <= ICON_GRID_BANKS (see iconHandler.h)

// Top left corner of the first box, and the distance from one box to the next.
// The small cartridge icons take the top 33 pixels and the L/R shoulders start at 172.
#define GRID_X0 24
#define GRID_XSTEP 72
#define GRID_Y0 34
#define GRID_YSTEP 68
#define GRID_BOX 64

// Row shown at the top of the grid. Always kept so that the cursor is on screen.
extern int gridTopRow;

// True while the grid view is in use
bool gridEnabled();

// Scroll just enough to bring slot pos on screen. Returns true if the rows moved.
bool gridEnsureVisible(int pos);

// First slot shown (top left box). The visible slots are first .. first + GRID_VISIBLE - 1.
int gridFirstSlot();

// Screen position of a slot's box. Returns false if the slot is not on screen.
bool gridCellPos(int slot, int *x, int *y);

// Slot under a screen position (a touch), or -1 if the point is not on a box.
int gridSlotAt(int px, int py);

// Writes the name of the selected app below the grid (replaces the title bubble).
void gridWriteTitle(bool isDir, std::string_view name);

#endif
