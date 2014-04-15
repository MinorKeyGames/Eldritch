#include "core.h"
#include "kdtree.h"
#include "collisioninfo.h"
#include "ray.h"
#include "segment.h"
#include "ellipsoid.h"
#include "mesh.h"

KDTree::KDTree()
:	m_FrontChild( NULL )
,	m_BackChild( NULL )
,	m_SplittingPlane()
,	m_FullBound()
,	m_TightBound()
,	m_LocalBound()
,	m_NumMeshes( 0 )
,	m_Meshes( NULL )
{
}

KDTree::~KDTree()
{
	// kd-tree doesn't actually manage the meshes (world does),
	// so no need to delete the elements of the array
	SafeDeleteArray( m_Meshes );
}

bool KDTree::Trace( const Segment& s, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	CollisionInfo Info;
	CollisionInfo MinInfo;
	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}

	// Test local meshes
	for( uint i = 0; i < m_NumMeshes; ++i )
	{
		Mesh* pMesh = m_Meshes[i];

		// Cull non-solid meshes
		if( pMesh->m_CollisionMesh.m_MutableCollidable && pMesh->m_CollisionMesh.m_NumTris )
		{
			// Cull non-matching flags
			if( Info.m_CollisionMeshFlags & pMesh->m_CollisionMesh.m_Flags )
			{
				if( s.Intersects( pMesh->m_AABB ) )
				{
					if( s.Intersects( pMesh->m_CollisionMesh, &Info ) )
					{
						Info.m_HitMesh = pMesh;
						if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
						{
							MinInfo = Info;
						}
					}
				}
			}
		}
	}

	// Test meshes of children
	if( m_FrontChild )
	{
		ASSERT( m_BackChild );

		KDTree* pFirstChild = s.m_Point1.IsOnFrontSide( m_SplittingPlane ) ? m_FrontChild : m_BackChild;
		KDTree* pSecondChild = pFirstChild == m_FrontChild ? m_BackChild : m_FrontChild;

		if( pFirstChild && pFirstChild->Trace( s, &Info ) )
		{
			if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
			{
				MinInfo = Info;
			}
		}
		// Only test the other side if we did not intersect anything in
		// the first child and we actually intersect the splitting plane
		else
		{
			if( s.Intersects( m_SplittingPlane ) )
			{
				if( pSecondChild && pSecondChild->Trace( s, &Info ) )
				{
					if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
					{
						MinInfo = Info;
					}
				}
			}
		}
	}

	if( MinInfo.m_Collision )	// I.e., did we get a collision yet?
	{
		if( pInfo )
		{
			pInfo->CopyOutParametersFrom( MinInfo );
		}
		return true;
	}
	return false;
}

bool KDTree::Trace( const Ray& r, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	CollisionInfo Info;
	CollisionInfo MinInfo;
	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}

	// Test local meshes
	for( uint i = 0; i < m_NumMeshes; ++i )
	{
		Mesh* pMesh = m_Meshes[i];

		// Cull non-solid meshes
		if( pMesh->m_CollisionMesh.m_MutableCollidable && pMesh->m_CollisionMesh.m_NumTris )
		{
			// Cull non-matching flags
			if( Info.m_CollisionMeshFlags & pMesh->m_CollisionMesh.m_Flags )
			{
				if( r.Intersects( pMesh->m_AABB ) )
				{
					if( r.Intersects( pMesh->m_CollisionMesh, &Info ) )
					{
						Info.m_HitMesh = pMesh;
						if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
						{
							MinInfo = Info;
						}
					}
				}
			}
		}
	}

	// Test meshes of children
	if( m_FrontChild )
	{
		ASSERT( m_BackChild );

		KDTree* pFirstChild = r.m_Point.IsOnFrontSide( m_SplittingPlane ) ? m_FrontChild : m_BackChild;
		KDTree* pSecondChild = pFirstChild == m_FrontChild ? m_BackChild : m_FrontChild;

		if( pFirstChild->Trace( r, &Info ) )
		{
			if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
			{
				MinInfo = Info;
			}
		}
		// Only test the other side if we did not intersect anything in
		// the first child and we actually intersect the splitting plane
		else
		{
			if( r.Intersects( m_SplittingPlane ) )
			{
				if( pSecondChild->Trace( r, &Info ) )
				{
					if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
					{
						MinInfo = Info;
					}
				}
			}
		}
	}

	if( MinInfo.m_Collision )	// I.e., did we get a collision yet?
	{
		if( pInfo )
		{
			pInfo->CopyOutParametersFrom( MinInfo );
		}
		return true;
	}
	return false;
}

bool KDTree::SweepAgainst( const Ellipsoid& e, const Vector& v, const AABB& SweepAABB, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	CollisionInfo Info;
	CollisionInfo MinInfo;
	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}

	// Test local meshes
	for( uint i = 0; i < m_NumMeshes; ++i )
	{
		Mesh* pMesh = m_Meshes[i];

		// Cull non-solid meshes
		if( pMesh->m_CollisionMesh.m_MutableCollidable && pMesh->m_CollisionMesh.m_NumTris )
		{
			// Cull non-matching flags
			if( Info.m_CollisionMeshFlags & pMesh->m_CollisionMesh.m_Flags )
			{
				if( e.SweepAgainst( pMesh->m_AABB, v ) )
				{
					if( e.SweepAgainst( pMesh->m_CollisionMesh, v, SweepAABB, &Info ) )
					{
						Info.m_HitMesh = pMesh;
						if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
						{
							MinInfo = Info;
						}
					}
				}
			}
		}
	}

	// Test meshes of children
	if( m_FrontChild )
	{
		ASSERT( m_BackChild );

		KDTree* pFirstChild = e.m_Center.IsOnFrontSide( m_SplittingPlane ) ? m_FrontChild : m_BackChild;
		KDTree* pSecondChild = pFirstChild == m_FrontChild ? m_BackChild : m_FrontChild;

		if( pFirstChild && e.SweepAgainst( pFirstChild->m_TightBound, v ) )
		{
			if( pFirstChild->SweepAgainst( e, v, SweepAABB, &Info ) )
			{
				if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
				{
					MinInfo = Info;
				}
			}
		}

		if( pSecondChild && e.SweepAgainst( pSecondChild->m_TightBound, v ) )
		{
			if( pSecondChild->SweepAgainst( e, v, SweepAABB, &Info ) )
			{
				if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
				{
					MinInfo = Info;
				}
			}
		}
	}

	if( MinInfo.m_Collision )	// I.e., did we get a collision yet?
	{
		if( pInfo )
		{
			pInfo->CopyOutParametersFrom( MinInfo );
		}
		return true;
	}
	return false;
}