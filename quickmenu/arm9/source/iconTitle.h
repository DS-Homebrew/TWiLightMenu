#ifndef ICONTITLE_H
#define ICONTITLE_H

#define BOX_PX				22
#define BOX_PY				22

void iconTitleInit();
void loadConsoleIcons();
void getGameInfo(int num, bool isDir, const char* name);
void iconUpdate(int num, bool isDir, const char* name);
void titleUpdate(int num, bool top, bool isDir, const char* name);
void drawIcon(int num, int Xpos, int Ypos);

#endif // ICONTITLE_H