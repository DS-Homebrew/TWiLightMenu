#ifndef ICONTITLE_H
#define ICONTITLE_H

#define NDS_ICON_BANK_COUNT 8
#define BAD_ICON_IDX(i) (i < 0 || i > (NDS_ICON_BANK_COUNT - 1))

#define BOX_PX				40
#define BOX_PY				37
#define BOX_PX_SMALL		24
#define BOX_PY_SMALL		27

void iconTitleInit();

/**
 * Reloads the palette of all the icons in a slot, if
 * they have been corrupted.
 */
void reloadIconPalettes();

void loadConsoleIcons();
void copyGameInfo(int numDst, int numSrc);
void getGameInfo(int num, int fileOffset, bool isDir, const char* name, bool fromArgv);
void iconUpdate(int num, bool isDir, const char* name);
void titleUpdate(int num, bool isDir, const char* name, const bool highlighted);
void drawIconFolder(int Xpos, int Ypos, s32 scale);
void drawIcon(int num, int Xpos, int Ypos, s32 scale);

#endif // ICONTITLE_H