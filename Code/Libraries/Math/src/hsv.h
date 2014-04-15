#ifndef HSV_H
#define HSV_H

#include "vector.h"

namespace HSV
{
	Vector RGBToHSV( const Vector& ColorRGB );
	Vector HSVToRGB( const Vector& ColorHSV );
}

#endif // HSV