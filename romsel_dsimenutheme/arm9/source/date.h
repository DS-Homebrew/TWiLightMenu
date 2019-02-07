#ifndef DATE_H
#define DATE_H

#include <nds.h>
#include <string>
#include <stddef.h>

typedef enum {
	FORMAT_YDM	= 0,
	FORMAT_YMD	= 1,
	FORMAT_DM	= 2,
	FORMAT_MD	= 3,
	FORMAT_M_D	= 4,
	FORMAT_MY	= 5,
	FORMAT_M	= 6,
	FORMAT_Y	= 7,
} DateFormat;

/**
 * Get the current date as a C string.
 * @param format Date format.
 * @param buf Output buffer.
 * @param size Size of the output buffer.
 * @return Number of bytes written, excluding the NULL terminator.
 * @return Current date. (Caller must free() this string.)
 */
size_t GetDate(DateFormat format, char *buf, size_t size);

/**
 * Get the current time formatted for the top bar.
 * @return std::string containing the time.
 */
std::string RetTime();

/**
 * Draw the date using the specified format.
 * @param format Date format.
 */
char* DrawDateF(DateFormat format);

/**
 * Draw the month and year using the specified color.
 * Format is selected based on the language setting.
 */
char* DrawDate(void);

#endif // DATE_H
