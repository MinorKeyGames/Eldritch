#ifndef BOX2D_H
#define BOX2D_H

#include "vector2.h"
#include "array.h"

class Triangle2D;
class CollisionInfo2D;
class Segment2D;

class Box2D
{
public:
	Box2D();
	Box2D( const Vector2& InMin, const Vector2& InMax );

	float	GetArea() const;

	bool	Intersects( const Box2D& Box ) const;
	bool	Contains( const Vector2& Point ) const;

	bool	SweepAgainst( const Segment2D& Offset, const Array<Triangle2D>& Tris, CollisionInfo2D* const pInfo = NULL ) const;
	bool	SweepAgainst( const Segment2D& Offset, const Triangle2D& Tri, CollisionInfo2D* const pInfo = NULL ) const;

	void	ExpandTo( const Vector2& v );
	void	ExpandBy( const Vector2& v );

	Vector2	GetCenter() const;
	Vector2	GetExtents() const;
	void	GetCenterAndExtents( Vector2& OutCenter, Vector2& OutExtents ) const;

	Box2D operator+( const Vector2& Offset ) const;
	Box2D& operator+=( const Vector2& Offset );

	Box2D operator-( const Vector2& Offset ) const;
	Box2D& operator-=( const Vector2& Offset );

	static Box2D GetBound( const Box2D& BoxA, const Box2D& BoxB );

	Vector2	m_Min;
	Vector2 m_Max;
};

#endif BOX2D_H