#pragma once
#ifndef _DSIMENUPP_SETTINGS_GUI_H_
#define _DSIMENUPP_SETTINGS_GUI_H_
#include <vector>
#include "settingspage.h"
#include "graphics/fontHandler.h"

class SettingsGUI
{
  public:
    SettingsGUI()
        : _selectedPage(0), _selectedOption(0), _inSub(false) {}
    ~SettingsGUI() {}

  public:
    void draw();

    void drawSub(Option& subOption);

    void enableSub() { _inSub = true; }
    void disableSub() { _inSub = false; }
    bool inSub() { return _inSub; }

    SettingsGUI &addPage(SettingsPage &page)
    {
        _pages.emplace_back(std::move(page));
        return *this;
    }

    void incrementOption();
    void decrementOption();
    
    void setOptionNext();

  private:
    int _selectedPage;
    int _selectedOption;
    bool _inSub;
    std::vector<SettingsPage> _pages;
};

#endif