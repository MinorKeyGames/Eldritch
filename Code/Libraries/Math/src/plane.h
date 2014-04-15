#ifndef PLANE_H
#define PLANE_H

#include "vector.h"
#include "line.h"

class Sphere;
class AABB;
class Ray;
class Segment;
class CollisionInfo;

class Plane
{
public:
	Plane();
	Plane( const Vector& Normal, float Distance );
	Plane( const Vector& Normal, const Vector& Point );
	Plane( const Plane& p );

	Plane& operator=( const Plane& p );

	void	Negate();

	void	Normalize();
	void	FastNormalize();
	Plane	GetNormalized() const;
	Plane	GetFastNormalized() const;

	float	DistanceTo( const Vector& v ) const;	// Get the (signed) distance between plane and point
	Vector	ProjectPoint( const Vector& v ) const;
	Vector	ProjectVector( const Vector& v ) const;
	Vector	Reflect( const Vector& v ) const;
	bool	OnFrontSide( const Vector& v ) const;	// These are liberal tests that include points within
	bool	OnBackSide( const Vector& v ) const;	// an epsilon on the other side.
	bool	Contains( const Vector& v ) const;		// Test whether the point is in the plane--might make more sense as a Vector function

	bool	Intersects( const Sphere& s ) const;
	bool	Intersects( const Plane& p ) const;
	bool	Intersects( const Line& l ) const;
	bool	Intersects( const AABB& a ) const;
	bool	Intersects( const Ray& r, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Segment& s, CollisionInfo* const pInfo = NULL ) const;

	Line	GetIntersection( const Plane& p ) const;
	Vector	GetIntersection( const Line& l ) const;

	bool	IsCoplanarWith( const Plane& p ) const;
	bool	IsFacingSameAs( const Plane& p ) const;

	Vector	m_Normal;	// Or A, B, C
	float	m_Distance;	// D	(negative of what I thought it was--it is the distance from plane to origin along normal)
};

#endif // PLANE_H