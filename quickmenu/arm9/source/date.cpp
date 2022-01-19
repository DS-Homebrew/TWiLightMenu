#include "date.h"

#include <ctime>

#include "common/twlmenusettings.h"
#include "common/tonccpy.h"
#include "language.h"

tm iTimeParts;

/**
 * Get the current date formatted for the current language.
 * @return std::string containing the date
 */
std::string getDate(void)
{
	time_t raw;
	time(&raw);
	const struct tm *time = localtime(&raw);

	char buf[24];
	strftime(buf, sizeof(buf), STR_DATE_FORMAT.c_str(), time);
	return std::string(buf);
}

/**
 * Get the current time formatted for the top bar.
 * @return std::string containing the time.
 */
std::string retTime(void)
{
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	char buf[24];
	strftime(buf, sizeof(buf), ms().show12hrClock ? STR_TIME_FORMAT_12.c_str() : STR_TIME_FORMAT_24.c_str(), Time);
	return std::string(buf);
}
