#ifndef TIMEDATE_H
#define TIMEDATE_H

#include "simplestring.h"

namespace TimeDate
{
	int	GetHours12();	// 0-11
	int	GetHours24();	// 0-23
	int	GetAMPM();		// 0 = AM, 1 = PM
	int	GetMinutes();	// 0-59
	int	GetSeconds();	// 0-59
	int GetDay();		// 0-31
	int GetMonth();		// 0-11
	int GetYear();		// Actual year AD

	SimpleString GetMonthName( int Month );

	SimpleString GetTimeDateString();	// Format: yyyy-mm-dd-HH-MM-SS
}

#endif // TIMEDATE_H