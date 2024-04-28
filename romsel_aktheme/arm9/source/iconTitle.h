#ifndef ICONTITLE_H
#define ICONTITLE_H

#define BOX_PX				40
#define BOX_PY				40

void iconTitleInit();
void loadConsoleIcons();
void getGameInfo(int num, bool isDir, const char* name);
void iconUpdate(int num, bool isDir, const char* name);
void titleUpdate(int num, bool isDir, const char* name);
void drawIconFolder(int Xpos, int Ypos);
void drawIcon(int num, int Xpos, int Ypos);

#endif // ICONTITLE_H