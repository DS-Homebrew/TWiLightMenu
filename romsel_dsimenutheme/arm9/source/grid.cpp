#include "grid.h"

#include <algorithm>
#include <string>

#include "common/twlmenusettings.h"
#include "graphics/fontHandler.h"
#include "language.h"

int gridTopRow = 0;

bool gridEnabled() {
	return ms().multiRow && ms().theme == TWLSettings::ETheme3DS && !ms().macroMode;
}

int gridFirstSlot() {
	return gridTopRow * GRID_COLS;
}

bool gridEnsureVisible(int pos) {
	const int oldTop = gridTopRow;
	const int row = std::max(pos, 0) / GRID_COLS;
	if (row < gridTopRow) {
		gridTopRow = row;
	} else if (row > gridTopRow + GRID_ROWS - 1) {
		gridTopRow = row - (GRID_ROWS - 1);
	}
	if (gridTopRow < 0) {
		gridTopRow = 0;
	}
	return gridTopRow != oldTop;
}

bool gridCellPos(int slot, int *x, int *y) {
	if (slot < 0) {
		return false;
	}
	const int row = (slot / GRID_COLS) - gridTopRow;
	const int col = slot % GRID_COLS;
	if (row < 0 || row >= GRID_ROWS) {
		return false;
	}
	*x = GRID_X0 + (col * GRID_XSTEP);
	*y = GRID_Y0 + (row * GRID_YSTEP);
	return true;
}

int gridSlotAt(int px, int py) {
	for (int row = 0; row < GRID_ROWS; row++) {
		for (int col = 0; col < GRID_COLS; col++) {
			const int x = GRID_X0 + (col * GRID_XSTEP);
			const int y = GRID_Y0 + (row * GRID_YSTEP);
			if (px >= x && px < x + GRID_BOX && py >= y && py < y + GRID_BOX) {
				const int slot = ((gridTopRow + row) * GRID_COLS) + col;
				return (slot <= 39) ? slot : -1;
			}
		}
	}
	return -1;
}

void gridWriteTitle(bool isDir, std::string_view name) {
	std::string text;
	if (isDir && name == "..") {
		text = STR_BACK;
	} else {
		text = std::string(name);
		if (!isDir) {
			// Hide the file extension
			const size_t dot = text.rfind('.');
			if (dot != std::string::npos && dot > 0) {
				text.erase(dot);
			}
		}
	}
	// Fit between the L and R shoulder labels
	const int maxWidth = 112;
	if (calcSmallFontWidth(text) > maxWidth) {
		while (text.size() > 1 && calcSmallFontWidth(text + "...") > maxWidth) {
			text.pop_back();
			// Do not leave half of a multi-byte character behind
			size_t start = text.size() - 1;
			while (start > 0 && (text[start] & 0xC0) == 0x80) {
				start--;
			}
			const unsigned char lead = (unsigned char)text[start];
			const size_t need = (lead >= 0xF0) ? 4 : (lead >= 0xE0) ? 3 : (lead >= 0xC0) ? 2 : 1;
			if (text.size() - start < need && start > 0) {
				text.erase(start);
			}
		}
		text += "...";
	}
	printSmall(false, 128, 175, text, Alignment::center);
}
