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
// Class DateTime
//
// This class is used to represent an abstract representation of a fixed date
// and time.  The date is stored a a Julian Day Number with a reference date
// of 0/0/1.
//
// Note: The Gregorian Calendar did not come into existence until 15 Oct 1582
// but dates earlier than this will still use the same algorithm
// and will therefore be treated as if they are in the proleptic Gregorian
// Calendar.
//
// The conversion of a Julian Day number into the Gregorian callendar
// has been the subject of a fair amount of research.  
//
// Fliegel and van Flandern (1968) published compact computer algorithms
// for converting between Julian dates and Gregorian calendar dates.
// Their algorithms were presented in the Fortran programming language, 
// and take advantage of the truncation feature of integer arithmetic. 
//
// These algorithms have been re-used in this class by simply transcoding
// the Fortran into C++.
//
// Fliegel, H. F. and van Flandern, T. C. (1968). 
// - Communications of the ACM, Vol. 11, No. 10 (October, 1968). 
// Further info: http://aa.usno.navy.mil/faq/docs/JD_Formula.html
//
//==============================================================================

#include "DateTime.h"
#include "InvalidDateException.h"
#include "StringTokenizer.h"

#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/ArrayAutoPtr.h"
#include "QcCore/base/NullPointerException.h"

#include <time.h>

#if defined(WIN32)

#include "QcCore/base/winincl.h"

#if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
struct timeval {
	long    tv_sec;         /* seconds */
	long    tv_usec;        /* microseconds */
};
#endif 

#else // !WIN32

#include <sys/time.h>

#endif // WIN32

QC_UTIL_NAMESPACE_BEGIN

	const long MSIN1DAY   = 86400000;
const long SECSIN1DAY = 86400;
const long MSIN1HOUR  = 3600000;

const short LeapMonths[] = {31, 29, 31, 30,  31,  30,  31,  31,  30,  31,  30,  31};
const short StdMonths[]  = {31, 28, 31, 30,  31,  30,  31,  31,  30,  31,  30,  31};
const short LeapDays[]   = {0,  31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};
const short StdDays[]    = {0,  31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

const long UNIX_REF_JD = 2440588; // The Julian Day number for 1/1/1970

#if defined(_MSC_VER) || defined(_WINDOWS_)
#define EPOCHFILETIME (116444736000000000LL)
int gettimeofday(struct timeval* tv, const char *tz) 
{
	union {
		long long ns100;
		FILETIME ft;
	} now;

	GetSystemTimeAsFileTime (&now.ft);
	tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
	tv->tv_sec = (long) ((now.ns100 - EPOCHFILETIME) / 10000000LL);
	return (0);
}
#endif

//==============================================================================
// DateTime::DateTime
//
//==============================================================================
DateTime::DateTime() :
m_JD(0),
	m_ms(0)
{
}

//==============================================================================
// DateTime::DateTime
//
//==============================================================================
DateTime::DateTime(const String& strDateTime) :
m_JD(0),
	m_ms(0)
{
	if(!parseDateTime(strDateTime))
	{
		throw InvalidDateException(strDateTime);
	}
}

//==============================================================================
// DateTime::DateTime
//
//==============================================================================
DateTime::DateTime(UShort year, UShort month, UShort day) :
m_JD(0),
	m_ms(0)
{
	if(!IsValidDate(year, month, day))
	{
		throw InvalidDateException();
	}
	setDate(year, month, day);
}

//==============================================================================
// DateTime::DateTime
//
//==============================================================================
DateTime::DateTime(UShort year, UShort month, UShort day,
	UShort hour, UShort minute, UShort second, UShort ms) :
m_JD(0),
	m_ms(0)
{
	if(!IsValidDate(year, month, day) || !IsValidTime(hour, minute, second, ms))
	{
		throw InvalidDateException();
	}
	setDate(year, month, day);
	setTime(hour, minute, second, ms);
}

//==============================================================================
// DateTime::DateTime
//
//==============================================================================
DateTime::DateTime(unsigned long julianDay) :
m_JD(julianDay),
	m_ms(0)
{
}

//==============================================================================
// DateTime::DateTime
//
//==============================================================================
DateTime::DateTime(unsigned long julianDay, unsigned long milliseconds) :
m_JD(julianDay),
	m_ms(milliseconds)
{
}

bool DateTime::operator==(const DateTime& rhs) const
{
	return (m_JD == rhs.m_JD && m_ms == rhs.m_ms);
}

bool DateTime::operator!=(const DateTime& rhs) const
{
	return !(*this == rhs);
}

bool DateTime::operator<(const DateTime& rhs) const
{
	return (m_JD < rhs.m_JD ||
		(m_JD == rhs.m_JD && m_ms < rhs.m_ms));
}

bool DateTime::operator<=(const DateTime& rhs) const
{
	return (m_JD < rhs.m_JD ||
		(m_JD == rhs.m_JD && m_ms <= rhs.m_ms));
}

bool DateTime::operator>(const DateTime& rhs) const
{
	return (m_JD > rhs.m_JD ||
		(m_JD == rhs.m_JD && m_ms > rhs.m_ms));
}

bool DateTime::operator>=(const DateTime& rhs) const
{
	return (m_JD > rhs.m_JD ||
		(m_JD == rhs.m_JD && m_ms >= rhs.m_ms));
}

//==============================================================================
// DateTime::parseDateTime
//
// The initial motivation for this function was to be able to
// parse date/time strings returned in HTTP responses.
//
// The implementation, however, generalizes the concept slightly so that
// a wider range of dates can be parsed than just those present in HTTP headers.
//
// Returns true if the date is recognized as a valid date
//==============================================================================
bool DateTime::parseDateTime(const String& strDateTime)
{
	// RFC2616 (HTTP/1.1) gives the following example date formats:-
	//
	// Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
	// Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
	// Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format (fixed width)
	//
	// Email, USENET and HTTP all seem to favour RFC 822 formats.
	// In a straw poll, all HTTP servers seem to report the date using GMT
	//

	// 
	// RFC822 gives date/time syntax as:
	//
	//   date-time   =  [ day "," ] date time        ; dd mm yy
	//                                               ;  hh:mm:ss zzz
	//
	//   day         =  "Mon"  / "Tue" /  "Wed"  / "Thu"
	//               /  "Fri"  / "Sat" /  "Sun"
	//
	//   date        =  1*2DIGIT month 2DIGIT        ; day month year
	//                                               ;  e.g. 20 Jun 82
	//
	//   month       =  "Jan"  /  "Feb" /  "Mar"  /  "Apr"
	//               /  "May"  /  "Jun" /  "Jul"  /  "Aug"
	//               /  "Sep"  /  "Oct" /  "Nov"  /  "Dec"
	//
	//   time        =  hour zone                    ; ANSI and Military
	//
	//   hour        =  2DIGIT ":" 2DIGIT [":" 2DIGIT]
	//                                               ; 00:00:00 - 23:59:59
	//
	//   zone        =  "UT"  / "GMT"                ; Universal Time
	//                                               ; North American : UT
	//               /  "EST" / "EDT"                ;  Eastern:  - 5/ - 4
	//               /  "CST" / "CDT"                ;  Central:  - 6/ - 5
	//               /  "MST" / "MDT"                ;  Mountain: - 7/ - 6
	//               /  "PST" / "PDT"                ;  Pacific:  - 8/ - 7
	//               /  1ALPHA                       ; Military: Z = UT;
	//                                               ;  A:-1; (J not used)
	//                                               ;  M:-12; N:+1; Y:+12
	//               / ( ("+" / "-") 4DIGIT )        ; Local differential
	//                                               ;  hours+min. (HHMM)
	//
	//
	// The most flexible approach to parsing dates is a heuristic one, where
	// we split the date into tokens and analyse each token to determine
	// what date part it represents.
	//
	// One advantage of using a heuristic approach is that once complete,
	// it should remain stable even when parsing dates from new software.
	// The alternative approach, where we parse a number of known formats
	// like the RFC list above, is easier initially, but may break whenever
	// a new type of date is introduced.
	//
	// One disadvantage of using a heuristic approach is that it is more 
	// difficult to detect errors.  With a fixed-width parsing approach
	// such as RFC 822 it is easy to determine if a date is correctly
	// specified.
	//
	// At the very least, any heuristic approach should successfully parse
	// the dates of RFC 822, 850 and asctime.  It should also be able to
	// parse a date string such as 2001/12/09 12:00:00, but it need not parse
	// 01/02/2001 as this is ambiguous whether the 2nd token is the day or month.
	//
	//-------------------------------------------------------------------------
	//
	// The algorithm
	// -------------
	//
	// 1) split date string into tokens separated by ", "
	//    this gives a list of tokens containing single or compound terms
	//
	// 2) For each token do the following tests.  Once a test is passed, stop
	//    processing that token
	//    i)   does it contain "/-" separators?  If so parse a compound date
	//    ii)  does it contain ":" separators?  Of so parse a coumpound time
	//    iii) is the token alpha? If so parse month names and timezones
	//    iv)  is the token numeric?  If so the first such token should be taken
	//         to represent the day of the month, unless > 31.  Next such token
	//         should represent the year in 4 digits.  2-digit years are
	//         not recognized.
	//    v)   does the token start with "-" or "+".  If so use this to adjust the
	//         timezone.
	//    vi)  Is the token surrounded by brackets ().  If so this is taken to
	//         represent an indicative timezone, but the actual timezone is UTC
	//         plus the difference indicated by (v).
	// 3) If all the date components are missing, but a time components exists,
	//    create an invalid Date with a valid time.
	// 4) If the time components are missing, but the date components exist
	//    create a valid date with a time of 00:00:00.
	//
	// Returns: true if a valid date or time was found.

	String dateString;
	short  nYear=0;
	short  nMonth=0;
	short  nDay=0;
	short  nHours=0;
	short  nMinutes=0;
	short  nSeconds=0;
	short  nTimezoneMinutes=0;

	bool bHaveDate, bHaveTime(false), bHaveTZ(false);

	const String dateSeps = QC_T("/-");
	const String timeSeps = QC_T(":");
	bool bValid=true;

	//
	// Tokenize an upper-case representation of the input string
	//
	StringTokenizer tokenizer(
		StringUtils::ToUpper(strDateTime), QC_T(", "), false);

	while(tokenizer.hasMoreTokens() && bValid)
	{
		const String& token = tokenizer.nextToken();

		//
		// To compensate for libstdc++ lack of compare(x,x,x,x)
		// we create a ByteString so that we can use strncmp instead
		//
		ByteString narrowToken = StringUtils::ToAscii(token);

		//
		// test for a date string
		//
		if(token.find_first_of(dateSeps, 1) != String::npos)
		{
			std::vector<String> dateVec = StringTokenizer(token, dateSeps, false).toVector();
			if(dateVec.size() != 3)
			{
				bValid = false;
				break;
			}

			//
			// We only accept dates in YYYY/MM/DD format
			//
			nYear  = NumUtils::ToInt(dateVec[0]);
			nMonth = NumUtils::ToInt(dateVec[1]);
			nDay   = NumUtils::ToInt(dateVec[2]);
			continue;
		}

		//
		// test for a time string
		//
		if(token.find_first_of(timeSeps, 1) != String::npos)
		{
			std::vector<String> timeVec = StringTokenizer(token, timeSeps, false).toVector();
			if(timeVec.size() != 3)
			{
				bValid = false;
				break;
			}

			if(bHaveTime) {bValid=false; break;}

			//
			// Times are always in HH:MM:SS format
			//
			nHours   = NumUtils::ToInt(timeVec[0]);
			nMinutes = NumUtils::ToInt(timeVec[1]);
			nSeconds = NumUtils::ToInt(timeVec[2]);
			bHaveTime = true;

			continue;
		}
		//
		// if we don't yet have a month test for a month name
		//
		// Currently we just check the first three characters of any
		// name.  This should work okay as no English day names co-incide
		// with month names in the first three characters and this method allows
		// for long names to be passed.
		if(token.length() >= 3 && nMonth==0)
		{
			static const char* szMonths = "JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";
			for(int m=0; m<12; ++m)
			{
				if(strncmp(szMonths+m*3, narrowToken.c_str(), 3)==0)
				{
					nMonth = m+1;
					break;
				}
			}

			if(nMonth)
				continue;
		}
		//
		// is the token numeric?
		//
		int nToken = atoi(narrowToken.c_str());
		if(nToken)
		{
			//
			// Does the token start with + or -
			// If so use it as a TZ offset
			//
			if(narrowToken[0] == '+' || narrowToken[0] == '-')
			{
				if((token.length() != 5) || nTimezoneMinutes)
				{
					bValid = false;
					break;
				}
				short tzHours   = nToken / 100;
				nTimezoneMinutes = nToken - (tzHours * 40);
				continue;
			}

			if(token.length() == 4)
			{
				if(nYear) {bValid = false; break;}
				nYear = nToken;
			}
			else if(nToken <= 31)
			{
				if(nDay) {bValid = false; break;}
				nDay = nToken;
			}
			continue;
		}
		//
		// All thats left of interest is timezones
		//
		if(!bHaveTZ)
		{
			if(narrowToken == "UT")
			{
				bHaveTZ = true;
			}
			else if(narrowToken.length() ==3)
			{
				static const size_t zones = 11;
				static const char* szTZ = "GMTUTCUCTESTEDTCSTCDTMSTMDTPSTPDT";
				static int UTCOffset[zones] = {0,0,0,-5,-4,-6,-5,-7,-6,-8,-7};

				for(size_t z=0; z<zones; ++z)
				{
					if(strncmp(szTZ+z*3, narrowToken.c_str(), 3)==0)
					{
						bHaveTZ = true;
						nTimezoneMinutes = UTCOffset[z]*60;
						break;
					}
				}

				if(bHaveTZ)
				{
					continue;
				}
			}
		}
		//
		// Other tokens such as the day of week or relative TZ just aren't
		// very interesting
		//
	}

	//
	// Now that all the tokens have been processed we can decide if the date/time
	// is actually useful
	//
	bHaveDate = (nYear && nMonth && nDay);
	bValid = bValid && (bHaveTime || bHaveDate);
	bValid = bValid && (!bHaveTime || IsValidTime(nHours, nMinutes, nSeconds, 0));
	bValid = bValid && (!bHaveDate || IsValidDate(nYear, nMonth, nDay));

	if(bValid && bHaveDate)
	{
		setDate(nYear, nMonth, nDay);
	}
	if(bValid && bHaveTime)
	{
		setTime(nHours, nMinutes, nSeconds, 0);
	}

	if(nTimezoneMinutes)
	{
		adjust(0, 0, -1 * nTimezoneMinutes, 0, 0);
	}

	return bValid; 
}

//==============================================================================
// DateTime::setTime
//
//==============================================================================
void DateTime::setTime(UShort hour, UShort minute, UShort second, UShort millisecond)
{
	if(!IsValidTime(hour, minute, second, millisecond))
	{
		throw InvalidDateException();
	}
	m_ms = millisecond + (second*1000) + (minute * 60000) + (hour*MSIN1HOUR);
}

//==============================================================================
// DateTime::setDate
//
//==============================================================================
void DateTime::setDate(UShort year, UShort month, UShort day)
{
	if(!IsValidDate(year, month, day))
	{
		throw InvalidDateException();
	}

	long I,J,K;
	I= year;
	J= month;
	K= day;

	m_JD= K-32075+1461*(I+4800+(J-14)/12)/4+367*(J-2-(J-14)/12*12)/12-3*
		((I+4900+(J-14)/12)/100)/4;
}

//==============================================================================
// DateTime::adjust
//
// Any of the arguments may be positive or negative integers.  A negative
// integer means the DateTime should be adjusted backwards in time by the
// specified amount.
//==============================================================================
void DateTime::adjust(short nDays, short hours, short nMinutes,
	short nSeconds, short nMilliseconds)
{
	long adjust = nMilliseconds + (nSeconds*1000) + (nMinutes * 60000) + (hours*MSIN1HOUR);

	//
	// It is quite possible and legal for the combined time adjustment values
	// to exceed 1 day, so normalize it into a ms and day value, either of which
	// may be positive or negative.
	// 
	long numDays = nDays + adjust / MSIN1DAY;
	long totalMS = m_ms + (adjust % MSIN1DAY);

	//
	// If we have a -ve time , we convert it into a positive
	// time combined with a negative day adjustment.
	//
	if(totalMS < 0)
	{
		numDays--;
		totalMS+=MSIN1DAY;
	}

	m_JD += numDays;
	m_ms = totalMS;
}

//==============================================================================
// DateTime::getDate
//
// Returns the date in the Gregorian Calendar
//
//==============================================================================
void DateTime::getDate(UShort& year, UShort& month, UShort& day, UShort& dayOfWeek) const
{
	//
	// Fliegel and Flandern algorithm in C++
	//
	long L= m_JD+68569;
	long N= 4*L/146097;
	L= L-(146097*N+3)/4;
	long I= 4000*(L+1)/1461001;
	L= L-1461*I/4+31;
	long J= 80*L/2447;
	long K= L-2447*J/80;
	L= J/11;
	J= J+2-12*L;
	I= 100*(N-49)+I+L;

	year  = UShort(I);
	month = UShort(J);
	day   = UShort(K);
	dayOfWeek = UShort(m_JD+1) % 7; // (0=Sunday, 1=Monday etc)
}

//==============================================================================
// DateTime::getTime
//
//==============================================================================
void DateTime::getTime(UShort& hour, UShort& minute, UShort& second, UShort& millisecond) const
{
	hour = UShort(m_ms / MSIN1HOUR);
	long work = m_ms % MSIN1HOUR;
	minute = UShort(work / 60000);   // milliseconds in one minute
	work = work % 60000;
	second = UShort(work / 1000);    // milliseconds in one second
	millisecond = UShort(work % 1000);
}

//==============================================================================
// DateTime::toAnsiTime
//
// Returns the number of seconds elapsed between our reference time and
// 00:00:00 on January 1, 1970,
//==============================================================================
time_t DateTime::toAnsiTime() const
{
	//
	// Re-base our Julian Day to be equal to the UNIX epoch of
	// 01/01/1970 which has a JD of 2440588
	//
	const long UnixReference = 2440588;
	time_t ret = (m_JD - UnixReference) * SECSIN1DAY + (m_ms / 1000);
	return ret;
}

//==============================================================================
// DateTime::toAnsiTM
//
// Fill the passed tm structure using the Gregorian Calendar
//==============================================================================
void DateTime::toAnsiTM(struct tm* ptm) const
{
	if(!ptm) throw NullPointerException();

	UShort year, month, day, dayOfWeek;
	getDate(year, month, day, dayOfWeek);

	memset(ptm, 0, sizeof(struct tm));
	ptm->tm_mon = month - 1;
	ptm->tm_year = year - 1900;
	ptm->tm_mday = day;
	ptm->tm_yday = day + IsLeapYear(year) ? LeapDays[month] : StdDays[month];
	ptm->tm_wday = dayOfWeek;

	ptm->tm_hour = m_ms / MSIN1HOUR;
	long workMS = m_ms % MSIN1HOUR;
	ptm->tm_min = workMS / 60000; // milliseconds in a minute
	workMS = workMS % 60000;
	ptm->tm_sec = workMS / 1000;
	ptm->tm_isdst = 0; // show time in UTC format
}

//==============================================================================
// DateTime::IsLeapYear
//
//==============================================================================
bool DateTime::IsLeapYear(UShort nYear)
{
	// Years divisible by four are leap years, unless...
	// Years also divisible by 100 are not leap years, except...
	// Years divisible by 400 are leap years.

	return ((nYear % 4 == 0) && ((nYear % 400 == 0) || (nYear % 100 != 0)));
}

//==============================================================================
// DateTime::IsValidTime
//
//==============================================================================
bool DateTime::IsValidTime(UShort hour, UShort minute, UShort second, UShort ms)
{
	return (hour<24 && minute<60 && second<60 && ms<1000);
}

//==============================================================================
// DateTime::IsValidDate
//
//==============================================================================
bool DateTime::IsValidDate(UShort year, UShort month, UShort day)
{
	if(year>0 && year<=9999 && month<=12 && month && day<=31 && day)
	{
		UShort dayMax = IsLeapYear(year) ? LeapMonths[month-1] : StdMonths[month-1];
		if(day <= dayMax)
		{
			return true;
		}
	}
	return false;
}

//==============================================================================
// DateTime::getJulianDay
//
//==============================================================================
unsigned long DateTime::getJulianDay() const
{
	return m_JD;
}

//==============================================================================
// DateTime::getTimeOfDay
//
//==============================================================================
unsigned long DateTime::getTimeOfDay() const
{
	return m_ms;
}

//==============================================================================
// DateTime::GetSystemTime
//
//==============================================================================
DateTime DateTime::GetSystemTime()
{

#if defined(WIN32)

	SYSTEMTIME now;
	::GetSystemTime(&now);
	return DateTime(now.wYear, now.wMonth, now.wDay,
		now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);

#else // !WIN32

	struct timeval tv;
	::gettimeofday(&tv, 0);

	//
	// The UNIX timeval represents the number of seconds since the epoch
	// and the number of micro-seconds elapsed in that second
	//
	return FromAnsiTime(tv.tv_sec, tv.tv_usec);

#endif // WIN32

}

//==============================================================================
// DateTime::FromAnsiTime
//
//==============================================================================
DateTime DateTime::FromAnsiTime(long secsSinceEpoch, unsigned long microSeconds)
{
	long seconds = secsSinceEpoch % SECSIN1DAY;
	long days = secsSinceEpoch / SECSIN1DAY;
	unsigned long JD = days + UNIX_REF_JD;
	unsigned long MS = (seconds * 1000) + (microSeconds / 1000);

	return DateTime(JD, MS);
}

//==============================================================================
// DateTime::currentTimeMicros
//
//==============================================================================
double DateTime::currentTimeMicros()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec * 1000000.0 + tv.tv_usec);
}

//==============================================================================
// DateTime::currentTimeMillis
//
//==============================================================================
double DateTime::currentTimeMillis()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (DateTime::currentTimeMicros() / 1000);
}


//==============================================================================
// DateTime::isValid
//
//==============================================================================
bool DateTime::isValid() const
{
	return m_JD > 0;
}

//==============================================================================
// DateTime::toString
//
//==============================================================================
String DateTime::toString() const
{
	//  Format as RFC 822 eg: Thu, 25 Oct 2001 20:03:28 GMT
	return Format(QC_T("%a, %d %b %Y %H:%M:%S GMT"));
}

//==============================================================================
// DateTime::Format
//
//==============================================================================
String DateTime::Format(const String& format) const
{
	if(!isValid())
	{
		return String();
	}

	struct tm mytm;
	toAnsiTM(&mytm);
	ByteString strFormat = StringUtils::ToAscii(format);

	const int stackBufferSize = 255;
	char stackBuffer[stackBufferSize];

	size_t n = strftime(stackBuffer, stackBufferSize, strFormat.c_str(), &mytm);

	//
	// Note: strftime does not give us a chanve qc allocate a larger buffer
	// because it returns either the length copied or 0 - without informing
	// us how large the buffer should be.  And a zero return code does
	// not necessarily indicate failure - the empty string it possible
	// given some formatting options.
	//
	if(n==0)
	{
		return String();
	}
	else
	{
		return StringUtils::FromLatin1(stackBuffer);
	}
}

QC_UTIL_NAMESPACE_END
