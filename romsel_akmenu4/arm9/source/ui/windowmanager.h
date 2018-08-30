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
#include "userinput.h"

namespace akui
{
class WindowManager
{
private:
  struct WindowRec
  {
    Window *_window;
    Window *_focused;
    WindowRec(Window *window, Window *focused = NULL) : _window(window), _focused(focused) {}
    Window *operator()(void) const { return _window; }
  };
  typedef std::list<WindowRec> Windows;

private:
  Windows _backgroundWindows;
  WindowRec _currentWindow;
  Window *_focusedWindow;
  Window *_windowBelowPen;
  Window *_capturedWindow; //process touch for non-focusable window
protected:
  WindowManager &checkForWindowBelowPen(const Point &touchPoint);
  Window *windowBelowPen(void) const { return _windowBelowPen; }
  void updateFocusIfNecessary(const Point &touchPoint);
  bool process(Message &message) const;
  bool processTouchMessage(TouchMessage &message);
  const WindowManager &updateBackground(void);

public:
  WindowManager();
  ~WindowManager();
  Window *focusedWindow(void) const { return _focusedWindow; }
  void setFocusedWindow(Window *aWindow);
  WindowManager &addWindow(Window *aWindow);
  WindowManager &removeWindow(Window *aWindow);
  Window *currentWindow(void) const { return _currentWindow(); }
  const WindowManager &update(void);
  bool onKeyDown(unsigned char keyCode, unsigned char shift);
  bool onKeyUp(unsigned char keyCode, unsigned char shift);
  bool onTouchDown(int x, int y);
  bool onTouchUp(int x, int y);
  bool onTouchMove(int x, int y);
};

typedef singleton<WindowManager> WindowManager_s;
inline WindowManager &windowManager(void) { return WindowManager_s::instance(); }
} // namespace akui

#endif //_AKUI_WINDOWMANAGER_H_
