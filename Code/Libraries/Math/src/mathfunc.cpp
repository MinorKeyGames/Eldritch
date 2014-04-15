#include "core.h"
#include "mathfunc.h"
#include "mathcore.h"
#include "random.h"
#include <math.h>

static Random Generator;

void Math::Bresenham( int x0, int y0, int x1, int y1, BresenhamPointFunc PointFunc, void* pContext )
{
	bool steep = abs( y1 - y0 ) > abs( x1 - x0 );	// Is the slope greater than 1?
	if( steep )
	{	// If so, swap all x and y values throughout algorithm
		int t = x0;
		x0 = y0;
		y0 = t;
		t = x1;
		x1 = y1;
		y1 = t;
	}

	int deltax	= abs( x1 - x0 );
	int deltay	= abs( y1 - y0 );
	int error	= 0;
	int xstep	= ( x0 < x1 ) ? 1 : -1;
	int ystep	= ( y0 < y1 ) ? 1 : -1;
	int y		= y0;

	bool Break = false;
	for( int x = x0; ( ( x0 < x1 ) && ( x <= x1 ) || ( x0 > x1 ) && ( x >= x1 ) ); x += xstep )
	{
		if( steep )
		{
			PointFunc( y, x, Break, pContext );
		}
		else
		{
			PointFunc( x, y, Break, pContext );
		}

		if( Break )
		{
			return;
		}

		error += deltay;
		if( error << 1 >= deltax )
		{
			y += ystep;
			error -= deltax;
		}
	}
}

void Math::Bresenham3( int x0, int y0, int z0, int x1, int y1, int z1, Bresenham3PointFunc PointFunc, void* pContext )
{
	bool SteepXY = Abs( y1 - y0 ) > Abs( x1 - x0 );	// Is the slope greater than 1?
	if( SteepXY )									// If so, swap all x and y values throughout algorithm
	{
		Swap( x0, y0 );
		Swap( x1, y1 );
	}

	bool SteepXZ = Abs( z1 - z0 ) > Abs( x1 - x0 );	// Is the slope greater than 1?
	if( SteepXZ )									// If so, swap all x and z values throughout algorithm
	{
		Swap( x0, z0 );
		Swap( x1, z1 );
	}

	int deltax	= Abs( x1 - x0 );
	int deltay	= Abs( y1 - y0 );
	int deltaz	= Abs( z1 - z0 );
	int errory	= 0;
	int errorz	= 0;
	int xstep	= ( x0 < x1 ) ? 1 : -1;
	int ystep	= ( y0 < y1 ) ? 1 : -1;
	int	zstep	= ( z0 < z1 ) ? 1 : -1;
	int y		= y0;
	int z		= z0;

	bool Break = false;
	for( int x = x0; ( ( x0 < x1 ) && ( x <= x1 ) || ( x0 > x1 ) && ( x >= x1 ) ); x += xstep )
	{
		if( SteepXY )
		{
			if( SteepXZ )
			{
				PointFunc( z, x, y, Break, pContext );
			}
			else
			{
				PointFunc( y, x, z, Break, pContext );
			}
		}
		else
		{
			if( SteepXZ )
			{
				PointFunc( z, y, x, Break, pContext );
			}
			else
			{
				PointFunc( x, y, z, Break, pContext );
			}
		}

		if( Break )
		{
			return;
		}

		errory += deltay;
		errorz += deltaz;

		if( errory << 1 >= deltax )
		{
			y += ystep;
			errory -= deltax;
		}

		if( errorz << 1 >= deltax )
		{
			z += zstep;
			errorz -= deltax;
		}
	}
}

bool Math::RandomF( float Chance )
{
	// This ensures that Chance = 1.0 will *always* return true,
	// and Chance = 0.0 will *always* return false. Edge cases!
	return ( Chance >= 1.0f ) || Random( 0.0f, 1.0f ) < Chance;
}

uint Math::Random( uint Range )
{
	DEBUGASSERT( Range > 0 );
	return Generator.Get( Range );
}

uint Math::Random( uint Min, uint Max )
{
	DEBUGASSERT( Max >= Min );
	uint Range = Max - Min;
	return Min + Generator.Get( 1 + Range );
}

int Math::Random( int Min, int Max )
{
	DEBUGASSERT( Max >= Min );
	int Range = Max - Min;
	return Min + Generator.Get( 1 + Range );
}

float Math::Random( float Min, float Max )
{
#define RANDOM_PRECISION 10000000
	DEBUGASSERT( Max >= Min );
	float Range = Max - Min;
	float Random = (float)( Generator.Get( 1 + RANDOM_PRECISION ) );
	return ( ( Random / RANDOM_PRECISION ) * Range ) + Min;
}

Vector2 Math::Random( const Vector2& Min, const Vector2& Max )
{
	return Vector2(
		Random( Min.x, Max.x ),
		Random( Min.y, Max.y ) );
}

Vector Math::Random( const Vector& Min, const Vector& Max )
{
	return Vector(
		Random( Min.x, Max.x ),
		Random( Min.y, Max.y ),
		Random( Min.z, Max.z ) );
}

Vector4 Math::Random( const Vector4& Min, const Vector4& Max )
{
	return Vector4(
		Random( Min.x, Max.x ),
		Random( Min.y, Max.y ),
		Random( Min.z, Max.z ),
		Random( Min.w, Max.w ) );
}

Angles Math::Random( const Angles& Min, const Angles& Max )
{
	return Angles(
		Random( Min.Pitch, Max.Pitch ),
		Random( Min.Roll, Max.Roll ),
		Random( Min.Yaw, Max.Yaw ) );
}

Color Math::Random( const Color& Min, const Color& Max )
{
	return Color(
		(byte)Random( Min.A, Max.A ),
		(byte)Random( Min.R, Max.R ),
		(byte)Random( Min.G, Max.G ),
		(byte)Random( Min.B, Max.B ) );
}

bool Math::Random()
{
#define RANDOM_RANGE 0x0000ffff
	return 0 == ( Generator.Get( RANDOM_RANGE ) & 0x00000001 );
}

void Math::SeedGenerator()
{
	Generator.Seed();
}

void Math::SeedGenerator( uint Seed )
{
	Generator.SetRandomSeed( Seed );
}