#ifndef AABB_H
#define AABB_H

#include "vector.h"

class Frustum;
class Segment;
class Plane;
class Ray;
class Sphere;
class Matrix;

class AABB
{
public:
	AABB();
	AABB( const Vector& Point );
	AABB( const Vector& MinCorner, const Vector& MaxCorner );

	Vector	GetClosestPoint( const Vector& v ) const;
	Vector	GetExtents() const { return 0.5f * ( m_Max - m_Min ); }
	Vector	GetCenter() const { return m_Min + GetExtents(); }
	float	GetSquaredDistance( const Vector& v ) const;

	bool	Equals( const AABB& a ) const;

	float	GetVolume() const;
	bool	IsZero() const;

	bool	Intersects( const AABB& a ) const;
	bool	Intersects( const Segment& s ) const;
	bool	Intersects( const Vector& v ) const;	// Point
	bool	Intersects( const Frustum& f ) const;
	bool	IsOnFrontSide( const Plane& p ) const;
	bool	IsOnBackSide( const Plane& p ) const;
	bool	Intersects( const Plane& p ) const;
	bool	Intersects( const Ray& r ) const;
	bool	Intersects( const Sphere& s ) const;

	void	ExpandTo( const Vector& v );
	void	ExpandBy( const Vector& v );
	void	ExpandTo( const AABB& a );

	void	MoveBy( const Vector& v );

	AABB	GetTransformedBound( const Matrix& m ) const;

	Vector m_Min;
	Vector m_Max;

	static inline AABB CreateFromCenterAndExtents( const Vector& Center, const Vector& HalfExtents )
	{
		return AABB( Center - HalfExtents, Center + HalfExtents );
	}
};

#endif // AABB_H