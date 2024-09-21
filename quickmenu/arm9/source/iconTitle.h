#ifndef ICONTITLE_H
#define ICONTITLE_H

#define NDS_ICON_BANK_COUNT 2
#define BAD_ICON_IDX(i) (i < 0 || i > (NDS_ICON_BANK_COUNT - 1))

#define BOX_PX				22
#define BOX_PY				22

void iconTitleInit();

/**
 * Reloads the palette of all the icons in a slot, if
 * they have been corrupted.
 */
void reloadIconPalettes();

void loadConsoleIcons();
void getGameInfo(int num, bool isDir, const char* name, bool fromArgv);
void iconUpdate(int num, bool isDir, const char* name);
void titleUpdate(int num, bool top, bool isDir, const char* name);
void drawIcon(int num, int Xpos, int Ypos);

#endif // ICONTITLE_H