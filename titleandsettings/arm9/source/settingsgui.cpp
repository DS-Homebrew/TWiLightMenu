#include "settingsgui.h"

#include "graphics/fontHandler.h"
#include <variant>

void SettingsGUI::draw()
{
    auto selectedOption = _pages[_selectedPage].options()[_selectedOption];
    if (inSub())
    {
        auto &action = selectedOption.action_sub();
        if (action.sub())
        {
            drawSub(*action.sub());
            return;
        }
    }

    clearText();
    exitSub();
    printLarge(false, 6, 1, _pages[_selectedPage].title().c_str());
    for (int i = 0; i < _pages[_selectedPage].options().size(); i++)
    {
        int selected = _pages[_selectedPage].options()[i].selected();

        if (i == _selectedOption)
            printSmall(false, 4, 29 + i * 14, ">");

        printSmall(false, 12, 30 + i * 14, _pages[_selectedPage].options()[i].displayName().c_str());
        printSmall(false, 194, 30 + i * 14, _pages[_selectedPage].options()[i].labels()[selected].c_str());
    }
    printSmallCentered(false, 173, "DSiMenu++");
}

void SettingsGUI::drawSub(Option &option)
{
    clearText();
    printLarge(false, 6, 1, option.displayName().c_str());
    int selected = option.selected();

    for (int i = 0; i < option.values().size(); i++)
    {
        if (i == selected)
            printSmall(false, 4, 29 + i * 14, ">");
        printSmall(false, 12, 30 + i * 14, option.labels()[i].c_str());
    }
}

void SettingsGUI::rotateOptionValue(int rotateAmount)
{
    // Only the main menu pages have left-right option values. Sub menus only control one Option.
    if (inSub()) return;

    auto selectedOption = _pages[_selectedPage].options()[_selectedOption];
    int currentValueIndex = selectedOption.selected();
    int nextValueIndex = (currentValueIndex + rotateAmount) % (selectedOption.values().size());
    if (currentValueIndex == -1)
        nextValueIndex = 0;
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


void SettingsGUI::rotateOption(int rotateAmount)
{
    if (!inSub())
    {
        // If we're not in the sub option menu, change the option.
        _selectedOption = (_selectedOption + rotateAmount) % (_pages[_selectedPage].options().size());
    }
    else
    {
        // Change the sub option instead.
        auto selectedOption = _pages[_selectedPage].options()[_selectedOption];
        auto &action = selectedOption.action_sub();
        if (action.sub())
        {
            auto sub = *action.sub();
            int currentValueIndex = sub.selected();
            int nextValueIndex = (currentValueIndex + rotateAmount) % (sub.values().size());
            if (currentValueIndex == -1)
                nextValueIndex = 0;
            auto nextValue = selectedOption.values()[nextValueIndex];

            if (auto subaction = std::get_if<Option::Bool>(&sub.action()))
            {
                subaction->set(*std::get_if<bool>(&nextValue));
            }

            if (auto subaction = std::get_if<Option::Str>(&sub.action()))
            {
                subaction->set(*std::get_if<cstr>(&nextValue));
            }

            if (auto subaction = std::get_if<Option::Int>(&sub.action()))
            {
                subaction->set(*std::get_if<int>(&nextValue));
            }
        }
    }

    clearText();
}