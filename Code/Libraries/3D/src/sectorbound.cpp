#include "core.h"
#include "sectorbound.h"
#include "vector.h"
#include "plane.h"

SectorBound::SectorBound()
:	m_NumPlanes( 0 )
,	m_Planes( NULL ) {}

SectorBound::~SectorBound()
{
	SafeDeleteArray( m_Planes );
}

bool SectorBound::Contains( const Vector& Location )
{
	for( uint i = 0; i < m_NumPlanes; ++i )
	{
		if( !m_Planes[ i ].OnBackSide( Location ) )
		{
			return false;
		}
	}
	return true;
}