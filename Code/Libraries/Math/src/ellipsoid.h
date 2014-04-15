#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include "vector.h"
#include "aabb.h"

class Triangle;
class CollisionTriangle;
class CollisionInfo;
class CollisionMesh;
class Ray;
class Segment;

class Ellipsoid
{
public:
	Ellipsoid();
	Ellipsoid( const Vector& Center, const Vector& Extents );

	AABB	GetAABB() const;

	bool	Intersects( const Ray& r, CollisionInfo* const pInfo ) const;
	bool	Intersects( const Segment& s, CollisionInfo* const pInfo ) const;
	bool	Intersects( const Ellipsoid& e ) const;
	bool	Intersects( const Triangle& t, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const AABB& a ) const;
	bool	Intersects( const CollisionTriangle& t ) const;
	bool	Intersects( const CollisionMesh& m ) const;
	bool	SweepAgainst( const Ellipsoid& e, const Vector& v, CollisionInfo* const pInfo ) const;
	bool	SweepAgainst( const AABB& a, const Vector& v ) const;
	bool	SweepAgainst( const CollisionTriangle& t, const Vector& v, const AABB& SweepAABB, CollisionInfo* const pInfo ) const;
	bool	SweepAgainst( const CollisionMesh& m, const Vector& v, const AABB& SweepAABB, CollisionInfo* const pInfo ) const;

	Vector	m_Center;
	Vector	m_Extents;
};

#endif // ELLIPSOID_H