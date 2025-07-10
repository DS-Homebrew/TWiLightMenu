#include "date.h"

#include <ctime>

#include "common/twlmenusettings.h"
#include "common/tonccpy.h"
#include "language.h"

Datetime::Datetime(time_t rawTime)
	:_time(rawTime)
{
	localtime_r(&rawTime, &_formattedTime);
}

/**
 * Create a datetime from a specified date
 */
Datetime::Datetime(int year, int month, int day, int hour, int minute, int second) {
	_formattedTime.tm_year = year - 1900;
	_formattedTime.tm_mon = month - 1;
	_formattedTime.tm_mday = day;
	_formattedTime.tm_hour = hour;
	_formattedTime.tm_min = minute;
	_formattedTime.tm_sec = second;

	_time = mktime(&_formattedTime);
}

/**
 * @return Whether or not is this a leap year.
 */
bool Datetime::isLeapYear() const {
	int year = _formattedTime.tm_year+1900;
	return ((year % 4 == 0) ^ (year % 100 == 0)) || (year % 400 == 0);
}

/**
 * @return The number of days in the month
 */
int Datetime::getMonthDays() const {
	int month = _formattedTime.tm_mon+1;
	if (month == 2)
		return isLeapYear() ? 29 : 28;
	
	if (month <= 7)
		return 30+(month%2);
	else
		return 31-(month%2);
}

/**
 * Get weekday from this date
 * @return 0 for Sunday ... 6 for Saturday
 */
int Datetime::getWeekDay() const {
	return _formattedTime.tm_wday;
}

/**
 * Get the current datetime
 * @return Datetime with the current datetime.
 */
Datetime Datetime::now() {
	time_t raw;
	time(&raw);
	return Datetime(raw); 
}

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
 * Get the current year & month formatted for the current language.
 * @return std::string containing the date
 */
std::string getDateYear(void)
{
	time_t raw;
	time(&raw);
	const struct tm *time = localtime(&raw);

	char buf[24];
	strftime(buf, sizeof(buf), STR_DATE_YEAR_FORMAT.c_str(), time);
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
