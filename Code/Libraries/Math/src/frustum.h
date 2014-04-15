#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "plane.h"

enum EFrustumSide
{
	EFS_RIGHT,
	EFS_LEFT,
	EFS_BOTTOM,
	EFS_TOP,
	EFS_NEAR,
	EFS_FAR
};

class Matrix;
class Vector;
class AABB;
class Sphere;

class Frustum
{
public:
	Frustum();
	Frustum( const Matrix& m );	// Construct the frustum of the given view-projection matrix
	Frustum( const Frustum& f );

	void InitWith( const Matrix& m );

	bool Intersects( const Vector& Point ) const;	// TODO: More explicit return types for intersections?
	bool Intersects( const AABB& Box ) const;
	bool Intersects( const Sphere& s ) const;
	bool Intersects( const Frustum& f ) const;

	void GetCorners( Vector Corners[8] ) const;

	Plane m_Planes[6];	// Planes face inward
};

#endif // FRUSTUM_H