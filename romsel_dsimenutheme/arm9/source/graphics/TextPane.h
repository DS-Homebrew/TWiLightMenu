/*-----------------------------------------------------------------
 Copyright (C) 2015
	Matthew Scholefield

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#ifndef TEXTPANE_H
#define	TEXTPANE_H

#include <list>
#include <vector>
#include <string>
#include "TextEntry.h"



class TextPane
{
	const int START_PX, START_PY, SHOWN_ELEMENTS;
	int startIndex;
	std::list<TextEntry> shownText;
	std::vector<const char *> text;
	void wrapTransition();
public:
	void createDefaultEntries();
	void slideTransition(bool transitionIn, bool right = true, int delay = 0, int clickedIndex = -1);
	void scroll(bool up);
	void addLine(const char *line);
	bool update(bool top);
	TextPane(int startPX, int startPY, int shownElements);
};

#endif	/* TEXTPANE_H */

