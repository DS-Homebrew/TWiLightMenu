#include "date.h"

#include <ctime>

#include "common/twlmenusettings.h"

tm iTimeParts;

/**
 * Get the current time.
 * @return std::string containing the time.
 */
std::string retTime(void)
{
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	char buf[24];
	strftime(buf, sizeof(buf), ms().show12hrClock ? "%I:%M" : "%H:%M", Time);
	return std::string(buf);
}

/**
 * Get the current year.
 * @return std::string containing the year.
 */
std::string retYear(void)
{
	time_t raw;
	time(&raw);
	const struct tm *time = localtime(&raw);

	char buf[24];
	strftime(buf, sizeof(buf), "%Y", time);
	return std::string(buf);
}

/**
 * Get the current month.
 * @return std::string containing the month.
 */
std::string retMonth(void)
{
	time_t raw;
	time(&raw);
	const struct tm *time = localtime(&raw);

	char buf[24];
	strftime(buf, sizeof(buf), "%m", time);
	return std::string(buf);
}

/**
 * Get the current day.
 * @return std::string containing the day.
 */
std::string retDay(void)
{
	time_t raw;
	time(&raw);
	const struct tm *time = localtime(&raw);

	char buf[24];
	strftime(buf, sizeof(buf), "%d", time);
	return std::string(buf);
}
