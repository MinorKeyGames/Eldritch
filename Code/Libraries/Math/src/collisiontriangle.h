#ifndef COLLISIONTRIANGLE_H
#define COLLISIONTRIANGLE_H

#include "triangle.h"
#include "aabb.h"

class CollisionTriangle
{
public:
	CollisionTriangle();
	CollisionTriangle( const Triangle& t );

	Triangle	m_Triangle;
	AABB		m_AABB;

	void	UpdateAABB();
};

#endif // COLLISIONTRIANGLE_H