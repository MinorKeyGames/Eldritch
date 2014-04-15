#include "core.h"
#include "timedate.h"

#include <time.h>

tm GetLocalTime()
{
	__time64_t	Time;
	tm			TimeStruct;

	_tzset();	// Apply environment variables so that _localtime_s will convert properly
	_time64( &Time );
	_localtime64_s( &TimeStruct, &Time );

	return TimeStruct;
}

int TimeDate::GetHours12()
{
	int Hours = GetLocalTime().tm_hour % 12;
	if( Hours == 0 )
	{
		Hours = 12;
	}
	return Hours;
}

int TimeDate::GetHours24()
{
	return GetLocalTime().tm_hour;
}

int TimeDate::GetAMPM()
{
	int Hours = GetLocalTime().tm_hour;
	if( Hours < 12 )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int TimeDate::GetMinutes()
{
	return GetLocalTime().tm_min;
}

int TimeDate::GetSeconds()
{
	return GetLocalTime().tm_sec;
}

int TimeDate::GetDay()
{
	return GetLocalTime().tm_mday;
}

int TimeDate::GetMonth()
{
	return GetLocalTime().tm_mon;
}

int TimeDate::GetYear()
{
	return GetLocalTime().tm_year + 1900;
}

// If a localized month is needed, pass the return
// value to ConfigManager::GetLocalizedString
SimpleString TimeDate::GetMonthName( int Month )
{
	switch( Month )
	{
	case 0:
		return "January";
	case 1:
		return "February";
	case 2:
		return "March";
	case 3:
		return "April";
	case 4:
		return "May";
	case 5:
		return "June";
	case 6:
		return "July";
	case 7:
		return "August";
	case 8:
		return "September";
	case 9:
		return "October";
	case 10:
		return "November";
	case 11:
		return "December";
	default:
		WARNDESC( "Invalid month." );
		return "";
	}
}

SimpleString TimeDate::GetTimeDateString()
{
	return SimpleString::PrintF( "%04d-%02d-%02d-%02d-%02d-%02d", GetYear(), GetMonth() + 1, GetDay(), GetHours24(), GetMinutes(), GetSeconds() );
}