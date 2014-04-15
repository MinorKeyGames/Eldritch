#ifndef RAY_H
#define RAY_H

#include "vector.h"

class Triangle;
class AABB;
class CollisionMesh;
class CollisionInfo;
class Sphere;
class Plane;
class Ellipsoid;

class Ray
{
public:
	Ray();
	Ray( const Vector& Point, const Vector& Direction );

	bool	Intersects( const Triangle& t, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const AABB& a, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const CollisionMesh& m, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Sphere& s, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Plane& p, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Ellipsoid& e, CollisionInfo* const pInfo = NULL ) const;

	Vector m_Point;
	Vector m_Direction;
};

#endif // SEGMENT_H