#ifndef SPHERE_H
#define SPHERE_H

#include "vector.h"

class Plane;
class Frustum;
class Line;
class Triangle;
class CollisionInfo;
class AABB;
class CollisionMesh;
class Ray;
class Segment;

class Sphere
{
public:
	Sphere();
	Sphere( const Vector& Center, float Radius );

	bool	Intersects( const Vector& v ) const;
	bool	Intersects( const Plane& p ) const;
	bool	Intersects( const Frustum& f ) const;
	bool	Intersects( const Sphere& s ) const;
	bool	Intersects( const Line& l ) const;
	bool	Intersects( const Ray& r, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Segment& s, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Triangle& t, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const AABB& a ) const;
	bool	Intersects( const CollisionMesh& m, CollisionInfo* const pInfo = NULL ) const;
	bool	SweepAgainst( const Plane& p, const Vector& v, CollisionInfo* const pInfo = NULL ) const;
	bool	SweepAgainst( const AABB& a, const Vector& v ) const;
	bool	SweepAgainst( const Triangle& t, const Vector& v, CollisionInfo* const pInfo = NULL ) const;

	Vector	m_Center;
	float	m_Radius;
};

#endif // SPHERE_H