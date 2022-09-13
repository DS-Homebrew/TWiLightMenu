/*
    windowmanager.cpp
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

#include "ui.h"
#include "windowmanager.h"

namespace akui
{
WindowManager::WindowManager() : _currentWindow(NULL, NULL), _focusedWindow(NULL), _windowBelowPen(NULL), _capturedWindow(NULL)
{
}

WindowManager::~WindowManager()
{
}

void WindowManager::setFocusedWindow(Window *aWindow)
{
  if (aWindow != focusedWindow() && (!aWindow || aWindow->isFocusable()))
  {
    if (focusedWindow())
      focusedWindow()->disableFocused();
    _focusedWindow = aWindow;
    if (aWindow)
      aWindow->enableFocused();
  }
}

WindowManager &WindowManager::addWindow(Window *aWindow)
{
  if (_currentWindow())
  {
    _currentWindow._focused = focusedWindow();
    _backgroundWindows.push_back(_currentWindow);
  }
  _currentWindow = WindowRec(aWindow);
  setFocusedWindow(aWindow);
  updateBackground();
  return *this;
}

WindowManager &WindowManager::removeWindow(Window *aWindow)
{
  if (aWindow == _currentWindow())
  {
    if (_backgroundWindows.empty())
    {
      _currentWindow = WindowRec(NULL, NULL);
    }
    else
    {
      _currentWindow = _backgroundWindows.back();
      _backgroundWindows.pop_back();
      setFocusedWindow(_currentWindow._focused);
    }
  }
  else
  {
    for (Windows::iterator it = _backgroundWindows.begin(); it != _backgroundWindows.end(); ++it)
    {
      if ((*it)() == aWindow)
      {
        _backgroundWindows.erase(it);
        break;
      }
    }
  }
  if (focusedWindow() && aWindow->doesHierarchyContain(focusedWindow()))
  {
    _focusedWindow = _currentWindow();
  }
  updateBackground();
  return *this;
}

const WindowManager &WindowManager::update(void)
{
#if 0
    for (Windows::iterator it=_backgroundWindows.begin();it!=_backgroundWindows.end();++it)
    {
      dbg_printf("background (%s)\n",(*it)()->text().c_str());
    }
    dbg_printf("currentWindow (%s)\n",_currentWindow()?_currentWindow()->text().c_str():"NULL");
#endif
  if (_currentWindow())
  {
    _currentWindow()->update();
    _currentWindow()->render();
  }
  return *this;
}

const WindowManager &WindowManager::updateBackground(void)
{
  gdi().setMainEngineLayer(MEL_DOWN);
  for (Windows::iterator it = _backgroundWindows.begin(); it != _backgroundWindows.end(); ++it)
  {
    (*it)()->update();
  }
  for (Windows::iterator it = _backgroundWindows.begin(); it != _backgroundWindows.end(); ++it)
  {
    (*it)()->render();
  }
  gdi().setMainEngineLayer(MEL_UP);
  update();
  gdi().present();
  return *this;
}

bool WindowManager::process(Message &message) const
{
  return _currentWindow()->process(message);
}

WindowManager &WindowManager::checkForWindowBelowPen(const Point &touchPoint)
{
  _windowBelowPen = NULL;
  if (_currentWindow()->isVisible())
  {
    Window *wbp = _currentWindow()->windowBelow(touchPoint);
    if (wbp)
      _windowBelowPen = wbp;
  }
  return *this;
}

void WindowManager::updateFocusIfNecessary(const Point &touchPoint)
{
  Window *wbp = windowBelowPen();
  if (wbp != focusedWindow())
    setFocusedWindow(wbp);
  if (wbp && !wbp->isFocusable())
    _capturedWindow = wbp;
}

bool WindowManager::processTouchMessage(TouchMessage &message)
{
  bool isHandled = false;
  if (windowBelowPen())
  {
    isHandled = windowBelowPen()->process(message);
    //dbg_printf("windowBelowPen() %s process touch\n",windowBelowPen()->text().c_str());
  }
  else
  {
    isHandled = process(message);
    //dbg_printf("default process touch\n");
  }
  return isHandled;
}

bool WindowManager::onKeyDown(unsigned char keyCode, unsigned char shift)
{
  KeyMessage msg(Message::keyDown, keyCode, shift);
  return process(msg);
}

bool WindowManager::onKeyUp(unsigned char keyCode, unsigned char shift)
{
  KeyMessage msg(Message::keyUp, keyCode, shift);
  return process(msg);
}

bool WindowManager::onTouchDown(int x, int y)
{
  _capturedWindow = NULL;
  checkForWindowBelowPen(Point(x, y));
  TouchMessage msg(Message::touchDown, Point(x, y));
  bool isHandled = processTouchMessage(msg);
  updateFocusIfNecessary(Point(x, y));
  return isHandled;
}

bool WindowManager::onTouchUp(int x, int y)
{
  checkForWindowBelowPen(Point(x, y));
  TouchMessage msg(Message::touchUp, Point(x, y));
  dbg_printf("window below pen (%s)\n", _windowBelowPen ? _windowBelowPen->text().c_str() : "NULL");
  dbg_printf("focusedWindow (%s)\n", focusedWindow() ? focusedWindow()->text().c_str() : "NULL");
  if (_capturedWindow)
  {
    bool result = _capturedWindow->process(msg);
    _capturedWindow = NULL;
    return result;
  }
  if (focusedWindow() && windowBelowPen() != focusedWindow())
    return focusedWindow()->process(msg);
  return processTouchMessage(msg);
}

bool WindowManager::onTouchMove(int x, int y)
{
  const INPUT &input = getInput();
  TouchMessage msg(Message::touchMove, Point(x, y));
  checkForWindowBelowPen(Point(input.touchPt.px, input.touchPt.py));
  dbg_printf("touch move %d %d\n", x, y);
  return processTouchMessage(msg);
}
} // namespace akui
