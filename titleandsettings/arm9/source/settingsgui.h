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
        _bottomCursor(0), _subOption(nullptr), _subBottomCursor(0), _subTopCursor(0) {}
  ~SettingsGUI() {}

public:
  void setTopText(const std::string &text)
  {
    std::string _topTextStr(text);
    std::size_t pos;
    _topText.clear();
    while ((pos = _topTextStr.find('\n')) != std::string::npos)
    {
      _topText.push_back(_topTextStr.substr(0, pos));
      _topTextStr = _topTextStr.substr(pos + 1);
    }
    _topText.push_back(_topTextStr);
  }

  void draw();

  void drawSub();

  void drawTopText();

  void enterSub()
  {
    if (_inSub)
      return;
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

      setTopText(_subOption->longDescription());

      int selectedIndex = _subOption->selected();
      int valueCount = _subOption->values().size();

      // Establish a base for the cursors.
      _subBottomCursor = std::min<int>(valueCount, MAX_ELEMENTS);
      _subTopCursor = 0;

      // Shift cursors by the difference between the selected value
      // such that the selected value, if above MAX_ELEMENTS, is at the bottom.

      // Cursors are at most the size of the options vector.
      if (selectedIndex >= MAX_ELEMENTS)
      {
        _subTopCursor = (selectedIndex - MAX_ELEMENTS) + 1;
        _subBottomCursor = selectedIndex + 1;
      }

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
    rotateOption(0);
  }
  bool inSub() { return _inSub; }

  void incrementOption() { rotateOption(1); }
  void decrementOption() { rotateOption(-1); }

  void setOptionNext() { rotateOptionValue(1); }
  void setOptionPrev() { rotateOptionValue(-1); }

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
    rotateOption(0);
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

  std::unique_ptr<Option> _subOption;
  std::vector<SettingsPage> _pages;
  std::vector<std::string> _topText;
};

#endif