/*
 * This file is part of QuickCPP.
 * (c) Copyright 2011 Jie Wang(twj31470952@gmail.com)
 *
 * QuickCPP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuickCPP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QuickCPP.  If not, see <http://www.gnu.org/licenses/>.
 */
 
//==============================================================================
//
// $Revision$
// $Date$
//
//==============================================================================
//
// Class Date
//
// Class DateTime
//
// This class is used to represent a precise point in time (with an accuracy
// of one millisecond) without reference to a particular calendar system.
//
// Internally, the date and time is represented by two integer values, one
// representing the Julian Day number (number of days ellapsed from a reference
// day) and another representing the number of milliseconds since midnight.
//
// The Gregorian Callendar
// -----------------------
// In accordance with common practice, this class provides interfaces
// to set and extract date information using the Gregorian Calendar notations
// of year, month and day.  When these interfaces are used, the DateTime class
// convertes between its internal Julian Day representation to the Gregorian 
// callendar.
// This class can be used to store any date in any calendar system, but other
// calendar systems will have to make use of the internal representation.
//
//==============================================================================

#ifndef QC_UTIL_DateTime_h
#define QC_UTIL_DateTime_h

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

#include <time.h>
#include "QcCore/base/QCObject.h"

QC_UTIL_NAMESPACE_BEGIN

class QC_UTIL_PKG DateTime : public virtual QCObject
{
public:

	DateTime(); // construct an invalid DateTime
	DateTime(UShort year, UShort month, UShort day);
	DateTime(UShort year, UShort month, UShort day, UShort hour, UShort minute, UShort second, UShort ms);
	DateTime(const String& strDateTime);
	DateTime(unsigned long julianDay);
	DateTime(unsigned long julianDay, unsigned long milliseconds);

	bool operator==(const DateTime& rhs) const;
	bool operator!=(const DateTime& rhs) const;
	bool operator<(const DateTime& rhs) const;
	bool operator<=(const DateTime& rhs) const;
	bool operator>(const DateTime& rhs) const;
	bool operator>=(const DateTime& rhs) const;

	unsigned long getJulianDay() const;
	unsigned long getTimeOfDay() const;
	time_t toAnsiTime() const; //todo: make consistent with FromAnsiTime
	void toAnsiTM(struct tm* ptm) const;
	
	void getDate(UShort& year, UShort& month, UShort& day, UShort& dayOfWeek) const;
	void getTime(UShort& hour, UShort& minute, UShort& second, UShort& millisecond) const;
	void setDate(UShort year, UShort month, UShort day);
	void setTime(UShort hour, UShort minute, UShort second, UShort millisecond);
	void adjust(short days, short hours, short minutes, short seconds, short milliseconds);
	bool isValid() const;
	String toString() const;
	String Format(const String& format) const;

	static bool IsValidDate(UShort year, UShort month, UShort day);
	static bool IsValidTime(UShort hour, UShort minute, UShort second, UShort ms);
	static bool IsLeapYear(UShort nYear);
	
	static DateTime GetSystemTime();
	static DateTime FromAnsiTime(long secsSinceEpoch, unsigned long microSeconds);
	static double currentTimeMicros();
	static double currentTimeMillis();

protected:
	bool parseDateTime(const String& strDate);

private:
	unsigned long m_JD;
	unsigned long m_ms;
};

QC_UTIL_NAMESPACE_END

#endif //QC_UTIL_MIMEType_h
