#include "core.h"
#include "noise.h"
#include "mathcore.h"
#include "mathfunc.h"
#include "array.h"

// For cell noise
#include "vector2.h"
#include "vector.h"

#define PROFILE_NOISE 0
#if PROFILE_NOISE
#define PROFILE_NOISE_FUNCTION PROFILE_FUNCTION
#else
#define PROFILE_NOISE_FUNCTION DoNothing
#endif

int Hasher[] =
{
	0xac, 0xb4, 0x6f, 0x94, 0x5c, 0xb8, 0xeb, 0x6c,
	0x47, 0xc0, 0x6d, 0xe6, 0x78, 0xc7, 0x73, 0x84,
	0xa0, 0xd4, 0xc6, 0x68, 0x38, 0xc2, 0x90, 0x53,
	0xd1, 0x40, 0xa9, 0x3d, 0x49, 0x1a, 0x3f, 0x10,
	0x5d, 0x2f, 0x87, 0x24, 0x15, 0x6a, 0xcf, 0x1e,
	0xfc, 0xed, 0xe3, 0x1f, 0xf0, 0x7e, 0xcc, 0xaf,
	0x89, 0x76, 0xc4, 0xfa, 0xa1, 0x29, 0x13, 0x66,
	0xab, 0x1c, 0xb2, 0x05, 0x50, 0x7f, 0xd7, 0xcd,
	0x31, 0x12, 0xe4, 0x69, 0xb3, 0x3e, 0x21, 0x0a,
	0xf9, 0x62, 0x9e, 0x28, 0x59, 0x27, 0x48, 0xd8,
	0xef, 0x41, 0xbc, 0xe8, 0xc8, 0x46, 0x96, 0x86,
	0xaa, 0x9b, 0x37, 0xbd, 0xee, 0x7b, 0xd3, 0x5b,
	0x7c, 0x79, 0x04, 0x1d, 0x35, 0xf2, 0x57, 0x97,
	0xda, 0x44, 0xb0, 0xe7, 0x71, 0xf1, 0xa7, 0x4a,
	0x6b, 0x70, 0xc1, 0xa2, 0x52, 0xc9, 0x16, 0xca,
	0xbe, 0xce, 0xb1, 0xd5, 0xa5, 0xbb, 0xf7, 0xa4,
	0xc5, 0x56, 0x01, 0x9d, 0xcb, 0x1b, 0x9a, 0x08,
	0x43, 0x80, 0x17, 0xde, 0x0d, 0xb5, 0xe2, 0x9c,
	0xd6, 0x74, 0x36, 0x91, 0xbf, 0x09, 0x2e, 0xea,
	0xfb, 0x88, 0xb6, 0x95, 0xae, 0x8f, 0x45, 0xa3,
	0x26, 0xfe, 0x60, 0x51, 0x63, 0x81, 0x34, 0x0b,
	0x5a, 0x42, 0x3b, 0xba, 0x11, 0x07, 0x20, 0xe5,
	0x2d, 0xa8, 0x0e, 0x2c, 0x4b, 0x55, 0x0c, 0x58,
	0x8c, 0x75, 0xd0, 0x5e, 0x4f, 0x98, 0x03, 0xdf,
	0xf3, 0x19, 0x39, 0xec, 0xd9, 0x4c, 0x33, 0x2a,
	0xc3, 0x18, 0x83, 0x3c, 0xa6, 0x32, 0x92, 0x4e,
	0x77, 0x61, 0x06, 0x64, 0x72, 0x02, 0x9f, 0x7d,
	0xe1, 0x2b, 0xdd, 0x8d, 0x8b, 0xfd, 0x00, 0xf8,
	0x14, 0x5f, 0x65, 0xe0, 0x6e, 0xf5, 0x54, 0xdc,
	0x8a, 0xf6, 0xad, 0xe9, 0x7a, 0x25, 0xff, 0xd2,
	0x82, 0x8e, 0x23, 0x0f, 0xb7, 0x67, 0x3a, 0x4d,
	0x85, 0x30, 0xf4, 0xdb, 0x99, 0xb9, 0x22, 0x93,
};

int Hash( const int X )
{
	return Hasher[ X & 255 ];
}

float CubicValues[] =
{
	0.301961f,	-0.482353f,	-0.396078f,	0.286275f,	-0.286275f,	-0.921569f,	0.403922f,	0.513725f,
	0.592157f,	-0.952941f,	0.419608f,	0.074510f,	-0.600000f,	-0.960784f,	-0.129412f,	-0.874510f,
	0.694118f,	0.435294f,	-0.694118f,	0.168627f,	-0.372549f,	0.113725f,	-0.803922f,	-0.474510f,
	0.803922f,	-0.498039f,	0.992157f,	-0.866667f,	0.686275f,	0.121569f,	0.231373f,	0.921569f,
	0.050980f,	-0.709804f,	-0.301961f,	-0.239216f,	-0.356863f,	0.066667f,	0.129412f,	-0.050980f,
	-0.035294f,	-0.145098f,	-0.011765f,	0.584314f,	-0.968627f,	-0.027451f,	0.152941f,	-0.686275f,
	-0.349020f,	-0.741176f,	-0.607843f,	-0.764706f,	-0.490196f,	0.796078f,	-0.811765f,	-0.513725f,
	0.247059f,	-0.725490f,	0.701961f,	-0.984314f,	0.521569f,	-0.192157f,	0.858824f,	0.145098f,
	0.560784f,	0.388235f,	0.058824f,	0.929412f,	0.333333f,	0.176471f,	-0.176471f,	-0.639216f,
	0.003922f,	-0.262745f,	-0.466667f,	0.968627f,	-0.654902f,	-0.882353f,	0.270588f,	-0.184314f,
	0.396078f,	0.458824f,	0.662745f,	0.866667f,	-0.662745f,	-0.168627f,	-0.137255f,	0.913725f,
	0.105882f,	-0.019608f,	0.905882f,	0.262745f,	0.223529f,	-0.796078f,	0.741176f,	-0.066667f,
	-0.631373f,	-0.537255f,	-0.568627f,	-0.835294f,	0.882353f,	-0.858824f,	0.043137f,	-0.121569f,
	0.647059f,	-0.788235f,	0.960784f,	-0.819608f,	-0.309804f,	-0.552941f,	0.498039f,	0.160784f,
	-0.443137f,	0.717647f,	-0.937255f,	-0.529412f,	0.568627f,	0.341176f,	-0.231373f,	-0.215686f,
	0.600000f,	0.952941f,	-0.929412f,	-0.223529f,	0.749020f,	0.325490f,	0.537255f,	-0.278431f,
	0.278431f,	-0.058824f,	-0.380392f,	0.443137f,	0.945098f,	0.654902f,	-0.200000f,	0.035294f,
	-0.913725f,	-0.898039f,	-0.160784f,	-0.098039f,	-0.905882f,	-0.670588f,	0.349020f,	-0.450980f,
	0.364706f,	0.545098f,	0.317647f,	0.082353f,	-0.105882f,	0.937255f,	0.184314f,	0.850980f,
	0.098039f,	-0.247059f,	0.984314f,	0.725490f,	0.678431f,	0.607843f,	-0.388235f,	0.709804f,
	0.835294f,	-0.647059f,	0.843137f,	1.000000f,	-0.207843f,	-0.890196f,	0.294118f,	0.874510f,
	-0.545098f,	-0.992157f,	-0.678431f,	0.090196f,	0.772549f,	0.019608f,	-0.152941f,	-1.000000f,
	-0.270588f,	-0.717647f,	0.756863f,	-0.043137f,	0.137255f,	-0.325490f,	-0.333333f,	0.615686f,
	-0.945098f,	0.733333f,	-0.294118f,	-0.074510f,	0.639216f,	-0.521569f,	-0.827451f,	-0.584314f,
	0.788235f,	-0.505882f,	-0.419608f,	-0.113725f,	0.631373f,	-0.411765f,	-0.701961f,	0.827451f,
	0.490196f,	-0.254902f,	-0.317647f,	-0.623529f,	-0.090196f,	0.670588f,	0.215686f,	-0.976471f,
	0.356863f,	-0.615686f,	0.819608f,	-0.560784f,	0.372549f,	0.309804f,	0.450980f,	-0.780392f,
	-0.576471f,	-0.435294f,	0.427451f,	-0.592157f,	0.239216f,	-0.427451f,	-0.364706f,	-0.403922f,
	-0.733333f,	-0.772549f,	-0.850980f,	-0.843137f,	-0.756863f,	0.192157f,	0.623529f,	0.505882f,
	0.890196f,	-0.003922f,	0.529412f,	0.254902f,	-0.458824f,	0.482353f,	-0.082353f,	0.576471f,
	0.976471f,	-0.341176f,	0.200000f,	0.207843f,	0.466667f,	0.552941f,	0.898039f,	-0.749020f,
	0.764706f,	0.411765f,	0.380392f,	0.011765f,	0.811765f,	0.474510f,	0.780392f,	0.027451f,
};

void Noise::ShuffleHasher()
{
	for( int Index = 0; Index < 256; ++Index )
	{
		int OtherIndex = Math::Random( 0, 255 );
		Swap( Hasher[ Index ], Hasher[ OtherIndex ] );
	}
}

void Noise::ShuffleCubicValues()
{
	for( int Index = 0; Index < 256; ++Index )
	{
		int OtherIndex = Math::Random( 0, 255 );
		Swap( CubicValues[ Index ], CubicValues[ OtherIndex ] );
	}
}

float FadeCubic( float t )
{
	// Original Perlin noise:
	return t * t * ( 3.0f - 2.0f * t );
}

float Noise::CubicNoise1( float X )
{
	ASSERT( CubicValues );

	int UnitX = ( int )Floor( X ) & 255;

	X -= Floor( X );

	const float U = FadeCubic( X );

	const int A = Hash( UnitX );
	const int B = Hash( UnitX + 1 );

	return Lerp( CubicValues[ A ], CubicValues[ B ], U );
}

float Noise::CubicNoise2( float X, float Y )
{
	PROFILE_NOISE_FUNCTION;

	ASSERT( CubicValues );

	int UnitX = ( int )Floor( X ) & 255;
	int UnitY = ( int )Floor( Y ) & 255;

	X -= Floor( X );
	Y -= Floor( Y );

	const float U = FadeCubic( X );
	const float V = FadeCubic( Y );

	const int AA = Hash( Hash( UnitX ) + UnitY );
	const int AB = Hash( Hash( UnitX + 1 ) + UnitY );
	const int BA = Hash( Hash( UnitX ) + UnitY + 1 );
	const int BB = Hash( Hash( UnitX + 1 ) + UnitY + 1 );

	return
		Lerp(
			Lerp( CubicValues[ AA ], CubicValues[ AB ], U ),
			Lerp( CubicValues[ BA ], CubicValues[ BB ], U ),
			V );
}

float Noise::CubicNoise3( float X, float Y, float Z )
{
	ASSERT( CubicValues );

	int UnitX = ( int )Floor( X ) & 255;
	int UnitY = ( int )Floor( Y ) & 255;
	int UnitZ = ( int )Floor( Z ) & 255;

	X -= Floor( X );
	Y -= Floor( Y );
	Z -= Floor( Z );

	const float U = FadeCubic( X );
	const float V = FadeCubic( Y );
	const float W = FadeCubic( Z );

	const int AAA = Hash( Hash( Hash( UnitX ) + UnitY ) + UnitZ );
	const int AAB = Hash( Hash( Hash( UnitX + 1 ) + UnitY ) + UnitZ );
	const int ABA = Hash( Hash( Hash( UnitX ) + UnitY + 1 ) + UnitZ );
	const int ABB = Hash( Hash( Hash( UnitX + 1 ) + UnitY + 1 ) + UnitZ );
	const int BAA = Hash( Hash( Hash( UnitX ) + UnitY ) + UnitZ + 1 );
	const int BAB = Hash( Hash( Hash( UnitX + 1 ) + UnitY ) + UnitZ + 1 );
	const int BBA = Hash( Hash( Hash( UnitX ) + UnitY + 1 ) + UnitZ + 1 );
	const int BBB = Hash( Hash( Hash( UnitX + 1 ) + UnitY + 1 ) + UnitZ + 1 );

	return
		Lerp(
			Lerp(
				Lerp( CubicValues[ AAA ], CubicValues[ AAB ], U ),
				Lerp( CubicValues[ ABA ], CubicValues[ ABB ], U ),
				V ),
			Lerp(
				Lerp( CubicValues[ BAA ], CubicValues[ BAB ], U ),
				Lerp( CubicValues[ BBA ], CubicValues[ BBB ], U ),
				V ),
			W );
}

int Noise::IntNoise1( int X )
{
	// Period of 64k.
	int Lo16X = X & 0xffff;

	int ValueX1 = Hash( Hash( Lo16X & 255 ) + ( Lo16X >> 8 ) );
	int ValueX2 = Hash( Hash( ( Lo16X >> 8 ) ) + Lo16X & 255 );

	return ValueX1 + ( ValueX2 << 8 );
}

int Noise::IntNoise2( int X, int Y )
{
	// Period of 64k in each dimension.
	int Lo16X = X & 0xffff;
	int Lo16Y = Y & 0xffff;

	int ValueX1 = Hash( Hash( Lo16X & 255 ) + ( Lo16X >> 8 ) );
	int ValueY1 = Hash( Hash( Lo16Y & 255 ) + ( Lo16Y >> 8 ) );
	int Value1 = Hash( Hash( ValueX1 ) + ValueY1 );

	int ValueX2 = Hash( Hash( ( Lo16X >> 8 ) ) + Lo16X & 255 );
	int ValueY2 = Hash( Hash( ( Lo16Y >> 8 ) ) + Lo16Y & 255 );
	int Value2 = Hash( Hash( ValueX2 ) + ValueY2 );

	return Value1 + ( Value2 << 8 );
}

int Noise::IntNoise3( int X, int Y, int Z )
{
	// Period of 64k in each dimension.
	int Lo16X = X & 0xffff;
	int Lo16Y = Y & 0xffff;
	int Lo16Z = Z & 0xffff;

	int ValueX1 = Hash( Hash( Lo16X & 255 ) + ( Lo16X >> 8 ) );
	int ValueY1 = Hash( Hash( Lo16Y & 255 ) + ( Lo16Y >> 8 ) );
	int ValueZ1 = Hash( Hash( Lo16Z & 255 ) + ( Lo16Z >> 8 ) );
	int Value1 = Hash( Hash( Hash( ValueX1 ) + ValueY1 ) + ValueZ1 );

	int ValueX2 = Hash( Hash( ( Lo16X >> 8 ) ) + Lo16X & 255 );
	int ValueY2 = Hash( Hash( ( Lo16Y >> 8 ) ) + Lo16Y & 255 );
	int ValueZ2 = Hash( Hash( ( Lo16Z >> 8 ) ) + Lo16Z & 255 );
	int Value2 = Hash( Hash( Hash( ValueX2 ) + ValueY2 ) + ValueZ2 );

	return Value1 + ( Value2 << 8 );
}

//********************************************************
// Custom cellular noise reusing cubic values and hashing.

// Look up value and transform range -1..1 to 0..1
float GetCell( const int Index )
{
	DEBUGASSERT( Index >= 0 && Index <= 255 );
	return 0.5f * ( 1.0f + CubicValues[ Index ] );
}

float Min3( const float A, const float B, const float C )
{
	return Min( Min( A, B ), C );
}

float Max3( const float A, const float B, const float C )
{
	return Max( Max( A, B ), C );
}

float GetOffsetAtCell1( const int X )
{
	const int A = Hash( X );

	return GetCell( A );
}

Vector2 GetOffsetAtCell2( const int X, const int Y )
{
	const int A = Hash( Hash( Hash( 0 ) + X ) + Y );
	const int B = Hash( Hash( Hash( 1 ) + X ) + Y );

	return Vector2( GetCell( A ), GetCell( B ) );
}

Vector GetOffsetAtCell3( const int X, const int Y, const int Z )
{
	const int A = Hash( Hash( Hash( Hash( 0 ) + X ) + Y ) + Z );
	const int B = Hash( Hash( Hash( Hash( 1 ) + X ) + Y ) + Z );
	const int C = Hash( Hash( Hash( Hash( 2 ) + X ) + Y ) + Z );

	return Vector( GetCell( A ), GetCell( B ), GetCell( C ) );
}

float Noise::CellNoise1( float X )
{
	ASSERT( CubicValues );

	int UnitX = ( int )Floor( X ) & 255;

	X -= Floor( X );

	// Check local cube and each neighbor to find nearest point.
	// Add the offset to the neighbor as needed
	const float A = GetOffsetAtCell1( UnitX );				// Range [0,1]
	const float B = GetOffsetAtCell1( UnitX + 1 ) + 1.0f;	// Range [1,2]
	const float C = GetOffsetAtCell1( UnitX - 1 ) - 1.0f;	// Range [-1,0]

	// Now find the nearest of these values to X.
	// NOTE: I could use Square instead of Abs() to conform, if I ever decide
	// to use LengthSquared() function for performance in higher dimensions.

	Array<float> Dists;
	Dists.Resize( 3 );

	Dists[0] = Abs( X - A );
	Dists[1] = Abs( X - B );
	Dists[2] = Abs( X - C );

	Dists.InsertionSort();

	// Dists[0] (or 1.0f - Dists[0]) is the basic cellular noise function.
	// Other functions can produce more defined shapes.
	// Dists[0] in range 0..1.
	// Dists[1] in range 0..2 (although generally much smaller than 2).
	// So this has output in range 0..2, but generally closer to 0..1.
	// The client should scale and clamp the value for best results.
	return Dists[1] - Dists[0];
}

float Noise::CellNoise2( float X, float Y )
{
	PROFILE_NOISE_FUNCTION;

	ASSERT( CubicValues );

	int UnitX = ( int )Floor( X ) & 255;
	int UnitY = ( int )Floor( Y ) & 255;

	X -= Floor( X );
	Y -= Floor( Y );

	const Vector2 Local( X, Y );

	const Vector2 AA = GetOffsetAtCell2( UnitX, UnitY );
	const Vector2 AB = GetOffsetAtCell2( UnitX + 1, UnitY ) + Vector2( 1.0f, 0.0f );
	const Vector2 AC = GetOffsetAtCell2( UnitX - 1, UnitY ) + Vector2( -1.0f, 0.0f );
	const Vector2 BA = GetOffsetAtCell2( UnitX, UnitY + 1 ) + Vector2( 0.0f, 1.0f );
	const Vector2 BB = GetOffsetAtCell2( UnitX + 1, UnitY + 1 ) + Vector2( 1.0f, 1.0f );
	const Vector2 BC = GetOffsetAtCell2( UnitX - 1, UnitY + 1 ) + Vector2( -1.0f, 1.0f );
	const Vector2 CA = GetOffsetAtCell2( UnitX, UnitY - 1 ) + Vector2( 0.0f, -1.0f );
	const Vector2 CB = GetOffsetAtCell2( UnitX + 1, UnitY - 1 ) + Vector2( 1.0f, -1.0f );
	const Vector2 CC = GetOffsetAtCell2( UnitX - 1, UnitY - 1 ) + Vector2( -1.0f, -1.0f );

	Array<float> Dists;
	Dists.Resize( 9 );

	Dists[0] = ( Local - AA ).Length();
	Dists[1] = ( Local - AB ).Length();
	Dists[2] = ( Local - AC ).Length();
	Dists[3] = ( Local - BA ).Length();
	Dists[4] = ( Local - BB ).Length();
	Dists[5] = ( Local - BC ).Length();
	Dists[6] = ( Local - CA ).Length();
	Dists[7] = ( Local - CB ).Length();
	Dists[8] = ( Local - CC ).Length();

	Dists.InsertionSort();

	return Dists[1] - Dists[0];
}

float Noise::CellNoise3( float X, float Y, float Z )
{
	ASSERT( CubicValues );

	int UnitX = ( int )Floor( X ) & 255;
	int UnitY = ( int )Floor( Y ) & 255;
	int UnitZ = ( int )Floor( Z ) & 255;

	X -= Floor( X );
	Y -= Floor( Y );
	Z -= Floor( Z );

	const Vector Local( X, Y, Z );

	const Vector AAA = GetOffsetAtCell3( UnitX, UnitY, UnitZ );
	const Vector AAB = GetOffsetAtCell3( UnitX + 1, UnitY, UnitZ ) + Vector( 1.0f, 0.0f, 0.0f );
	const Vector AAC = GetOffsetAtCell3( UnitX - 1, UnitY, UnitZ ) + Vector( -1.0f, 0.0f, 0.0f );
	const Vector ABA = GetOffsetAtCell3( UnitX, UnitY + 1, UnitZ ) + Vector( 0.0f, 1.0f, 0.0f );
	const Vector ABB = GetOffsetAtCell3( UnitX + 1, UnitY + 1, UnitZ ) + Vector( 1.0f, 1.0f, 0.0f );
	const Vector ABC = GetOffsetAtCell3( UnitX - 1, UnitY + 1, UnitZ ) + Vector( -1.0f, 1.0f, 0.0f );
	const Vector ACA = GetOffsetAtCell3( UnitX, UnitY - 1, UnitZ ) + Vector( 0.0f, -1.0f, 0.0f );
	const Vector ACB = GetOffsetAtCell3( UnitX + 1, UnitY - 1, UnitZ ) + Vector( 1.0f, -1.0f, 0.0f );
	const Vector ACC = GetOffsetAtCell3( UnitX - 1, UnitY - 1, UnitZ ) + Vector( -1.0f, -1.0f, 0.0f );
	const Vector BAA = GetOffsetAtCell3( UnitX, UnitY, UnitZ + 1 ) + Vector( 0.0f, 0.0f, 1.0f );
	const Vector BAB = GetOffsetAtCell3( UnitX + 1, UnitY, UnitZ + 1 ) + Vector( 1.0f, 0.0f, 1.0f );
	const Vector BAC = GetOffsetAtCell3( UnitX - 1, UnitY, UnitZ + 1 ) + Vector( -1.0f, 0.0f, 1.0f );
	const Vector BBA = GetOffsetAtCell3( UnitX, UnitY + 1, UnitZ + 1 ) + Vector( 0.0f, 1.0f, 1.0f );
	const Vector BBB = GetOffsetAtCell3( UnitX + 1, UnitY + 1, UnitZ + 1 ) + Vector( 1.0f, 1.0f, 1.0f );
	const Vector BBC = GetOffsetAtCell3( UnitX - 1, UnitY + 1, UnitZ + 1 ) + Vector( -1.0f, 1.0f, 1.0f );
	const Vector BCA = GetOffsetAtCell3( UnitX, UnitY - 1, UnitZ + 1 ) + Vector( 0.0f, -1.0f, 1.0f );
	const Vector BCB = GetOffsetAtCell3( UnitX + 1, UnitY - 1, UnitZ + 1 ) + Vector( 1.0f, -1.0f, 1.0f );
	const Vector BCC = GetOffsetAtCell3( UnitX - 1, UnitY - 1, UnitZ + 1 ) + Vector( -1.0f, -1.0f, 1.0f );
	const Vector CAA = GetOffsetAtCell3( UnitX, UnitY, UnitZ - 1 ) + Vector( 0.0f, 0.0f, -1.0f );
	const Vector CAB = GetOffsetAtCell3( UnitX + 1, UnitY, UnitZ - 1 ) + Vector( 1.0f, 0.0f, -1.0f );
	const Vector CAC = GetOffsetAtCell3( UnitX - 1, UnitY, UnitZ - 1 ) + Vector( -1.0f, 0.0f, -1.0f );
	const Vector CBA = GetOffsetAtCell3( UnitX, UnitY + 1, UnitZ - 1 ) + Vector( 0.0f, 1.0f, -1.0f );
	const Vector CBB = GetOffsetAtCell3( UnitX + 1, UnitY + 1, UnitZ - 1 ) + Vector( 1.0f, 1.0f, -1.0f );
	const Vector CBC = GetOffsetAtCell3( UnitX - 1, UnitY + 1, UnitZ - 1 ) + Vector( -1.0f, 1.0f, -1.0f );
	const Vector CCA = GetOffsetAtCell3( UnitX, UnitY - 1, UnitZ - 1 ) + Vector( 0.0f, -1.0f, -1.0f );
	const Vector CCB = GetOffsetAtCell3( UnitX + 1, UnitY - 1, UnitZ - 1 ) + Vector( 1.0f, -1.0f, -1.0f );
	const Vector CCC = GetOffsetAtCell3( UnitX - 1, UnitY - 1, UnitZ - 1 ) + Vector( -1.0f, -1.0f, -1.0f );

	Array<float> Dists;
	Dists.Resize( 27 );

	Dists[0 ] = ( Local - AAA ).Length();
	Dists[1 ] = ( Local - AAB ).Length();
	Dists[2 ] = ( Local - AAC ).Length();
	Dists[3 ] = ( Local - ABA ).Length();
	Dists[4 ] = ( Local - ABB ).Length();
	Dists[5 ] = ( Local - ABC ).Length();
	Dists[6 ] = ( Local - ACA ).Length();
	Dists[7 ] = ( Local - ACB ).Length();
	Dists[8 ] = ( Local - ACC ).Length();
	Dists[9 ] = ( Local - BAA ).Length();
	Dists[10] = ( Local - BAB ).Length();
	Dists[11] = ( Local - BAC ).Length();
	Dists[12] = ( Local - BBA ).Length();
	Dists[13] = ( Local - BBB ).Length();
	Dists[14] = ( Local - BBC ).Length();
	Dists[15] = ( Local - BCA ).Length();
	Dists[16] = ( Local - BCB ).Length();
	Dists[17] = ( Local - BCC ).Length();
	Dists[18] = ( Local - CAA ).Length();
	Dists[19] = ( Local - CAB ).Length();
	Dists[20] = ( Local - CAC ).Length();
	Dists[21] = ( Local - CBA ).Length();
	Dists[22] = ( Local - CBB ).Length();
	Dists[23] = ( Local - CBC ).Length();
	Dists[24] = ( Local - CCA ).Length();
	Dists[25] = ( Local - CCB ).Length();
	Dists[26] = ( Local - CCC ).Length();

	Dists.InsertionSort();

	return Dists[1] - Dists[0];
}

//********************************************************
// Custom grid noise reusing int noise.

float Noise::GridNoise1( float X )
{
	PROFILE_NOISE_FUNCTION;

	const int UnitX = ( int )Floor( X ) & 255;

	X -= Floor( X );

	// Range 0..65535
	const int IntNoise = IntNoise1( UnitX );

	// TODO: Determine a better metric for which edges are active on this grid?
	return ( IntNoise & 1 ) ? 2.0f * ( 0.5f - Abs( 0.5f - X ) ) : 0.0f;
}

float Noise::GridNoise2( float X, float Y )
{
	PROFILE_NOISE_FUNCTION;

	const int UnitX = ( int )Floor( X ) & 255;
	const int UnitY = ( int )Floor( Y ) & 255;

	X -= Floor( X );
	Y -= Floor( Y );

	// Range 0..65535
	const int IntNoise = IntNoise2( UnitX, UnitY );

	// TODO: Determine a better metric for which edges are active on this grid?
	const float ValueX = ( IntNoise & 1 ) ? 2.0f * ( 0.5f - Abs( 0.5f - X ) ) : 0.0f;
	const float ValueY = ( IntNoise & 2 ) ? 2.0f * ( 0.5f - Abs( 0.5f - Y ) ) : 0.0f;

	return Max( ValueX, ValueY );
}

float Noise::GridNoise3( float X, float Y, float Z )
{
	PROFILE_NOISE_FUNCTION;

	const int UnitX = ( int )Floor( X ) & 255;
	const int UnitY = ( int )Floor( Y ) & 255;
	const int UnitZ = ( int )Floor( Z ) & 255;

	X -= Floor( X );
	Y -= Floor( Y );
	Z -= Floor( Z );

	// Range 0..65535
	const int IntNoise = IntNoise3( UnitX, UnitY, UnitZ );

	// TODO: Determine a better metric for which edges are active on this grid?
	const float ValueX = ( IntNoise & 1 ) ? 2.0f * ( 0.5f - Abs( 0.5f - X ) ) : 0.0f;
	const float ValueY = ( IntNoise & 2 ) ? 2.0f * ( 0.5f - Abs( 0.5f - Y ) ) : 0.0f;
	const float ValueZ = ( IntNoise & 4 ) ? 2.0f * ( 0.5f - Abs( 0.5f - Z ) ) : 0.0f;

	return Max3( ValueX, ValueY, ValueZ );
}

//********************************************************
// Helper functions to get more complex noise from fundamental functions.

float Noise::SumNoise1( float x, uint Octaves, Noise1Func NoiseFunc )
{
	float Sum = 0.0f;
	float NoiseScale = 1.0f;

	// Octave control vars--expose to client?
	static const float OctaveSizeRatio = 2.0f;
	static const float OctaveWeightRatio = 0.5f; //(typically 1/OctaveSizeRatio)

	for( uint i = 0; i < Octaves; ++i )
	{
		Sum += NoiseFunc( x ) * NoiseScale;
		x *= OctaveSizeRatio;
		NoiseScale *= OctaveWeightRatio;
	}

	return Sum;
}

float Noise::SumNoise2( float x, float y, uint Octaves, Noise2Func NoiseFunc )
{
	float Sum = 0.0f;
	float NoiseScale = 1.0f;

	// Octave control vars--expose to client?
	static const float OctaveSizeRatio = 2.0f;
	static const float OctaveWeightRatio = 0.5f; //(typically 1/OctaveSizeRatio)

	for( uint i = 0; i < Octaves; ++i )
	{
		Sum += NoiseFunc( x, y ) * NoiseScale;
		x *= OctaveSizeRatio;
		y *= OctaveSizeRatio;
		NoiseScale *= OctaveWeightRatio;
	}

	return Sum;
}

float Noise::SumNoise3( float x, float y, float z, uint Octaves, Noise3Func NoiseFunc )
{
	float Sum = 0.0f;
	float NoiseScale = 1.0f;

	// Octave control vars--expose to client?
	static const float OctaveSizeRatio = 2.0f;
	static const float OctaveWeightRatio = 0.5f; //(typically 1/OctaveSizeRatio)

	for( uint i = 0; i < Octaves; ++i )
	{
		Sum += NoiseFunc( x, y, z ) * NoiseScale;
		x *= OctaveSizeRatio;
		y *= OctaveSizeRatio;
		z *= OctaveSizeRatio;
		NoiseScale *= OctaveWeightRatio;
	}

	return Sum;
}

// Helper function for clients
float Noise::GetOctaveScale( uint Octaves )
{
	float Sum = 0.0f;
	float Scale = 1.0f;

	// Octave control vars--expose to client?
	float OctaveWeightRatio = 0.5f;

	for( uint i = 0; i < Octaves; ++i )
	{
		Sum += Scale;
		Scale *= OctaveWeightRatio;
	}

	return Sum;
}

float Noise::SumAbsNoise1( float x, uint Octaves, Noise1Func NoiseFunc )
{
	float Sum = 0.0f;
	float NoiseScale = 1.0f;

	// Octave control vars--expose to client?
	static const float OctaveSizeRatio = 2.0f;
	static const float OctaveWeightRatio = 0.5f; //(typically 1/OctaveSizeRatio)

	for( uint i = 0; i < Octaves; ++i )
	{
		Sum += Abs( NoiseFunc( x ) ) * NoiseScale;
		x *= OctaveSizeRatio;
		NoiseScale *= OctaveWeightRatio;
	}

	return Sum;
}

float Noise::SumAbsNoise2( float x, float y, uint Octaves, Noise2Func NoiseFunc )
{
	float Sum = 0.0f;
	float NoiseScale = 1.0f;

	// Octave control vars--expose to client?
	static const float OctaveSizeRatio = 2.0f;
	static const float OctaveWeightRatio = 0.5f; //(typically 1/OctaveSizeRatio)

	for( uint i = 0; i < Octaves; ++i )
	{
		Sum += Abs( NoiseFunc( x, y ) ) * NoiseScale;
		x *= OctaveSizeRatio;
		y *= OctaveSizeRatio;
		NoiseScale *= OctaveWeightRatio;
	}

	return Sum;
}

float Noise::SumAbsNoise3( float x, float y, float z, uint Octaves, Noise3Func NoiseFunc )
{
	float Sum = 0.0f;
	float NoiseScale = 1.0f;

	// Octave control vars--expose to client?
	static const float OctaveSizeRatio = 2.0f;
	static const float OctaveWeightRatio = 0.5f; //(typically 1/OctaveSizeRatio)

	for( uint i = 0; i < Octaves; ++i )
	{
		Sum += Abs( NoiseFunc( x, y, z ) ) * NoiseScale;
		x *= OctaveSizeRatio;
		y *= OctaveSizeRatio;
		z *= OctaveSizeRatio;
		NoiseScale *= OctaveWeightRatio;
	}

	return Sum;
}

float Fade( float t )
{
	// Improved Perlin noise: 6*t^5 - 15*t^4 + 10*t^3 (instead of the original fixed Hermite curve 3*t^2 - 2*t^3)
	return t * t * t * ( t * ( t * 6.0f - 15.0f ) + 10.0f );
}

// I didn't derive this one, it's based on http://webstaff.itn.liu.se/~stegu/aqsis/aqsis-newnoise/noise1234.cpp
// Also, I never tested this.
static float Grad1( int hash, float x )
{
	int h = hash & 15;
	float u = x * (1.0f + (h&7));
	return ((h&8) ? -u : u);
}

// Improved Perlin noise: fixed Gradients instead of pseudorandom Gradients
// Understanding Perlin noise: At this point, x and y are in the range 0..1.
// They are the fractional part of the input coordinate for the Noise function.
// A random value (the "hash", which is derived from the integral part of the
// input coordinate after several lookups in the random table) is used to select
// a Gradient, which is a directional slope. This function implicitly returns
// the dot product of the fractional part *with* that Gradient, which is the
// secret sauce I had been unable to figure out. (So there's no slope involved,
// the Gradient doesn't represent a directed rise; it's just a direction and
// the value is computed by the dot product of the input and that direction.)
// Then we lerp between the value produced by different Gradients at different
// points, and that produces a continuous value.
static float Grad2( int hash, float x, float y )
{
	// Convert low 3 bits of hash code into 8 Gradient directions (8 cardinal directions rotated 22.5 degrees)
	int h = hash & 7;
	float u = h<4 ? x : y;
	float v = h<4 ? y : x;
	return ((h&1) ? -u : u) + 0.5f*((h&2) ? -v : v);
}

// Improved Perlin noise: fixed Gradients instead of pseudorandom Gradients
// DLPNOTE: I've named this Grad3, because to do a proper 1D or 2D noise implementation,
// we'd need equivalent 1D/2D Gradients (similarly built not along axes and not along diagonals)
static float Grad3( int hash, float x, float y, float z )
{
	int h = hash & 15;		// CONVERT LO 4 BITS OF HASH CODE
	float u = h<8 ? x : y;	// INTO 12 GradIENT DIRECTIONS.
	float v = h<4 ? y : h==12||h==14 ? x : z;
	return ((h&1) ? -u : u) + ((h&2) ? -v : v);
}

// I never tested this, no idea if it works.
float Noise::Noise1( float x )
{
	// DLPNOTE: Quick hack
	//return Noise3( x, 0.0f, 0.0f );

	int X = (int)Floor(x) & 255;

	x -= Floor(x);

	float u = Fade(x);

	int A = Hash(X  ), AA = Hash(A),
		B = Hash(X+1), BA = Hash(B);

	return
		Lerp(
			Grad1(Hash(AA), x   ),
			Grad1(Hash(BA), x-1 ),
			u);
}

float Noise::Noise2( float x, float y )
{
	// DLPNOTE: Quick hack
	//return Noise3( x, y, 0.0f );

	int X = (int)Floor(x) & 255;
	int Y = (int)Floor(y) & 255;

	x -= Floor(x);
	y -= Floor(y);

	float u = Fade(x);
	float v = Fade(y);

	int A = Hash(X  )+Y, AA = Hash(A), AB = Hash(A+1),
		B = Hash(X+1)+Y, BA = Hash(B), BB = Hash(B+1);

	return
		Lerp(
			Lerp(
				Grad2(Hash(AA), x  , y  ),
				Grad2(Hash(BA), x-1, y  ),
				u),
			Lerp(
				Grad2(Hash(AB), x  , y-1),
				Grad2(Hash(BB), x-1, y-1),
				u),
			v);
}

float Noise::Noise3( float x, float y, float z )
{
	int X = (int)Floor(x) & 255;
	int Y = (int)Floor(y) & 255;
	int Z = (int)Floor(z) & 255;

	x -= Floor(x);
	y -= Floor(y);
	z -= Floor(z);

	float u = Fade(x);
	float v = Fade(y);
	float w = Fade(z);

	int A = Hash(X  )+Y, AA = Hash(A)+Z, AB = Hash(A+1)+Z,
		B = Hash(X+1)+Y, BA = Hash(B)+Z, BB = Hash(B+1)+Z;

	return
		Lerp(
			Lerp(
				Lerp(
					Grad3(Hash(AA  ), x  , y  , z   ),
					Grad3(Hash(BA  ), x-1, y  , z   ),
					u),
				Lerp(
					Grad3(Hash(AB  ), x  , y-1, z   ),
					Grad3(Hash(BB  ), x-1, y-1, z   ),
					u),
				v),
			Lerp(
				Lerp(
					Grad3(Hash(AA+1), x  , y  , z-1 ),
					Grad3(Hash(BA+1), x-1, y  , z-1 ),
					u),
				Lerp(
					Grad3(Hash(AB+1), x  , y-1, z-1 ),
					Grad3(Hash(BB+1), x-1, y-1, z-1 ),
					u),
				v),
			w);
}