#ifndef COLLISIONINFO2D_H
#define COLLISIONINFO2D_H

#include "vector2.h"

class CollisionInfo2D
{
public:
	CollisionInfo2D();

	// Partial assignment
	void	CopyInParametersFrom( const CollisionInfo2D& OtherInfo );
	void	CopyOutParametersFrom( const CollisionInfo2D& OtherInfo );

	enum EHitNormal
	{
		EHN_None,
		EHN_Left,
		EHN_Right,
		EHN_Up,
		EHN_Down,
	};

	bool		m_Collision;
	EHitNormal	m_EHitNormal;
	Vector2		m_HitNormal;
	Vector2		m_Intersection;
	float		m_HitT;
};

#endif COLLISIONINFO2D_H