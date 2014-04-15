#ifndef COLOR_H
#define COLOR_H

struct Color
{
	Color() : m_Value( 0 ) {}
	Color( uint Value ) : m_Value( Value ) {}
	Color( byte a, byte r, byte g, byte b ) : A(a), R(r), G(g), B(b) {}
	operator uint() const { return m_Value; }
	union
	{
		uint m_Value;
		struct
		{
			// Assuming little-endian architecture
			byte B;
			byte G;
			byte R;
			byte A;
		};
	};
};

#define R_FROM_COLOR( c )			( (unsigned char)( ( ( c ) >> 16 ) & 0xff ) )
#define G_FROM_COLOR( c )			( (unsigned char)( ( ( c ) >> 8 )& 0xff ) )
#define B_FROM_COLOR( c )			( (unsigned char)( ( c ) & 0xff ) )
#define A_FROM_COLOR( c )			( (unsigned char)( ( ( c ) >> 24 ) & 0xff ) )
#define R_TO_COLOR( r )				( (uint)( ( (r) & 0xff ) << 16 ) )
#define G_TO_COLOR( g )				( (uint)( ( (g) & 0xff ) << 8 ) )
#define B_TO_COLOR( b )				( (uint)( (b) & 0xff ) )
#define A_TO_COLOR( a )				( (uint)( ( (a) & 0xff ) << 24 ) )
#define SET_COLOR_R( c, r )			( ( ( c ) & 0xff00ffff ) | R_TO_COLOR(r) )
#define SET_COLOR_G( c, g )			( ( ( c ) & 0xffff00ff ) | G_TO_COLOR(g) )
#define SET_COLOR_B( c, b )			( ( ( c ) & 0xffffff00 ) | B_TO_COLOR(b) )
#define SET_COLOR_A( c, a )			( ( ( c ) & 0x00ffffff ) | A_TO_COLOR(a) )
#define RGB_TO_COLOR( r, g, b )		( (uint)( R_TO_COLOR(r) | G_TO_COLOR(g) | B_TO_COLOR(b) ) )
#define RGB_TO_BGRCOLOR( r, g, b )	( (uint)( B_TO_COLOR(r) | G_TO_COLOR(g) | R_TO_COLOR(b) ) )	// Hack for reversed byte ordering
#define ARGB_TO_COLOR( a, r, g, b )	( (uint)( A_TO_COLOR(a) | R_TO_COLOR(r) | G_TO_COLOR(g) | B_TO_COLOR(b) ) )

#endif // COLOR_H