#include "core.h"
#include "3d.h"
#include "irradiancevolumes.h"
#include "mathcore.h"

IrradianceVolumes::IrradianceVolumes()
:	m_Extents()
,	m_NumVolumesX(0)
,	m_NumVolumesY(0)
,	m_NumVolumesZ(0)
,	m_VolumeResolution()
,	m_Volumes( NULL ) {}

IrradianceVolumes::~IrradianceVolumes()
{
	SafeDeleteArray( m_Volumes );
}

const SIrradianceVolume& IrradianceVolumes::GetNearestVolume( const Vector& Location ) const
{
	Vector Indices = ( ( Location + m_VolumeResolution * 0.5f ) - m_Extents.m_Min ) / m_VolumeResolution;

	int IndexX = Clamp( (int)Indices.x, 0, (int)m_NumVolumesX - 1 );
	int IndexY = Clamp( (int)Indices.y, 0, (int)m_NumVolumesY - 1 );
	int IndexZ = Clamp( (int)Indices.z, 0, (int)m_NumVolumesZ - 1 );

	return GetIndexedVolume( IndexX, IndexY, IndexZ );
}

const SIrradianceVolume& IrradianceVolumes::GetIndexedVolume( uint X, uint Y, uint Z ) const
{
	ASSERT( X < m_NumVolumesX );
	ASSERT( Y < m_NumVolumesY );
	ASSERT( Z < m_NumVolumesZ );

	return *( m_Volumes +
		Z * m_NumVolumesX * m_NumVolumesY +
		Y * m_NumVolumesX +
		X );
}

Vector IrradianceVolumes::GetLocationOfIndexedVolume( uint X, uint Y, uint Z ) const
{
	return m_Extents.m_Min + Vector( (float)X, (float)Y, (float)Z ) * m_VolumeResolution;
}

/*static*/ Vector4 IrradianceVolumes::GetColor( const SIrradianceVolume& Volume, const Vector& Direction )
{
	Vector DirectionSquared = Direction * Direction;
	int XNegative = Direction.x < 0.0f;
	int YNegative = Direction.y < 0.0f;
	int ZNegative = Direction.z < 0.0f;
	return
		DirectionSquared.x * Volume.m_Light[ XNegative ] +
		DirectionSquared.y * Volume.m_Light[ YNegative + 2 ] +
		DirectionSquared.z * Volume.m_Light[ ZNegative + 4 ];
}

/*static*/ Vector4 IrradianceVolumes::GetSum( const SIrradianceVolume& Volume )
{
	Vector4 Sum;
	for( uint SideIndex = 0; SideIndex < 6; ++SideIndex )
	{
		Sum += Volume.m_Light[ SideIndex ];
	}
	return Sum;
}

/*static*/ Vector4 IrradianceVolumes::GetAverage( const SIrradianceVolume& Volume )
{
	return ( 0.1666666f * GetSum( Volume ) );
}

/*static*/ float IrradianceVolumes::GetLuminance( const SIrradianceVolume& Volume )
{
	Vector4 Average = GetAverage( Volume );
	float MinComponent = Min( Min( Average.x, Average.y ), Average.z );
	float MaxComponent = Max( Max( Average.x, Average.y ), Average.z );
	return ( 0.5f * ( MinComponent + MaxComponent ) );
}