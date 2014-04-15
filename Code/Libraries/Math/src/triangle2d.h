#ifndef TRIANGLE2D_H
#define TRIANGLE2D_H

#include "vector2.h"
#include "box2d.h"

class Triangle2D
{
public:
	Triangle2D();
	Triangle2D( const Vector2& Vec1, const Vector2& Vec2, const Vector2& Vec3 );

	Box2D	GetBound() const;

	Vector2	m_Vec1;
	Vector2	m_Vec2;
	Vector2	m_Vec3;
};

#endif // TRIANGLE2D_H