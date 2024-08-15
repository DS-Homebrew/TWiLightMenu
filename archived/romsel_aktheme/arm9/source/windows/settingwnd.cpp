/*
    settingwnd.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2009 somebody
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

#include "settingwnd.h"
#include "systemfilenames.h"
#include "ui/msgbox.h"
#include "ui/windowmanager.h"
#include "ui/uisettings.h"
#include "language.h"
#define TOP_MARGIN 4

using namespace akui;

SettingWnd::SettingWnd(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text)
    : Form(x, y, w, h, parent, text), _tabSwitcher(0, 0, w, 18, this, "spin"),
      _buttonOK(0, 0, 46, 18, this, "\x01 OK"), _buttonCancel(0, 0, 48, 18, this, "\x02 Cancel"),
      _buttonY(0, 0, 76, 18, this, "\x04 Common")
{
  _tabSwitcher.loadAppearance("");
  _tabSwitcher.changed.connect(this, &SettingWnd::onItemChanged);
  addChildWindow(&_tabSwitcher);
  _tabSwitcher.insertItem(_text, 0);
  _tabSwitcher.selectItem(0);
  _tabSwitcher.hide();
  _tabSwitcher.disableFocus();

  s16 buttonY = size().y - _buttonCancel.size().y - 4;

  _buttonCancel.setStyle(Button::press);
  _buttonCancel.setText("\x02 " + LANG("setting window", "cancel"));
  _buttonCancel.setTextColor(uis().buttonTextColor);
  _buttonCancel.loadAppearance(SFN_BUTTON3);
  _buttonCancel.clicked.connect(this, &SettingWnd::onCancel);
  addChildWindow(&_buttonCancel);

  _buttonOK.setStyle(Button::press);
  _buttonOK.setText("\x01 " + LANG("setting window", "ok"));
  _buttonOK.setTextColor(uis().buttonTextColor);
  _buttonOK.loadAppearance(SFN_BUTTON3);
  _buttonOK.clicked.connect(this, &SettingWnd::onOK);
  addChildWindow(&_buttonOK);

  _buttonY.setStyle(Button::press);
  _buttonY.setTextColor(uis().buttonTextColor);
  _buttonY.loadAppearance(SFN_BUTTON4);
  addChildWindow(&_buttonY);
  _buttonY.hide();
  _buttonY.disableFocus();

  s16 nextButtonX = size().x;
  s16 buttonPitch = _buttonCancel.size().x + 8;
  nextButtonX -= buttonPitch;
  _buttonCancel.setRelativePosition(Point(nextButtonX, buttonY));

  buttonPitch = _buttonOK.size().x + 8;
  nextButtonX -= buttonPitch;
  _buttonOK.setRelativePosition(Point(nextButtonX, buttonY));

  buttonPitch = _buttonY.size().x + 8;
  nextButtonX -= buttonPitch;
  _buttonY.setRelativePosition(Point(nextButtonX, buttonY));

  loadAppearance("");
  arrangeChildren();

  _tabs.push_back(sSetingTab(new std::vector<sSetingItem>, text));
  _currentTab = 0;
  _maxLabelLength = 0;
  CIniFile ini(SFN_UI_SETTINGS);
  _spinBoxWidth = ini.GetInt("setting window", "spinBoxWidth", 108);
  _simpleTabs = ini.GetInt("setting window", "simpleTabs", 0);
  ;
  _maxTabSize = 0;
  _confirmMessage = LANG("setting window", "confirm text");
}

void SettingWnd::setConfirmMessage(const std::string &text)
{
  _confirmMessage = text;
}

SettingWnd::~SettingWnd()
{
  for (size_t ii = 0; ii < _tabs.size(); ii++)
  {
    for (size_t jj = 0; jj < items(ii).size(); jj++)
    {
      delete items(ii)[jj]._label;
      delete items(ii)[jj]._item;
    }
    delete _tabs[ii]._tab;
  }
}

void SettingWnd::draw(void)
{
  _renderDesc.draw(windowRectangle(), _engine);
  Form::draw();
  swiWaitForVBlank();
}

bool SettingWnd::process(const akui::Message &msg)
{
  bool ret = false;
  ret = Form::process(msg);
  if (!ret)
  {
    if (msg.id() > Message::keyMessageStart && msg.id() < Message::keyMessageEnd)
    {
      ret = processKeyMessage((KeyMessage &)msg);
    }
    //if (msg.id()>cMessage::touchMessageStart&&msg.id()<Message::touchMessageEnd)
    //{
    //  ret=processTouchMessage((TouchMessage&)msg);
    //}
  }
  return ret;
}

void SettingWnd::onOK(void)
{
  u32 ret = messageBox(this, LANG("setting window", "confirm"), _confirmMessage, MB_OK | MB_CANCEL);
  if (ID_OK != ret)
    return;
  _modalRet = 1;
}

void SettingWnd::onCancel(void)
{
  _modalRet = 0;
}

bool SettingWnd::processKeyMessage(const KeyMessage &msg)
{
  bool ret = false;
  if (msg.id() == Message::keyDown)
  {
    switch (msg.keyCode())
    {
    case KeyMessage::UI_KEY_DOWN:
      onUIKeyDOWN();
      ret = true;
      break;
    case KeyMessage::UI_KEY_UP:
      onUIKeyUP();
      ret = true;
      break;
    case KeyMessage::UI_KEY_LEFT:
      onUIKeyLEFT();
      ret = true;
      break;
    case KeyMessage::UI_KEY_RIGHT:
      onUIKeyRIGHT();
      ret = true;
      break;
    case KeyMessage::UI_KEY_A:
      onOK();
      ret = true;
      break;
    case KeyMessage::UI_KEY_B:
      onCancel();
      ret = true;
      break;
    case KeyMessage::UI_KEY_Y:
    {
      _buttonY.clicked();
    }
      ret = true;
      break;
    case KeyMessage::UI_KEY_L:
      onUIKeyL();
      ret = true;
      break;
    case KeyMessage::UI_KEY_R:
      onUIKeyR();
      ret = true;
      break;
    default:
      break;
    }
  }
  return ret;
}

Window &SettingWnd::loadAppearance(const std::string &aFileName)
{
  _renderDesc.loadData(SFN_FORM_TITLE_L, SFN_FORM_TITLE_R, SFN_FORM_TITLE_M);
  _renderDesc.setTitleText(_text);
  return *this;
}

void SettingWnd::addSettingTab(const std::string &text)
{
  if (1 == _tabs.size())
  {
    if (_simpleTabs)
      _renderDesc.loadData("", "", "");
    _renderDesc.setTitleText("");
  }
  _tabSwitcher.insertItem(text, _tabs.size());
  _tabs.push_back(sSetingTab(new std::vector<sSetingItem>, text));
  _tabSwitcher.show();
}

void SettingWnd::addSettingItem(const std::string &text, const std::vector<std::string> &itemTexts, size_t defaultValue)
{
  if (0 == itemTexts.size())
    return;
  if (defaultValue >= itemTexts.size())
    defaultValue = 0;
  if (_maxLabelLength < text.length())
    _maxLabelLength = text.length();
  size_t lastTab = _tabs.size() - 1;

  //recompute window size and position
  if (_maxTabSize < (items(lastTab).size() + 1))
  {
    _maxTabSize = (items(lastTab).size() + 1);
    setSize(Size(_size.x, _maxTabSize * 20 + 41 + TOP_MARGIN));
    _position.x = (SCREEN_WIDTH - _size.x) / 2;
    _position.y = (SCREEN_HEIGHT - _size.y) / 2;
  }

  // insert label, item and set their position
  s32 itemY = items(lastTab).size() * 20 + 18 + TOP_MARGIN;
  s32 itemX = 8;

  SpinBox *item = new SpinBox(0, 0, 108, 18, this, "spin");
  for (size_t ii = 0; ii < itemTexts.size(); ++ii)
  {
    item->insertItem(itemTexts[ii], ii);
  }

  item->loadAppearance("");
  item->setSize(Size(_spinBoxWidth, 18));
  item->setRelativePosition(Point(_size.x - _spinBoxWidth - 4, itemY));
  item->hide();
  addChildWindow(item);
  item->selectItem(defaultValue);

  StaticText *label = new StaticText(0, 0, _maxLabelLength * 6, SYSTEM_FONT_HEIGHT, this, text);
  itemY += (item->windowRectangle().height() - label->windowRectangle().height()) / 2;
  label->setRelativePosition(Point(itemX, itemY));
  label->setTextColor(uis().formTextColor);
  label->setSize(Size(_size.x / 2 + 8, 12));
  label->hide();
  addChildWindow(label);

  items(lastTab).push_back(sSetingItem(label, item));

  // recompute button position
  s16 buttonY = size().y - _buttonCancel.size().y - 4;

  _buttonCancel.setRelativePosition(Point(_buttonCancel.relativePosition().x, buttonY));
  _buttonOK.setRelativePosition(Point(_buttonOK.relativePosition().x, buttonY));
  _buttonY.setRelativePosition(Point(_buttonY.relativePosition().x, buttonY));

  arrangeChildren();
}

void SettingWnd::onShow(void)
{
  ShowTab(_currentTab);
}

void SettingWnd::onUIKeyUP(void)
{
  ssize_t focusItem = focusedItemId();
  if (--focusItem < 0)
    focusItem = items(_currentTab).size() - 1;
  windowManager().setFocusedWindow(items(_currentTab)[focusItem]._item);
}

void SettingWnd::onUIKeyDOWN(void)
{
  ssize_t focusItem = focusedItemId();
  if (++focusItem >= (ssize_t)items(_currentTab).size())
    focusItem = 0;
  windowManager().setFocusedWindow(items(_currentTab)[focusItem]._item);
}

void SettingWnd::onUIKeyLEFT(void)
{
  SpinBox *item = focusedItem();
  if (item)
    item->selectPrev();
}

void SettingWnd::onUIKeyRIGHT(void)
{
  SpinBox *item = focusedItem();
  if (item)
    item->selectNext();
}

void SettingWnd::onUIKeyL(void)
{
  _tabSwitcher.selectPrev();
}

void SettingWnd::onUIKeyR(void)
{
  _tabSwitcher.selectNext();
}

ssize_t SettingWnd::getItemSelection(size_t tabId, size_t itemId)
{
  if (tabId >= _tabs.size())
    return -1;
  if (itemId >= items(tabId).size())
    return -1;
  return items(tabId)[itemId]._item->selectedItemId();
}

/*
0.. - item focused
-1 - something else focused
*/
ssize_t SettingWnd::focusedItemId(void)
{
  ssize_t focusItem = -1;
  for (size_t ii = 0; ii < items(_currentTab).size(); ++ii)
  {
    if (items(_currentTab)[ii]._item->isActive())
    {
      focusItem = ii;
      break;
    }
  }
  return focusItem;
}

SpinBox *SettingWnd::focusedItem(void)
{
  ssize_t focusItem = focusedItemId();
  if (focusItem >= 0)
    return items(_currentTab)[focusItem]._item;
  return NULL;
}

void SettingWnd::HideTab(size_t index)
{
  if (index >= _tabs.size())
    return;
  for (size_t ii = 0; ii < items(index).size(); ++ii)
  {
    items(index)[ii]._label->hide();
    items(index)[ii]._item->hide();
  }
}

void SettingWnd::ShowTab(size_t index)
{
  if (index >= _tabs.size())
    return;
  for (size_t ii = 0; ii < items(index).size(); ++ii)
  {
    items(index)[ii]._label->show();
    items(index)[ii]._item->show();
  }
  if (items(index).size())
    windowManager().setFocusedWindow(items(index)[0]._item);
}

void SettingWnd::SwitchTab(size_t oldIndex, size_t newIndex)
{
  HideTab(oldIndex);
  ShowTab(newIndex);
}

void SettingWnd::onItemChanged(akui::SpinBox *item)
{
  size_t newTab = item->selectedItemId();
  SwitchTab(_currentTab, newTab);
  _currentTab = newTab;
}
