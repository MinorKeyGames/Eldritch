#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"
#include "plane.h"
#include "aabb.h"

class Line;
class Segment;
class Ray;
class CollisionInfo;
class Sphere;
class Ellipsoid;

class Triangle
{
public:
	Triangle();
	Triangle( const Vector& Vec1, const Vector& Vec2, const Vector& Vec3 );

	Triangle	GetFlipped() const;

	Vector	GetNormal() const;
	Plane	GetPlane() const;
	bool	IsInFrontOf( const Plane& P ) const;
	bool	IsInBackOf( const Plane& P ) const;
	Vector	GetClosestPoint( const Vector& v ) const;
	Vector	GetClosestPoint( const Vector& v, Vector* OutBarycentricCoords ) const;

	bool	Contains( const Vector& p ) const;
	bool	Intersects( const Line& l, bool TwoSided ) const;
	bool	Intersects( const Segment& s, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Ray& r, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Sphere& s, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Ellipsoid& e, CollisionInfo* const pInfo = NULL ) const;

	Vector	GetCentroid() const;
	float	GetArea() const;
	AABB	GetAABB() const;

	Vector	m_Vec1;
	Vector	m_Vec2;
	Vector	m_Vec3;
};

#endif // TRIANGLE_H