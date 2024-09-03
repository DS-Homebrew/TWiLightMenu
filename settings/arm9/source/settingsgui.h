#pragma once
#ifndef _DSIMENUPP_SETTINGS_GUI_H_
#define _DSIMENUPP_SETTINGS_GUI_H_
#include <vector>
#include <algorithm>
#include "settingspage.h"
#include "graphics/fontHandler.h"
#include "common/singleton.h"

#define MAX_ELEMENTS 9

class SettingsGUI
{
public:
  SettingsGUI()
      : _selectedPage(0), _selectedOption(0),_topCursor(0),_bottomCursor(0),
        _selectedSub(0), _inSub(false),
        _subTopCursor(0), _subBottomCursor(0), 
        _isExited(false), _isSaved(false),
        _subOption(nullptr),
        _exitCallback(nullptr)
  {
    // Read nds-bootstrap version
    FILE* bsVerFile;
    for (int i = 0; i < 2; i++) {
      if (i == 1) {
        bsVerFile = fopen("/_nds/nightly-bootstrap.ver", "rb");
      } else {
        bsVerFile = fopen("/_nds/release-bootstrap.ver", "rb");
      }
      if (bsVerFile) {
		uint len = 7;
		if (i == 0) {
			fseek(bsVerFile, 0, SEEK_END);
			len = ftell(bsVerFile);
			fseek(bsVerFile, 0, SEEK_SET);
		}
        fread(bsVerText[i], 1, (i == 1) ? len : std::max(len, sizeof(bsVerText[i]) - 1), bsVerFile);
      } else {
        snprintf(bsVerText[i], sizeof(bsVerText[i]), "%s", "No version available");
      }
      fclose(bsVerFile);
    }
  }
  ~SettingsGUI() {}

public:
  /**
   * Processes inputs and modifies the state of the GUI
   */
  void processInputs(int inputs, touchPosition& touch);

  /**
   * Refreshes the text content of the GUI and re-draws
   * the scene.
   */
  void draw();

  /**
   * If we have exited settings and are waiting to
   * reboot into DSiMenu++
   */
  bool isExited() { return _isExited; }

  /**
   * Saves settings and prepares to exit the GUI.
   * This function writes settings to file and
   * freezes the GUI, but does not actually
   * reboot. To reboot, set the exit callback
   * using onExit.
   */
  void saveAndExit(int currentTheme);

private:
  /**
   * Sets the text at the top screen to
   * be printed next call of draw.
   */
  void setTopText(const std::string &text);

  void drawSub();

  void drawTopText();

  /**
   * Enters the sub option mode, and will draw a sub option next call of draw if one
   * exists.
   */
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
      // upon exit of this function, we move it into scope
      // to prevent it from being dropped after exit.
      _subOption = std::move(action.sub());

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

  /**
   * Exits the sub option mode and returns to the previous page.
   */
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
  /**
   * Scrolls down the option list by one.
   */
  void incrementOption() { rotateOption(1); }

  /**
   * Scroll up the option list by one.
   */
  void decrementOption() { rotateOption(-1); }

  /**
   * Moves the value of currently selected option right.
   */
  void setOptionNext() { rotateOptionValue(1); }

  /**
   * Moves the value of the currently selected option left.
   */
  void setOptionPrev() { rotateOptionValue(-1); }

public:
  /**
   * Adds a settings page to the GUI.
   */
  SettingsGUI &addPage(SettingsPage &page)
  {
    _pages.emplace_back(std::move(page));
    return *this;
  }

  /**
   * After adding all the pages, set the initial cursors,
   * and other requirements to show page 0.
   */
  void show()
  {
    rotatePage(0);
  }

  /**
   * Sets the callback to be called when the 
   * settings GUI exits.
   * 
   * Do reboots, and any necessary cleanup here.
   */
  SettingsGUI &onExit(std::function<void(void)> callback)
  {
    _exitCallback = callback;
    return *this;
  }

private:
  /*
  * Sets the currently selected option to the next nth option.
  * Generally should be used with -1, or 1, or 0 to
  * refresh the current state, usually after a page switch.
  */
  void rotateOption(int rotateAmount);

  /*
  * Sets the value of the currently selected option 
  * to the next nth value.
  * Generally should be used with -1, or 1, or 0 to
  * refresh the current state, usually after a page switch.
  */
  void rotateOptionValue(int rotateAmount);

  /*
  * Sets the current page to the next nth option.
  * Generally should be used with -1, or 1, or 0 to
  * refresh the current state, usually once on boot.
  */
  void rotatePage(int rotateAmount);

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
  std::u16string _topText;
  int _topTextLines;

  std::function<void(void)> _exitCallback;

  char bsVerText[2][25] = {{0}, {0}};
};

typedef singleton<SettingsGUI> settingsGui_s;
inline SettingsGUI &gui() { return settingsGui_s::instance(); }

#endif