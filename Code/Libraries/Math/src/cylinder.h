#ifndef CYLINDER_H
#define CYLINDER_H

#include "vector.h"

class Segment;
class CollisionInfo;

class Cylinder
{
public:
	Cylinder();
	Cylinder( const Vector& Point1, const Vector& Point2, float Radius );

	bool	Intersects( const Segment& s, CollisionInfo* const pInfo = NULL ) const;

	Vector	m_Point1;
	Vector	m_Point2;
	float	m_Radius;
};

#endif // CYLINDER_H