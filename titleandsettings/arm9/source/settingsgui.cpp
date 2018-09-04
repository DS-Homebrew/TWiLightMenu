#include "settingsgui.h"

#include "graphics/fontHandler.h"
#include <variant>
#include <algorithm>

void SettingsGUI::draw()
{
    if (_selectedPage < 0 || _pages.size() < 1 ||_selectedPage >= _pages.size()) return;

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

    // _topCursor = std::max<int>(0, _selectedOption - MAX_ELEMENTS);
    // _bottomCursor = std::max<int>(MAX_ELEMENTS, _selectedOption);

    for (int i = _topCursor; i < _bottomCursor; i++)
    {
        int selected = _pages[_selectedPage].options()[i].selected();

        if (i == _selectedOption)
            printSmall(false, 4, 29 + (i - _topCursor) * 14, ">");

        printSmall(false, 12, 30 + (i - _topCursor) * 14, _pages[_selectedPage].options()[i].displayName().c_str());
        printSmall(false, 194, 30 + (i - _topCursor) * 14, _pages[_selectedPage].options()[i].labels()[selected].c_str());
    }
    printSmallCentered(false, 173, "DSiMenu++");
}

void SettingsGUI::drawSub(Option &option)
{
    clearText();
    int selected = option.selected();

    // option may be destroyed upon exit of this function, so
    // we need to copy the option here.

    // The use of smart pointers ensures that the old value is dropped
    // once we're finished with it.

    // There is an optimization to be done here to check if
    // the copy is really necessary, but doing the copy here
    // is fast enough for now.
    _subOption = std::make_unique<Option>(option);

    for (int i = 0; i < _subOption->values().size(); i++)
    {
        if (i == selected)
            printSmall(false, 4, 29 + i * 14, ">");
        printSmall(false, 12, 30 + i * 14, _subOption->labels()[i].c_str());
    }
    printLarge(false, 6, 1, _subOption->displayName().c_str());
    printSmallCentered(false, 173, "DSiMenu++");
}

void SettingsGUI::rotateOptionValue(int rotateAmount)
{
    // Only the main menu pages have left-right option values. Sub menus only control one Option.
    if (inSub())
        return;

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
        if ((_selectedOption + rotateAmount) < 0 || ((_selectedOption + rotateAmount) >= _pages[_selectedPage].options().size()))
        {
            return;
        }

        if (_selectedOption + rotateAmount >= _bottomCursor)
        {
            _topCursor++;
            _bottomCursor++;
        }

        if (_selectedOption + rotateAmount < _topCursor)
        {
            _topCursor--;
            _bottomCursor--;
        }

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
            auto nextValue = sub.values()[nextValueIndex];


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
                int value = *std::get_if<int>(&nextValue);
                subaction->set(value);
            }
        }
    }

    clearText();
}