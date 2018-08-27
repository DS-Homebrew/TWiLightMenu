/*
    windowmanager.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _AKUI_WINDOWMANAGER_H_
#define _AKUI_WINDOWMANAGER_H_

#include <string>
#include <list>
#include "window.h"
#include "../userinput.h"

namespace akui
{
class cWindowManager
{
private:
  struct cWindowRec
  {
    cWindow *_window;
    cWindow *_focused;
    cWindowRec(cWindow *window, cWindow *focused = NULL) : _window(window), _focused(focused) {}
    cWindow *operator()(void) const { return _window; }
  };
  typedef std::list<cWindowRec> cWindows;

private:
  cWindows _backgroundWindows;
  cWindowRec _currentWindow;
  cWindow *_focusedWindow;
  cWindow *_windowBelowPen;
  cWindow *_capturedWindow; //process touch for non-focusable window
protected:
  cWindowManager &checkForWindowBelowPen(const cPoint &touchPoint);
  cWindow *windowBelowPen(void) const { return _windowBelowPen; }
  void updateFocusIfNecessary(const cPoint &touchPoint);
  bool process(cMessage &message) const;
  bool processTouchMessage(cTouchMessage &message);
  const cWindowManager &updateBackground(void);

public:
  cWindowManager();
  ~cWindowManager();
  cWindow *focusedWindow(void) const { return _focusedWindow; }
  void setFocusedWindow(cWindow *aWindow);
  cWindowManager &addWindow(cWindow *aWindow);
  cWindowManager &removeWindow(cWindow *aWindow);
  cWindow *currentWindow(void) const { return _currentWindow(); }
  const cWindowManager &update(void);
  bool onKeyDown(unsigned char keyCode, unsigned char shift);
  bool onKeyUp(unsigned char keyCode, unsigned char shift);
  bool onTouchDown(int x, int y);
  bool onTouchUp(int x, int y);
  bool onTouchMove(int x, int y);
};

typedef t_singleton<cWindowManager> cWindowManager_s;
inline cWindowManager &windowManager(void) { return cWindowManager_s::instance(); }
} // namespace akui

#endif //_AKUI_WINDOWMANAGER_H_
