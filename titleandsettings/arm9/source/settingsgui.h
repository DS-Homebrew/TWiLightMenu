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
        : _selectedPage(0), _selectedOption(0), _inSub(false), _selectedSub(0), _topCursor(0), 
        _bottomCursor(0) {}
    ~SettingsGUI() {}

  public:
    void draw();

    void drawSub(Option& subOption);

    void enterSub() { _inSub = true; }
    
    void exitSub() { _inSub = false; _selectedSub = 0; }
    bool inSub() { return _inSub; }

    SettingsGUI &addPage(SettingsPage &page);

    void incrementOption() { rotateOption(1); }
    void decrementOption() { rotateOption(-1); }
    
    void setOptionNext() { rotateOptionValue(1); }
    void setOptionPrev() { rotateOptionValue(-1); }

  private:
    void rotateOption(int rotateAmount);
    void rotateOptionValue(int rotateAmount);
    
    int _selectedPage;
    int _selectedOption;
    int _selectedSub;
    bool _inSub;
    int _topCursor;
    int _bottomCursor;
    std::vector<SettingsPage> _pages;
};

#endif