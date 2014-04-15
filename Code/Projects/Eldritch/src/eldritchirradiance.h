#ifndef ELDRITCHIRRADIANCE_H
#define ELDRITCHIRRADIANCE_H

#include "vector4.h"

// Indicates direction of normal that gets lit; *opposite* of the direction light is traveling through volume
enum EIrradianceDir
{
	IRRDIR_Right,
	IRRDIR_Left,
	IRRDIR_Front,
	IRRDIR_Back,
	IRRDIR_Up,
	IRRDIR_Down
};

struct SVoxelIrradiance
{
	SVoxelIrradiance();

	// Ordered X+, X-, Y+, Y-, Z+, Z-
	Vector4	m_Light[6];

	SVoxelIrradiance operator+( const SVoxelIrradiance& Other ) const;
	SVoxelIrradiance operator*( const float T ) const;

	static SVoxelIrradiance Lerp( const SVoxelIrradiance& IrrA, const SVoxelIrradiance& IrrB, const float T );
};

#endif // ELDRITCHIRRADIANCE_H