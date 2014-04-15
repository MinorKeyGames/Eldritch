#ifndef NOISE_H
#define NOISE_H

typedef float ( *Noise1Func )( float );
typedef float ( *Noise2Func )( float, float );
typedef float ( *Noise3Func )( float, float, float );

namespace Noise
{
	void	ShuffleHasher();
	void	ShuffleCubicValues();

	// Cubic noise functions return range -1..1 (easily changed to 0..1 in InitCubicNoise)
	float	CubicNoise1( float X );
	float	CubicNoise2( float X, float Y );
	float	CubicNoise3( float X, float Y, float Z );

	// Range 0..65535
	int		IntNoise1( int X );
	int		IntNoise2( int X, int Y );
	int		IntNoise3( int X, int Y, int Z );

	// Range 0..2 (see notes in CellNoise1)
	float	CellNoise1( float X );
	float	CellNoise2( float X, float Y );
	float	CellNoise3( float X, float Y, float Z );

	// Range 0..1
	float	GridNoise1( float X );
	float	GridNoise2( float X, float Y );
	float	GridNoise3( float X, float Y, float Z );

	// For noise range -1..1, these approach range -2..2 as Octaves approaches infinity
	// For noise range 0..2, these approach range 0..4
	float	SumNoise1( float x, uint Octaves, Noise1Func NoiseFunc );
	float	SumNoise2( float x, float y, uint Octaves, Noise2Func NoiseFunc );
	float	SumNoise3( float x, float y, float z, uint Octaves, Noise3Func NoiseFunc );

	// Helper function, returns the scale applied when summing noise with the given number of octaves.
	// (E.g., 1 => 1.0f, 2 => 1.5f, 3 => 1.75f, etc.)
	float	GetOctaveScale( uint Octaves );

	// For noise range -1..1, these approach range 0..2 as Octaves approaches infinity
	// For noise range 0..1, these approach range 0..2
	float	SumAbsNoise1( float x, uint Octaves, Noise1Func NoiseFunc );
	float	SumAbsNoise2( float x, float y, uint Octaves, Noise2Func NoiseFunc );
	float	SumAbsNoise3( float x, float y, float z, uint Octaves, Noise3Func NoiseFunc );

	// Perlin noise
	float Noise1( float x );
	float Noise2( float x, float y );
	float Noise3( float x, float y, float z );
}

#endif // NOISE_H