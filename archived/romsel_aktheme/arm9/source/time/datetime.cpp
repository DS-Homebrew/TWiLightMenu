/*
    datetime.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h> //memset
#include "datetime.h"
#include "common/tonccpy.h"
const char * DateTime::weekdayStrings[]= { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

void DateTime::FillTimeParts(void)
{
  time_t epochTime;
  if (time(&epochTime)==(time_t)-1)
  {
    toncset(&iTimeParts,0,sizeof(iTimeParts));
  }
  else
  {
    localtime_r(&epochTime,&iTimeParts);
  }
}

u16 DateTime::year()
{
  FillTimeParts();
  return iTimeParts.tm_year+1900;
}

u8 DateTime::month()
{
  FillTimeParts();
  return iTimeParts.tm_mon+1;
}

u8 DateTime::day()
{
  FillTimeParts();
  return iTimeParts.tm_mday;
}

u8 DateTime::weekday()
{
  FillTimeParts();
  return iTimeParts.tm_wday;
}

u8 DateTime::hours()
{
  FillTimeParts();
  return iTimeParts.tm_hour;
}

u8 DateTime::minutes()
{
  FillTimeParts();
  return iTimeParts.tm_min;
}

u8 DateTime::seconds()
{
  FillTimeParts();
  return iTimeParts.tm_sec;
}

std::string DateTime::getDateString()
{
  //FillTimeParts();
  return formatString( "%d/%d%/%d %s\n", year(), month(), day(), weekdayStrings[weekday()] );
}

std::string DateTime::getTimeString()
{
  //FillTimeParts();
  return formatString( "%d:%d%:%d\n", hours(), minutes(), seconds() );
}

std::string DateTime::getTimeStampString()
{
  //FillTimeParts();
  return formatString( "%04d%02d%02d%02d%02d%02d", year(), month(), day(), hours(), minutes(), seconds() );
}
