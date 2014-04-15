#ifndef SEGMENT2D_H
#define SEGMENT2D_H

#include "vector2.h"
#include "array.h"

class CollisionInfo2D;
class Box2D;
class Triangle2D;

class Segment2D
{
public:
	Segment2D();
	Segment2D( const Vector2& InStart, const Vector2& InEnd );

	bool Intersects( const Box2D& Box, CollisionInfo2D* const pInfo = NULL ) const;
	bool Intersects( const Array<Triangle2D>& Tris, CollisionInfo2D* const pInfo = NULL ) const;
	bool Intersects( const Triangle2D& Tri, CollisionInfo2D* const pInfo = NULL ) const;
	bool Intersects( const Segment2D& Other, CollisionInfo2D* const pInfo = NULL ) const;

	Segment2D	operator+( const Vector2& Offset ) const;
	Segment2D&	operator+=( const Vector2& Offset );

	Segment2D	operator-( const Vector2& Offset ) const;
	Segment2D&	operator-=( const Vector2& Offset );

	Vector2	m_Start;
	Vector2 m_End;
};

#endif SEGMENT2D_H