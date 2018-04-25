#ifndef _display_h_
#define _display_h_

#define TOPSCREEN 0
#define BTMSCREEM 0

void initDisplay();
void kprintf(const char *str, ...);
void setCursor(int row, int column);
void getCursor(int *row, int *column);
void setScreen(int screen);

#endif
