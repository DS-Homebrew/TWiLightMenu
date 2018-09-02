/*
    settingwnd.h
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

#ifndef _SETTINGWND_H_
#define _SETTINGWND_H_

#include "ui/form.h"
#include "ui/formdesc.h"
#include "ui/spinbox.h"
#include "ui/statictext.h"
#include "ui/message.h"
#include <string>

class SettingWnd : public akui::Form
{
public:
  SettingWnd(s32 x, s32 y, u32 w, u32 h, Window *parent, const std::string &text);
  ~SettingWnd();

public:
  void draw(void);
  bool process(const akui::Message &msg);
  Window &loadAppearance(const std::string &aFileName);
  void addSettingTab(const std::string &text);
  void addSettingItem(const std::string &text, const std::vector<std::string> &itemTexts, size_t defaultValue);
  ssize_t getItemSelection(size_t tabId, size_t itemId);
  void setConfirmMessage(const std::string &text);
  template <class TargetType>
  void addYButton(std::string title, TargetType *slotHolder, void (TargetType::*aMemberFunction)())
  {
    _buttonY.setText("\x04 " + title);
    _buttonY.clicked.connect(slotHolder, aMemberFunction);
    _buttonY.show();
  }

protected:
  struct sSetingItem
  {
    akui::StaticText *_label;
    akui::SpinBox *_item;
    sSetingItem(akui::StaticText *label, akui::SpinBox *item) : _label(label), _item(item){};
  };
  struct sSetingTab
  {
    std::vector<sSetingItem> *_tab;
    std::string _title;
    sSetingTab(std::vector<sSetingItem> *tab, const std::string &title) : _tab(tab), _title(title){};
  };

protected:
  void onOK(void);
  void onCancel(void);
  void onShow(void);
  void onUIKeyUP(void);
  void onUIKeyDOWN(void);
  void onUIKeyLEFT(void);
  void onUIKeyRIGHT(void);
  void onUIKeyL(void);
  void onUIKeyR(void);
  bool processKeyMessage(const akui::KeyMessage &msg);
  void onItemChanged(akui::SpinBox *item);
  ssize_t focusedItemId(void);
  akui::SpinBox *focusedItem(void);

  void HideTab(size_t index);
  void ShowTab(size_t index);
  void SwitchTab(size_t oldIndex, size_t newIndex);

  std::vector<sSetingItem> &items(size_t index) { return *_tabs[index]._tab; };

  u32 _maxLabelLength;
  u32 _spinBoxWidth;
  bool _simpleTabs;
  std::vector<sSetingTab> _tabs;
  size_t _currentTab;
  size_t _maxTabSize;
  std::string _confirmMessage;

  akui::SpinBox _tabSwitcher;
  akui::Button _buttonOK;
  akui::Button _buttonCancel;
  akui::FormDesc _renderDesc;
  akui::Button _buttonY;
};

#endif //_SETTINGWND_H_
