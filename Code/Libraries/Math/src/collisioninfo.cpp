#include "core.h"
#include "collisioninfo.h"
#include "collisionmesh.h"

CollisionInfo::CollisionInfo()
:	m_CollideWorld( false )
,	m_CollideEntities( false )
,	m_CollideNonBlockingEntities( false )
,	m_UseCompositeCollision( false )
,	m_StopAtAnyCollision( false )
,	m_CollidingEntity( NULL )
,	m_TargetEntity( NULL )
,	m_CollisionMeshFlags( CMF_Default )
,	m_UserFlags( 0 )
,	m_Collision( false )
,	m_Plane()
,	m_Intersection()
,	m_HitT( 1.0f )	// 30 Mar 2013: Changed from 0 to 1 so a trace that doesn't collide has a max T. Shouldn't cause problems, but just in case.
,	m_HitEntity( NULL )
,	m_HitMesh( NULL )
,	m_HitName( HashedString::NullString )
{
}

void CollisionInfo::ResetInParameters()
{
	m_CollideWorld					= false;
	m_CollideEntities				= false;
	m_CollideNonBlockingEntities	= false;
	m_UseCompositeCollision			= false;
	m_StopAtAnyCollision			= false;
	m_CollidingEntity				= NULL;
	m_TargetEntity					= NULL;
	m_CollisionMeshFlags			= CMF_Default;
	m_UserFlags						= 0;
}

void CollisionInfo::ResetOutParameters()
{
	m_Collision		= false;
	m_Plane			= Plane();
	m_Intersection	= Vector();
	m_HitT			= 1.0f;
	m_HitEntity		= NULL;
	m_HitMesh		= NULL;
	m_HitName		= HashedString::NullString;
}

void CollisionInfo::CopyInParametersFrom( const CollisionInfo& OtherInfo )
{
	m_CollideWorld					= OtherInfo.m_CollideWorld;
	m_CollideEntities				= OtherInfo.m_CollideEntities;
	m_CollideNonBlockingEntities	= OtherInfo.m_CollideNonBlockingEntities;
	m_UseCompositeCollision			= OtherInfo.m_UseCompositeCollision;
	m_StopAtAnyCollision			= OtherInfo.m_StopAtAnyCollision;
	m_CollidingEntity				= OtherInfo.m_CollidingEntity;
	m_TargetEntity					= OtherInfo.m_TargetEntity;
	m_CollisionMeshFlags			= OtherInfo.m_CollisionMeshFlags;
	m_UserFlags						= OtherInfo.m_UserFlags;
}

void CollisionInfo::CopyOutParametersFrom( const CollisionInfo& OtherInfo )
{
	m_Collision		= OtherInfo.m_Collision;
	m_Plane			= OtherInfo.m_Plane;
	m_Intersection	= OtherInfo.m_Intersection;
	m_HitT			= OtherInfo.m_HitT;
	m_HitEntity		= OtherInfo.m_HitEntity;
	m_HitMesh		= OtherInfo.m_HitMesh;
	m_HitName		= OtherInfo.m_HitName;
}