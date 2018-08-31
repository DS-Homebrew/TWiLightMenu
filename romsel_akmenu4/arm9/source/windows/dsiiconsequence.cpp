#include "dsiiconsequence.h"
#include <nds.h>

DSiIconSequence::DSiIconSequence()
{
    memset(_gameTid, 0, sizeof(_gameTid));
    memset(_sequence, 0, sizeof(_sequence));
    _needUpdateSequence = false;
    _flipH = false;
    _flipV = false;
    _currentSequenceIndex = -1;
    _updatesUntilNewIndex = 0;
    _bitmapIndex = 0;
    _paletteIndex = 0;
}

void DSiIconSequence::update()
{
    if (!_visible || (_sequence[0] == 0x0001 && _sequence[1] == 0x0100))
        return;

    if (_needUpdateSequence)
    {
        reset();
        _needUpdateSequence = false;
        return;
    }

    if (_updatesUntilNewIndex <= 0)
    {
        _currentSequenceIndex++;

        if (_sequence[_currentSequenceIndex] == 0x0000) {
            reset();
            return;
        }

        _updatesUntilNewIndex == SEQ_DUR(_sequence[_currentSequenceIndex]);
        _flipH = SEQ_FLIPH(_sequence[_currentSequenceIndex]);
        _flipV = SEQ_FLIPV(_sequence[_currentSequenceIndex]);
        _bitmapIndex = SEQ_BMP(_sequence[_currentSequenceIndex]);
        _paletteIndex = SEQ_PAL(_sequence[_currentSequenceIndex]);
        return;
    }
    else
    {
        _updatesUntilNewIndex--;
        return;
    }
}

void DSiIconSequence::reset()
{
    _currentSequenceIndex = -1;
    _updatesUntilNewIndex = 0;
    _bitmapIndex = 0;
    _paletteIndex = 0;
}

DSiIconSequence::~DSiIconSequence()
{
}