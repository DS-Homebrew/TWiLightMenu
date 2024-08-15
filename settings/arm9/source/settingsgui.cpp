#include "settingsgui.h"

#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "common/bootstrapsettings.h"
#include "common/twlmenusettings.h"
#include <variant>
#include <algorithm>
#include <nds.h>
#include <maxmod9.h>
#include "soundeffect.h"
#include "language.h"
#include "gbarunner2settings.h"
#include "version.h"

// Screen offsets for scrollbar
#define CURSOR_MIN 30
#define CURSOR_MAX (SCREEN_HEIGHT - 40)
#define CURSOR_HEIGHT (CURSOR_MAX - CURSOR_MIN)

extern bool fadeType; // false = out, true = in
extern int currentTheme;
extern bool currentMacroMode;
extern bool lcdSwapped;
extern int titleDisplayLength;

void SettingsGUI::processInputs(int pressed, touchPosition &touch)
{
	if (currentMacroMode && ((pressed & KEY_SELECT) || (lcdSwapped && (pressed & KEY_B)))) {
		mmEffectEx(currentTheme == 4 ? &snd().snd_saturn_launch : &snd().snd_switch);
		fadeType = false;
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		lcdSwap();
		lcdSwapped = !lcdSwapped;
		fadeType = true;
		return;
	}

	if (!lcdSwapped) {
		if ((pressed & KEY_B || pressed & KEY_A) && inSub()) {
			mmEffectEx(currentTheme == 4 ? &snd().snd_saturn_back : &snd().snd_select);
			exitSub();
			return;
		} else if ((pressed & KEY_B && !inSub()) || ((pressed & KEY_TOUCH && touch.py >= 170) && !inSub())) {
			saveAndExit(currentTheme);
		}

		if (pressed & KEY_RIGHT) {
			mmEffectEx(currentTheme == 4 ? &snd().snd_saturn_select : &snd().snd_select);
			setOptionNext();
		}

		if (pressed & KEY_LEFT) {
			mmEffectEx(currentTheme == 4 ? &snd().snd_saturn_select : &snd().snd_select);
			setOptionPrev();
		}

		if (pressed & KEY_DOWN) {
			mmEffectEx(currentTheme == 4 ? &snd().snd_saturn_select : &snd().snd_select);
			incrementOption();
		}

		if (pressed & KEY_UP) {
			mmEffectEx(currentTheme == 4 ? &snd().snd_saturn_select : &snd().snd_select);
			decrementOption();
		}

		if ((pressed & KEY_X || pressed & KEY_R) && !inSub()) {
			mm_sound_effect pannedSound = currentTheme == 4 ? snd().snd_saturn_launch : snd().snd_switch;
			pannedSound.panning = 178;

			mmEffectEx(&pannedSound);
			titleDisplayLength = 0;
			rotatePage(1);
		}

		if ((pressed & KEY_Y || pressed & KEY_L) && !inSub()) {
			mm_sound_effect pannedSound = currentTheme == 4 ? snd().snd_saturn_launch : snd().snd_switch;
			pannedSound.panning = 76;

			mmEffectEx(&pannedSound);
			titleDisplayLength = 0;
			rotatePage(-1);
		}

		if (pressed & KEY_A && !inSub()) {
			auto opt = _pages[_selectedPage].options()[_selectedOption];
			if (opt.action_sub().has_sub()) {
				if (opt.action_sub().sub()) {
					mmEffectEx(currentTheme == 4 ? &snd().snd_saturn_launch : &snd().snd_select);
					titleDisplayLength = 0;
					enterSub();
				} else {
					mmEffectEx(&snd().snd_wrong);
				}
			} else {
				setOptionNext();
			}
		}
	}
}

static bool wasOptionModified(Option& option) {
	if (auto action = std::get_if<Option::Bool>(&option.action())) {
		return action->was_modified();
	}
	if (auto action = std::get_if<Option::Int>(&option.action())) {
		return action->was_modified();
	}
	if (auto action = std::get_if<Option::Str>(&option.action())) {
		return action->was_modified();
	}
	if (auto action = std::get_if<Option::Nul>(&option.action())) {
		return action->was_modified();
	}
	return false;
}

void SettingsGUI::draw()
{
	if (_selectedPage < 0 || (int)_pages.size() < 1 || _selectedPage >= (int)_pages.size())
		return;

	if (inSub()) {
		drawSub();
		return;
	}

	/*if (_isSaved) {
		clearText();
		printSmall(false, 4, 2, "Settings saved.");
		return;
	} else if (_isExited) {
		clearText();
		printSmall(false, 4, 2, "Saving settings...");
		return;
	}*/

	clearText();
	drawTopText();

	// If the language is set to RTL, mirror everything
	if (ms().rtl()) {
		printLarge(false, SCREEN_WIDTH - 6, 1, titleDisplayLength>=60*4 ? STR_SELECT_SEE_DESC_VER : _pages[_selectedPage].title().c_str(), Alignment::right);

		std::string str;
		for (int i = _topCursor; i < _bottomCursor; i++) {
			auto option = _pages[_selectedPage].options()[i];
			int selected = option.selected();

			if (i == _selectedOption) {
				if (currentTheme == 4) {
					printSmall(false, SCREEN_WIDTH - 4, 29 + (i - _topCursor) * 14, "<", Alignment::right);
				}
				// print scroller on the other side
				drawScroller(30 + i * CURSOR_HEIGHT / _pages[_selectedPage].options().size() + 1, (CURSOR_HEIGHT / _pages[_selectedPage].options().size() + 1), true);
			}

			bool modified = wasOptionModified(option);

			printSmall(false, SCREEN_WIDTH - 12, 30 + (i - _topCursor) * 14, _pages[_selectedPage].options()[i].displayName().c_str(), Alignment::right, (i == _selectedOption && currentTheme != 4) ? FontPalette::user : FontPalette::regular);
			if (selected < 0) continue;

			str = _pages[_selectedPage].options()[i].labels()[selected];
			if (modified)
				str += "*";
			printSmall(false, 12, 30 + (i - _topCursor) * 14, str.c_str(), Alignment::left, (i == _selectedOption && currentTheme != 4) ? FontPalette::user : FontPalette::regular);
		}
	} else {
		printLarge(false, 6, 1, titleDisplayLength>=60*4 ? STR_SELECT_SEE_DESC_VER : _pages[_selectedPage].title().c_str());

		std::string str;
		for (int i = _topCursor; i < _bottomCursor; i++) {
			auto option = _pages[_selectedPage].options()[i];
			int selected = option.selected();

			if (i == _selectedOption) {
				if (currentTheme == 4) {
					printSmall(false, 4, 29 + (i - _topCursor) * 14, ">");
				}
				// print scroller on the other side
				drawScroller(30 + i * CURSOR_HEIGHT / _pages[_selectedPage].options().size() + 1, (CURSOR_HEIGHT / _pages[_selectedPage].options().size() + 1), false);
			}

			bool modified = wasOptionModified(option);

			printSmall(false, 12, 30 + (i - _topCursor) * 14, _pages[_selectedPage].options()[i].displayName().c_str(), Alignment::left, (i == _selectedOption && currentTheme != 4) ? FontPalette::user : FontPalette::regular);
			if (selected < 0) continue;

			str = _pages[_selectedPage].options()[i].labels()[selected];
			if (modified)
				str += "*";

			printSmall(false, SCREEN_WIDTH - 12, 30 + (i - _topCursor) * 14, str.c_str(), Alignment::right, (i == _selectedOption && currentTheme != 4) ? FontPalette::user : FontPalette::regular);
		}
	}

	// Divide CURSOR_HEIGHT into _subOption->values().size() pieces and get the ith piece.
	// Integer division is good enough for this case.
	// int scrollSections = CURSOR_HEIGHT / _pages[_selectedPage].options().size() + 1;
	// // Print a nice thick scroller.
	// printSmall(false, 252, (scrollSections * (_selectedOption)) + CURSOR_MIN, "|");
	// printSmall(false, 254, (scrollSections * (_selectedOption)) + CURSOR_MIN, "|");

	printSmall(false, 0, 173, "TWiLight Menu++", Alignment::center);
	printSmall(false, 2, 173, "<  / "); // L / Y
	printSmall(false, 256 - 2, 173, " /  >", Alignment::right); // R / X

	updateText(false);
	updateText(true);
}

void SettingsGUI::setTopText(const std::string &text)
{
	_topText.clear();
	_topTextLines = 0;
	std::u16string _topTextStr(FontGraphic::utf8to16(text));
	std::vector<std::u16string> words;
	std::size_t pos;

	// Process comment to stay within the box
	while ((pos = _topTextStr.find(' ')) != std::string::npos) {
		words.push_back(_topTextStr.substr(0, pos));
		_topTextStr = _topTextStr.substr(pos + 1);
	}
	if (_topTextStr.size())
		words.push_back(_topTextStr);
	std::u16string temp;
	for (auto word : words) {
		// Split word if the word is too long for a line
		int width = calcSmallFontWidth(word);
		if (width > 240) {
			if (temp.length()) {
				_topText += temp + u"\n";
				_topTextLines++;
				temp = u"";
			}
			for (int i = 0; i < width/240.0; i++) {
				word.insert((float)((i + 1) * word.length()) / ((width/240) + 1), u"\n");
			}
			_topText += word + u"\n";
			_topTextLines++;
			continue;
		}

		width = calcSmallFontWidth(temp + u" " + word);
		if (width > 240) {
			_topText += temp + u"\n";
			_topTextLines++;
			temp = word;
		} else {
			temp += u" " + word;
		}
	}
	if (temp.size()) {
		_topText += temp;
		_topTextLines++;
	}
	
	// Ensure there are no newlines at the beggining / end
	while (_topText[0] == '\n')
		_topText = _topText.substr(1);
	while (_topText[_topText.length() - 1] == '\n')
		_topText = _topText.substr(0, _topText.length() - 2);
}

void SettingsGUI::drawSub()
{
	clearText();
	drawTopText();
	int selected = _subOption->selected();

	if (ms().rtl()) {
		printLarge(false, SCREEN_WIDTH - 6, 1, titleDisplayLength>=60*4 ? STR_SELECT_SEE_DESC_VER : _subOption->displayName().c_str(), Alignment::right);
		for (int i = _subTopCursor; i < _subBottomCursor; i++) {
			if (i == selected) {
				if (currentTheme == 4) {
					printSmall(false, SCREEN_WIDTH - 4, 29 + (i - _subTopCursor) * 14, "<", Alignment::right);
				}

				// print scroller on the other side
				drawScroller(30 + i * CURSOR_HEIGHT / _subOption->labels().size() + 1, (CURSOR_HEIGHT / _subOption->labels().size() + 1), true);
			}

			printSmall(false, SCREEN_WIDTH - 12, 30 + (i - _subTopCursor) * 14, _subOption->labels()[i].c_str(), Alignment::right, (i == selected && currentTheme != 4) ? FontPalette::user : FontPalette::regular);
		}
	} else {
		printLarge(false, 6, 1, titleDisplayLength>=60*4 ? STR_SELECT_SEE_DESC_VER : _subOption->displayName().c_str());
		for (int i = _subTopCursor; i < _subBottomCursor; i++) {
			if (i == selected) {
				if (currentTheme == 4) {
					printSmall(false, 4, 29 + (i - _subTopCursor) * 14, ">");
				}

				// print scroller on the other side
				drawScroller(30 + i * CURSOR_HEIGHT / _subOption->labels().size() + 1, (CURSOR_HEIGHT / _subOption->labels().size() + 1), false);
			}

			printSmall(false, 12, 30 + (i - _subTopCursor) * 14, _subOption->labels()[i].c_str(), Alignment::left, (i == selected && currentTheme != 4) ? FontPalette::user : FontPalette::regular);
		}
	}

	printSmall(false, 0, 173, "TWiLight Menu++", Alignment::center);

	updateText(false);
	updateText(true);
}

void SettingsGUI::drawTopText()
{
	if (ms().rtl()) {
		printSmall(true, SCREEN_WIDTH - 4, 0, titleDisplayLength>=60*4 ? STR_SELECT_SETTING_LIST : STR_NDS_BOOTSTRAP_VER + " " + bsVerText[ms().bootstrapFile], Alignment::right);
	} else {
		printSmall(true, 4, 0, titleDisplayLength>=60*4 ? STR_SELECT_SETTING_LIST : STR_NDS_BOOTSTRAP_VER + " " + bsVerText[ms().bootstrapFile]);
	}
	if (_topTextLines < 6 || currentTheme == 4)
		printSmall(true, 256 - 4, 174, VER_NUMBER, Alignment::right);
	printSmall(true, 0, (currentTheme == 4 ? 96 : 138) - (calcSmallFontHeight(_topText) / 2), _topText, Alignment::center);
}

void SettingsGUI::rotatePage(int rotateAmount)
{
	// int pageIndex = (_selectedPage + rotateAmount) % (_pages.size());
	int pageIndex = (_selectedPage + rotateAmount);
	if (pageIndex < 0) pageIndex = _pages.size()-1;
	else if (pageIndex > ((int)_pages.size()) - 1) pageIndex = 0;
	_selectedPage = pageIndex;
	_bottomCursor = std::min<int>(_pages[_selectedPage].options().size(), MAX_ELEMENTS);
	_topCursor = 0;
	_topText.clear();
	_selectedOption = 0;
	rotateOption(0);
}

void SettingsGUI::rotateOptionValue(int rotateAmount)
{
	// Only the main menu pages have left-right option values. Sub menus only control one Option.
	if (inSub())
		return;
	if (_pages[_selectedPage].options().size() == 0) return;

	auto selectedOption = _pages[_selectedPage].options()[_selectedOption];
	int currentValueIndex = selectedOption.selected();
	int nextValueIndex = (currentValueIndex + rotateAmount + selectedOption.values().size()) % (selectedOption.values().size());
	if (currentValueIndex == -1)
		nextValueIndex = 0;
	auto nextValue = selectedOption.values()[nextValueIndex];

	if (auto action = std::get_if<Option::Bool>(&selectedOption.action())) {
		action->set(*std::get_if<bool>(&nextValue));
	}

	if (auto action = std::get_if<Option::Int>(&selectedOption.action())) {
		action->set(*std::get_if<int>(&nextValue));
	}

	if (auto action = std::get_if<Option::Str>(&selectedOption.action())) {
		action->set(*std::get_if<cstr>(&nextValue));
	}

	if (auto action = std::get_if<Option::Nul>(&selectedOption.action())) {
		action->set();
	}
	clearText();
}

void SettingsGUI::rotateOption(int rotateAmount)
{
	if (!inSub()) {
		// If we're not in the sub option menu, change the option.
		if ((_selectedOption + rotateAmount) < 0 || ((_selectedOption + rotateAmount) >= (int)_pages[_selectedPage].options().size())) {
			return;
		}

		if (_selectedOption + rotateAmount >= _bottomCursor) {
			_topCursor++;
			_bottomCursor++;
		}

		if (_selectedOption + rotateAmount < _topCursor) {
			_topCursor--;
			_bottomCursor--;
		}

		_selectedOption = (_selectedOption + rotateAmount) % (_pages[_selectedPage].options().size());
		setTopText(_pages[_selectedPage].options()[_selectedOption].longDescription());
	} else {
		if (_pages[_selectedPage].options().size() == 0) return;
		// Change the sub option instead.
		auto selectedOption = _pages[_selectedPage].options()[_selectedOption];
		auto &action = selectedOption.action_sub();
		if (action.sub()) {
			auto sub = *action.sub();
			int currentValueIndex = sub.selected();

			// Update cursors.
			if ((currentValueIndex + rotateAmount) < 0 || ((currentValueIndex + rotateAmount) >= (int)sub.values().size())) {
				// Prevent overflows...
				return;
			}

			if (currentValueIndex + rotateAmount >= _subBottomCursor) {
				_subTopCursor++;
				_subBottomCursor++;
			}

			if (currentValueIndex + rotateAmount < _subTopCursor) {
				_subTopCursor--;
				_subBottomCursor--;
			}

			int nextValueIndex = (currentValueIndex + rotateAmount) % (sub.values().size());
			if (currentValueIndex == -1) {
				nextValueIndex = 0;
			}
			auto nextValue = sub.values()[nextValueIndex];

			if (auto subaction = std::get_if<Option::Bool>(&sub.action())) {
				subaction->set(*std::get_if<bool>(&nextValue));
			}

			if (auto subaction = std::get_if<Option::Str>(&sub.action())) {
				subaction->set(*std::get_if<cstr>(&nextValue));
			}

			if (auto subaction = std::get_if<Option::Int>(&sub.action())) {
				int value = *std::get_if<int>(&nextValue);
				subaction->set(value);
			}
			if (auto subaction = std::get_if<Option::Nul>(&sub.action())) {
				subaction->set();
			}
		}
	}

	clearText();
}

void SettingsGUI::saveAndExit(int currentTheme)
{
	_isExited = true;
	mmEffectEx(currentTheme == 4 ? &snd().snd_saturn_back : &snd().snd_back);
	// Draw in between here.
	draw();

	fadeType = false;
	for (int i = 0; i < 25; i++) {
		swiWaitForVBlank();
	}
	ms().saveSettings();
	gs().saveSettings();
	bs().saveSettings();
	_isSaved = true;
	draw();
	for (int i = 0; i < 30; i++)
		swiWaitForVBlank();
	snd().stopBgMusic();

	if (_exitCallback != nullptr) {
		_exitCallback();
	}
}