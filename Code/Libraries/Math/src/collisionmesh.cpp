#include "core.h"
#include "collisionmesh.h"
#include "collisiontriangle.h"
#include "segment.h"
#include "ray.h"
#include "collisioninfo.h"
#include "sphere.h"

CollisionMesh::CollisionMesh()
:	m_NumTris( 0 )
,	m_Tris( NULL )
,	m_MutableCollidable( true )
,	m_Flags( CMF_All )
{
}

CollisionMesh::CollisionMesh( unsigned int NumTris, CollisionTriangle* Tris, uint Flags )
:	m_NumTris( NumTris )
,	m_Tris( Tris )
,	m_MutableCollidable( true )
,	m_Flags( Flags )
{
}

CollisionMesh::~CollisionMesh()
{
	SafeDeleteArray( m_Tris );
}

// TODO: These are all ending up being the exact same code, copied because
// primitives don't share an interface. It'd be nice to refactor that somehow.

bool CollisionMesh::Intersects( const Segment& s, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	CollisionInfo Info;
	CollisionInfo MinInfo;
	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}
	for( uint i = 0; i < m_NumTris; ++i )
	{
		if( s.Intersects( m_Tris[i].m_Triangle, &Info ) )
		{
			if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
			{
				MinInfo = Info;
			}
		}
	}
	if( MinInfo.m_Collision )
	{
		if( pInfo )
		{
			pInfo->CopyOutParametersFrom( MinInfo );
		}
		return true;
	}
	return false;
}

bool CollisionMesh::Intersects( const Ray& r, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	CollisionInfo Info;
	CollisionInfo MinInfo;
	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}
	for( uint i = 0; i < m_NumTris; ++i )
	{
		if( r.Intersects( m_Tris[i].m_Triangle, &Info ) )
		{
			if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
			{
				MinInfo = Info;
			}
		}
	}
	if( MinInfo.m_Collision )
	{
		if( pInfo )
		{
			pInfo->CopyOutParametersFrom( MinInfo );
		}
		return true;
	}
	return false;
}

bool CollisionMesh::Intersects( const Sphere& s, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	CollisionInfo Info;
	CollisionInfo MinInfo;
	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}
	for( uint i = 0; i < m_NumTris; ++i )
	{
		if( s.Intersects( m_Tris[i].m_Triangle, &Info ) )
		{
			if( ( Info.m_Intersection - s.m_Center ).LengthSquared() < ( MinInfo.m_Intersection - s.m_Center ).LengthSquared() || !MinInfo.m_Collision )
			{
				MinInfo = Info;
			}
		}
	}
	if( MinInfo.m_Collision )
	{
		if( pInfo )
		{
			pInfo->CopyOutParametersFrom( MinInfo );
		}
		return true;
	}
	return false;
}