#include "core.h"
#include "hsv.h"
#include "mathcore.h"

// NOTE: For HSV, x is hue, y is saturation, and z is value.
// Many sources encode hue in [0, 360], but I'm using [0, 1].

// NOTE: This actually seems to work ok for RGB values greater than 1.0,
// but I may just have gotten lucky so far. It seems like it would depend
// on the chroma (max - min) not being greater than 1.0.
Vector HSV::RGBToHSV( const Vector& ColorRGB )
{
	Vector ColorHSV;

	ASSERT( ColorRGB.r >= 0.0f );
	ASSERT( ColorRGB.r <= 1.0f );
	ASSERT( ColorRGB.g >= 0.0f );
	ASSERT( ColorRGB.g <= 1.0f );
	ASSERT( ColorRGB.b >= 0.0f );
	ASSERT( ColorRGB.b <= 1.0f );

	const float MaxValue	= Max( ColorRGB.r, Max( ColorRGB.g, ColorRGB.b ) );

	// Value is the greatest component of RGB
	ColorHSV.z = MaxValue;

	if( MaxValue < EPSILON )
	{
		// Without value, hue is undefined and saturation is 0.
		ColorHSV.x = 0.0f;
		ColorHSV.y = 0.0f;
		return ColorHSV;
	}

	const float MinValue	= Min( ColorRGB.r, Min( ColorRGB.g, ColorRGB.b ) );
	const float Chroma		= MaxValue - MinValue;	// Roughly, relative colorfulness of components

	// Saturation is normalized chroma
	ColorHSV.y = Chroma / MaxValue;

	if( Chroma < EPSILON )
	{
		// Without chroma, hue is undefined.
		ColorHSV.x = 0.0f;
		return ColorHSV;
	}

	// Do in 0,360 first, then divide. Figure out where I'm going wrong.
	static const float kHueScalar	= 1.0f / 6.0f;
	static const float kGreenShift	= 1.0f / 3.0f;
	static const float kBlueShift	= 2.0f / 3.0f;
	if( MaxValue == ColorRGB.r )
	{
		ColorHSV.x = ( ( ColorRGB.g - ColorRGB.b ) / Chroma ) * kHueScalar;
		if( ColorHSV.x < 0.0f )
		{
			ColorHSV.x += 1.0f;
		}
	}
	else if( MaxValue == ColorRGB.g )
	{
		ColorHSV.x = kGreenShift + ( ( ( ColorRGB.b - ColorRGB.r ) / Chroma ) * kHueScalar );
	}
	else
	{
		ColorHSV.x = kBlueShift + ( ( ( ColorRGB.r - ColorRGB.g ) / Chroma ) * kHueScalar );
	}

	return ColorHSV;
}

inline float HSVMod( float F )
{
	while( F < 0.0f )
	{
		F += 1.0f;
	}
	while( F > 1.0f )
	{
		F -= 1.0f;
	}
	return F;
}

Vector HSV::HSVToRGB( const Vector& ColorHSV )
{
	Vector ColorRGB;

	const float Hue			= HSVMod( ColorHSV.x );
	const float Chroma		= ColorHSV.z * ColorHSV.y;
	const float MinValue	= ColorHSV.z - Chroma;
	const float MidValue	= MinValue + Chroma * ( 1.0f - Abs( Mod( Hue * 6.0f, 2.0f ) - 1.0f ) );	// wat
	const float MaxValue	= ColorHSV.z;

	if( Hue < ( 1.0f / 6.0f ) )
	{
		ColorRGB.r = MaxValue;
		ColorRGB.g = MidValue;
		ColorRGB.b = MinValue;
	}
	else if( Hue < ( 2.0f / 6.0f ) )
	{
		ColorRGB.r = MidValue;
		ColorRGB.g = MaxValue;
		ColorRGB.b = MinValue;
	}
	else if( Hue < ( 3.0f / 6.0f ) )
	{
		ColorRGB.r = MinValue;
		ColorRGB.g = MaxValue;
		ColorRGB.b = MidValue;
	}
	else if( Hue < ( 4.0f / 6.0f ) )
	{
		ColorRGB.r = MinValue;
		ColorRGB.g = MidValue;
		ColorRGB.b = MaxValue;
	}
	else if( Hue < ( 5.0f / 6.0f ) )
	{
		ColorRGB.r = MidValue;
		ColorRGB.g = MinValue;
		ColorRGB.b = MaxValue;
	}
	else
	{
		ColorRGB.r = MaxValue;
		ColorRGB.g = MinValue;
		ColorRGB.b = MidValue;
	}

	return ColorRGB;
}