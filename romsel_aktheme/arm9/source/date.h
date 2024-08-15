#ifndef DATE_H
#define DATE_H

#include <nds/ndstypes.h>
#include <string>
// #include <stddef.h>

u16 dateYear();
u8 dateMonth();
u8 dateDay();
u8 dateWeekday();
u8 dateHours();
u8 dateMinutes();
u8 dateSeconds();

/**
 * Get the current time.
 * @return std::string containing the time.
 */
std::string retTime(void);

/**
 * Get the current year.
 * @return std::string containing the year.
 */
std::string retYear(void);

/**
 * Get the current month.
 * @return std::string containing the month.
 */
std::string retMonth(void);

/**
 * Get the current day.
 * @return std::string containing the day.
 */
std::string retDay(void);

#endif // DATE_H
