#ifndef SECTORBOUND_H
#define SECTORBOUND_H

class Vector;
class Plane;

class SectorBound
{
public:
	SectorBound();
	~SectorBound();

	bool	Contains( const Vector& Location );

	uint	m_NumPlanes;
	Plane*	m_Planes;
};

#endif // SECTORBOUND_H