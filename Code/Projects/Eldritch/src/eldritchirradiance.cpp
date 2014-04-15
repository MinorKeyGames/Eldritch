#include "core.h"
#include "eldritchirradiance.h"

SVoxelIrradiance::SVoxelIrradiance()
:	m_Light()
{
}

SVoxelIrradiance SVoxelIrradiance::operator+( const SVoxelIrradiance& Other ) const
{
	SVoxelIrradiance RetVal;
	RetVal.m_Light[0] = m_Light[0] + Other.m_Light[0];
	RetVal.m_Light[1] = m_Light[1] + Other.m_Light[1];
	RetVal.m_Light[2] = m_Light[2] + Other.m_Light[2];
	RetVal.m_Light[3] = m_Light[3] + Other.m_Light[3];
	RetVal.m_Light[4] = m_Light[4] + Other.m_Light[4];
	RetVal.m_Light[5] = m_Light[5] + Other.m_Light[5];
	return RetVal;
}

SVoxelIrradiance SVoxelIrradiance::operator*( const float T ) const
{
	SVoxelIrradiance RetVal;
	RetVal.m_Light[0] = m_Light[0] * T;
	RetVal.m_Light[1] = m_Light[1] * T;
	RetVal.m_Light[2] = m_Light[2] * T;
	RetVal.m_Light[3] = m_Light[3] * T;
	RetVal.m_Light[4] = m_Light[4] * T;
	RetVal.m_Light[5] = m_Light[5] * T;
	return RetVal;
}

/*static*/ SVoxelIrradiance SVoxelIrradiance::Lerp( const SVoxelIrradiance& IrrA, const SVoxelIrradiance& IrrB, const float T )
{
	return ( IrrA * ( 1.0f - T ) ) + ( IrrB * T );
}