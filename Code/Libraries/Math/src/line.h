#ifndef LINE_H
#define LINE_H

#include "vector.h"

class Plane;
class Sphere;
class Triangle;

class Line
{
public:
	Line();
	Line( const Vector& Point, const Vector& Direction );

	Vector NearestPointTo( const Vector& v ) const;	// Get the nearest point on the line to the given point
	float DistanceTo( const Vector& v ) const;	// Get the shortest distance from line to the point

	bool Intersects( const Vector& v ) const;
	bool Intersects( const Plane& p ) const;
	bool Intersects( const Sphere& s ) const;
	bool Intersects( const Triangle& t, bool TwoSided ) const;

	Vector GetIntersection( const Plane& p ) const;

	Vector m_Point;
	Vector m_Direction;
};

#endif // LINE_H