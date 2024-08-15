#ifndef DATE_H
#define DATE_H

#include <string>
#include <ctime>

/**
 * An utility class for working with dates and times.
 */
class Datetime {
private:
    time_t _time;
    struct tm _formattedTime;
public:
    Datetime(time_t rawTime);
    Datetime() :Datetime(0) {}

    /**
     * Create a datetime from a specified date
     */
    Datetime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0);

    inline int getYear() const { return _formattedTime.tm_year+1900; };
    inline int getMonth() const { return _formattedTime.tm_mon+1; };
    inline int getDay() const { return _formattedTime.tm_mday; };
    inline int getHour() const { return _formattedTime.tm_hour; };
    inline int getMinute() const { return _formattedTime.tm_min; };
    inline int getSecond() const { return _formattedTime.tm_sec; };

    /**
     * @return Whether or not is this a leap year.
     */
    bool isLeapYear() const;

    /**
     * @return The number of days in the month
     */
    int getMonthDays() const;

    /**
     * Get weekday from this date
     * @return 0 for Sunday ... 6 for Saturday
     */
    int getWeekDay() const;

    /** 
     * Get the current datetime
     * @return Datetime type with the current datetime.
     */
    static Datetime now();

    time_t getRawTime() const { return _time; }
    operator time_t() const {
        return _time;
    }
};

/**
 * Get the current date formatted for the current language.
 * @return std::string containing the date
 */
std::string getDate(void);

/**
 * Get the current year & month formatted for the current language.
 * @return std::string containing the date
 */
std::string getDateYear(void);

/**
 * Get the current time formatted for the top bar.
 * @return std::string containing the time.
 */
std::string retTime(void);

#endif // DATE_H
