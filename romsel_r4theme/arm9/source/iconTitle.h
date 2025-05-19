#ifndef ICONTITLE_H
#define ICONTITLE_H

#define BOX_PX				20
#define BOX_PY				44
#define BOX_PY_GBNP			135

void iconTitleInit();

/**
 * Reloads the palette of all the icons in a slot, if
 * they have been corrupted.
 */
void reloadIconPalettes();

void loadConsoleIcons();
void getGameInfo(int fileOffset, bool isDir, const char* name, bool fromArgv);
void iconUpdate(bool isDir, const char* name);
void titleUpdate(bool isDir, const char* name);
void drawIconFolder(int Xpos, int Ypos);
void drawIcon(int Xpos, int Ypos);

#endif // ICONTITLE_H