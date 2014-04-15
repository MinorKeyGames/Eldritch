#include "core.h"
#include "collisioninfo2d.h"

CollisionInfo2D::CollisionInfo2D()
:	m_Collision( false )
,	m_EHitNormal( CollisionInfo2D::EHN_None )
,	m_HitNormal()
,	m_Intersection()
,	m_HitT( 0.0f )
{
}

void CollisionInfo2D::CopyInParametersFrom( const CollisionInfo2D& OtherInfo )
{
	Unused( OtherInfo );
}

void CollisionInfo2D::CopyOutParametersFrom( const CollisionInfo2D& OtherInfo )
{
	m_Collision = OtherInfo.m_Collision;
	m_EHitNormal = OtherInfo.m_EHitNormal;
	m_HitNormal = OtherInfo.m_HitNormal;
	m_Intersection = OtherInfo.m_Intersection;
	m_HitT = OtherInfo.m_HitT;
}