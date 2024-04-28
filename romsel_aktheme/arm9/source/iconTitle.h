#ifndef ICONTITLE_H
#define ICONTITLE_H

#define BOX_PX				20
#define BOX_PY				44

void iconTitleInit();
void loadConsoleIcons();
void getGameInfo(bool isDir, const char* name);
void iconUpdate(bool isDir, const char* name);
void titleUpdate(bool isDir, const char* name);
void drawIconFolder(int Xpos, int Ypos);
void drawIcon(int Xpos, int Ypos);

#endif // ICONTITLE_H