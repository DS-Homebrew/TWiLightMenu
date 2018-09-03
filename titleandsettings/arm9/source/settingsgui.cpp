#include "settingsgui.h"

#include "graphics/fontHandler.h"
#include <variant>

void SettingsGUI::draw()
{
    printLarge(false, 6, 1, _pages[_selectedPage].title().c_str());
    for (int i = 0; i < _pages[_selectedPage].options().size(); i++)
    {
        int selected = _pages[_selectedPage].options()[i].selected();

        if (i == _selectedOption)
            printSmall(false, 4, 29 + i * 14, ">");

        printSmall(false, 12, 30 + i * 14, _pages[_selectedPage].options()[i].displayName().c_str());
        printSmall(false, 194, 30 + i * 14, _pages[_selectedPage].options()[i].labels()[selected].c_str());
    }
}

void SettingsGUI::incrementOption()
{
    _selectedOption = (1 + _selectedOption) % (_pages[_selectedPage].options().size());
    clearText();
}

void SettingsGUI::decrementOption()
{
    _selectedOption = (-1 + _selectedOption) % (_pages[_selectedPage].options().size());
    clearText();
}

void SettingsGUI::setOptionNext()
{
    auto selectedOption = _pages[_selectedPage].options()[_selectedOption];
    int currentValueIndex = selectedOption.selected();
    int nextValueIndex = (currentValueIndex + 1) % (selectedOption.values().size());
    if (currentValueIndex == -1) nextValueIndex = 0;
    auto nextValue = selectedOption.values()[nextValueIndex];

    if (auto action = std::get_if<Option::Bool>(&selectedOption.action()))
    {
        action->set(*std::get_if<bool>(&nextValue));
    }

    if (auto action = std::get_if<Option::Int>(&selectedOption.action()))
    {
        action->set(*std::get_if<int>(&nextValue));
    }

    if (auto action = std::get_if<Option::Str>(&selectedOption.action()))
    {
        action->set(*std::get_if<cstr>(&nextValue));
    }
    clearText();
}