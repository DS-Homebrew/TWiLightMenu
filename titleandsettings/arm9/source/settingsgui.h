#pragma once
#ifndef _DSIMENUPP_SETTINGS_GUI_H_
#define _DSIMENUPP_SETTINGS_GUI_H_
#include <vector>
#include <algorithm>
#include "settingspage.h"
#include "graphics/fontHandler.h"
#define MAX_ELEMENTS 9

class SettingsGUI
{
public:
  SettingsGUI()
      : _selectedPage(-1), _selectedOption(0), _inSub(false), _selectedSub(0), _topCursor(0),
        _bottomCursor(0), _subOption(nullptr), _subBottomCursor(0), _subTopCursor(0), _isExited(false), _isSaved(false) {}
  ~SettingsGUI() {}

public:
  void draw();

  void drawSub();

  void enterSub()
  {
    if (_inSub) return;
    auto selectedOption = _pages[_selectedPage].options()[_selectedOption];
    auto &action = selectedOption.action_sub();
    if (action.sub())
    {
      _inSub = true;
      
      // Since the value of Option::sub will be dropped
      // upon exit of this function, we make a copy of it
      // as a unique_ptr, to ensure that when we replace it,
      // it drops the old value as well.
      _subOption = std::make_unique<Option>(*action.sub());

      _subTopCursor = 0;
      _subBottomCursor = std::min<int>(_subOption->values().size(), MAX_ELEMENTS);
      return;
    }
    _inSub = false;
  }

  void exitSub()
  {
    _inSub = false;
    _selectedSub = 0;
    _subOption = nullptr;
    _subTopCursor = 0;
    _subBottomCursor = 0;
  }
  bool inSub() { return _inSub; }

  void incrementOption() { rotateOption(1); }
  void decrementOption() { rotateOption(-1); }

  void setOptionNext() { rotateOptionValue(1); }
  void setOptionPrev() { rotateOptionValue(-1); }

  void saveAndExit()
public:
  SettingsGUI &addPage(SettingsPage &page)
  {
    _pages.emplace_back(std::move(page));
    return *this;
  }

  SettingsGUI &setPage(int pageIndex)
  {
    _selectedPage = pageIndex;
    _bottomCursor = std::min<int>(_pages[_selectedPage].options().size(), MAX_ELEMENTS);
    _topCursor = 0;
    return *this;
  }

private:
  void rotateOption(int rotateAmount);
  void rotateOptionValue(int rotateAmount);

  int _selectedPage;
  int _selectedOption;

  // Cursors are used to keep track of scroll positions.
  int _topCursor;
  int _bottomCursor;

  int _selectedSub;
  bool _inSub;

  int _subTopCursor;
  int _subBottomCursor;  

  bool _isExited;
  bool _isSaved;
  std::unique_ptr<Option> _subOption;
  std::vector<SettingsPage> _pages;
};

#endif