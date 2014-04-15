#ifndef IRRADIANCEVOLUMES_H
#define IRRADIANCEVOLUMES_H

#include "vector.h"
#include "vector4.h"
#include "aabb.h"

struct SIrradianceVolume
{
	SIrradianceVolume()
	:	m_Light()
	{
	}

	// Ordered X+, X-, Y+, Y-, Z+, Z-
	Vector4	m_Light[6];
};

// Different from SIrradianceVolumeArray in WorldCompiler/RadiositySolver
// only by using a static array instead of a class Array.
class IrradianceVolumes
{
public:
	IrradianceVolumes();
	~IrradianceVolumes();

	const SIrradianceVolume&	GetNearestVolume( const Vector& Location ) const;
	const SIrradianceVolume&	GetIndexedVolume( uint X, uint Y, uint Z ) const;
	Vector						GetLocationOfIndexedVolume( uint X, uint Y, uint Z ) const;

	static Vector4				GetColor( const SIrradianceVolume& Volume, const Vector& Direction );
	static Vector4				GetSum( const SIrradianceVolume& Volume );
	static Vector4				GetAverage( const SIrradianceVolume& Volume );
	static float				GetLuminance( const SIrradianceVolume& Volume );

	AABB				m_Extents;
	uint				m_NumVolumesX;
	uint				m_NumVolumesY;
	uint				m_NumVolumesZ;
	Vector				m_VolumeResolution;
	SIrradianceVolume*	m_Volumes;
};

#endif // IRRADIANCEVOLUMES_H