#ifndef DATE_H
#define DATE_H

#include <string>

/**
 * Get the current date formatted for the current language.
 * @return std::string containing the date
 */
std::string getDate(void);

/**
 * Get the current time formatted for the top bar.
 * @return std::string containing the time.
 */
std::string retTime(void);

#endif // DATE_H
