#ifndef MATHCORE_H
#define MATHCORE_H

#include <math.h>	// For sqrtf, etc.
#include <float.h>	// For FLT_MIN/MAX

#define EPSILON 0.0001f
#define SMALLER_EPSILON 1.0e-10f	// ^___________________^;

#define DEGREES_TO_RADIANS( d ) ( ( d ) * 0.01745329252f )
#define RADIANS_TO_DEGREES( r ) ( ( r ) * 57.2957795131f )

#define PI			3.14159265359f
#define TWOPI		6.28318530718f
#define ONEOVERPI	0.31830988618f

const float kAspect_4_3		= 4.0f / 3.0f;
const float kAspect_16_9	= 16.0f / 9.0f;
const float kAspect_16_10	= 16.0f / 10.0f;

#define MAbs( a )			( ( ( a ) < 0 ) ? -( a ) : ( a ) )
#define MMax( a, b )		( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define MMin( a, b )		( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define MClamp( v, l, h )	( ( ( v ) < ( l ) ) ? ( l ) : ( ( v ) > ( h ) ) ? ( h ) : ( v ) )

inline void Swap( char& A, char& B ) { char T = A; A = B; B = T; }

inline unsigned int Min( const unsigned int A, const unsigned int B ) { return ( A < B ) ? A : B; }
inline unsigned int Max( const unsigned int A, const unsigned int B ) { return ( A > B ) ? A : B; }
inline unsigned int Pick( const unsigned int A, const unsigned int B ) { return A != 0 ? A : B; }	// Not a standard operator, just something useful for me
inline unsigned int Clamp( const unsigned int Value, const unsigned int Low, const unsigned int High ) { return ( Value < Low ) ? Low : ( Value > High ) ? High : Value; }
inline void Swap( unsigned int& A, unsigned int& B ) { unsigned int T = A; A = B; B = T; }
inline unsigned int Square( const unsigned int V ) { return V * V; }

inline int Abs( const int A ) { return ( A < 0 ) ? -A : A; }
inline int Min( const int A, const int B ) { return ( A < B ) ? A : B; }
inline int Max( const int A, const int B ) { return ( A > B ) ? A : B; }
inline int Pick( const int A, const int B ) { return A != 0 ? A : B; }	// Not a standard operator, just something useful for me
inline int Clamp( const int Value,const  int Low, const int High ) { return ( Value < Low ) ? Low : ( Value > High ) ? High : Value; }
inline void Swap( int& A, int& B ) { int T = A; A = B; B = T; }
inline int Square( const int V ) { return V * V; }
inline int Mod( const int N, const int D ) { int R = N % D; if( R < 0 ) return R + D; return R; } // Handles negative numbers the way I want, i.e., (-7) % 8 == 1
inline int Sign( const int N ) { return ( N > 0 ) ? 1 : ( ( N < 0 ) ? -1 : 0 ); }

inline float Abs( const float A ) { return ( A < 0.0f ) ? -A : A; }
inline float Min( const float A, const float B ) { return ( A < B ) ? A : B; }
inline float Max( const float A, const float B ) { return ( A > B ) ? A : B; }
inline void MinMax( const float A, const float B, float& OutMin, float& OutMax ) { if( A < B ) { OutMin = A; OutMax = B; } else { OutMin = B; OutMax = A; } }
inline float Pick( const float A, const float B ) { return A != 0.0f ? A : B; }	// Not a standard operator, just something useful for me
inline float Clamp( const float Value, const float Low, const float High ) { return ( Value < Low ) ? Low : ( Value > High ) ? High : Value; }
inline bool Equal( const float A, const float B, const float E = EPSILON ) { return Abs( A - B ) < E; }
inline void Swap( float& A, float& B ) { float T = A; A = B; B = T; }
inline void MinMax( float& A, float& B ) { if ( A > B ) { Swap( A, B ); } }
inline float SqRt( const float V ) { return sqrt( V ); }
inline float Square( const float V ) { return V * V; }
inline float Pow( const float V, const float E ) { return pow( V, E ); }
inline float SignedSquare( const float V ) { return ( V > 0.0f ) ? V * V : -V * V; }
inline float SignedPow( const float V, const float E ) { return Pow( Abs( V ), E ) * ( ( V > 0.0f ) ? 1.0f : -1.0f ); }
inline float Floor( const float F ) { return floor( F ); }
inline float Ceiling( const float F ) { return ceil( F ); }
inline float Mod( const float N, const float D ) { return fmod( N, D ); }
inline float Round( const float F ) { return floor( F + 0.5f ); }
inline float Log( const float F ) { return log( F ); }
inline float Log10( const float F ) { return log10( F ); }
inline float LogBase( const float F, const float Base ) { return Log( F ) / Log( Base ); }
inline float Log2( const float F ) { return LogBase( F, 2.0f ); }
inline float Lerp( const float A, const float B, const float T ) { return ( A + ( T * ( B - A ) ) ); }
inline float InvLerp( const float F, const float A, const float B ) { return ( F - A ) / ( B - A ); }
inline float Hermite( const float A, const float B, const float T ) { return ( A + ( ( T * T * ( 3.0f - 2.0f * T ) ) * ( B - A ) ) ); }
inline float Sign( const float F ) { return ( F > 0.0f ) ? 1.0f : ( ( F < 0.0f ) ? -1.0f : 0.0f ); }
inline float Saturate( const float F ) { return Clamp( F, 0.0f, 1.0f ); }
inline float Attenuate( const float Distance, const float RadiusHalf ) { return RadiusHalf / ( RadiusHalf + Distance ); }

inline void CheckFloatingPointControlWord()
{
#if BUILD_WINDOWS
	uint control_word;
	_controlfp_s( &control_word, 0, 0 );
	//_controlfp_s(&control_word, _CW_DEFAULT, MCW_PC);	// If I ever need it, this resets the FPCW to default
	PRINTF( "FPCW: 0x%08X\n", control_word );
#endif
}

inline float FastInvSqRt( float F )
{
	float Half = 0.5f * F;
	int FAsI = *( int* )&F;
	FAsI = 0x5f3759df - ( FAsI >> 1 );
	F = *( float* )&FAsI;
	F = F * ( 1.5f - Half * F * F );
	//F = F * ( 1.5f - Half * F * F ); // Another iteration of Newton's method can be done to get more precision.
	return F;
}

inline float VolumeToDB( const float V ) { return 20.0f * Log10( V ); }
inline float DBToVolume( const float V ) { return Pow( 10.0f, V / 20.0f ); }
inline float VolumeToMB( const float V ) { return 100.0f * VolumeToDB( V ); }
inline float MBToVolume( const float V ) { return DBToVolume( V / 100.0f ); }

inline float Sin( const float F ) { return sin( F ); }
inline float ASin( const float F ) { return asin( F ); }	// F in [-1,1], returns in [-pi/2,pi/2]
inline float Cos( const float F ) { return cos( F ); }
inline float ACos( const float F ) { return acos( F ); }	// F in [-1,1], returns in [0,pi]
inline float Tan( const float F ) { return tan( F ); }
inline float ATan( const float F ) { return atan( F ); }	// Returns in [-pi/2,pi/2]
inline float ATan2( const float Y, const float X ) { return atan2( Y, X ); }	// Returns in [-pi,pi]

#if BUILD_DEV
inline bool FIsANumber( const float F ) { return ( F == F ); }
inline bool FIsFinite( const float F ) { return ( F >= -FLT_MAX && F <= FLT_MAX ); }
inline bool FIsValid( const float F ) { return FIsANumber( F ) && FIsFinite( F ); }
#endif

inline uint CountBits( uint Value )
{
	uint Count = 0;
	for( ; Value; Count++)
	{
		Value &= Value - 1;
	}
	return Count;
}

#endif // MATHCORE_H