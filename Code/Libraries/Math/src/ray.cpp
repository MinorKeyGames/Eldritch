#include "core.h"
#include "ray.h"
#include "triangle.h"
#include "aabb.h"
#include "collisionmesh.h"
#include "collisioninfo.h"
#include "mathcore.h"
#include "sphere.h"
#include "ellipsoid.h"

Ray::Ray()
:	m_Point( Vector( 0.0f, 0.0f, 0.0f ) ),
	m_Direction( Vector( 0.0f, 0.0f, 0.0f ) ) {}

Ray::Ray( const Vector& Point, const Vector& Direction )
:	m_Point( Point ),
	m_Direction( Direction ) {}

bool Ray::Intersects( const Triangle& t, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	// Could be optimized
	// See Real-Time Collision Detection p. 191
	Vector v12 = t.m_Vec2 - t.m_Vec1;
	Vector v13 = t.m_Vec3 - t.m_Vec1;
	Vector Direction = -m_Direction;	// Hurrr

	Vector TriNormal = v12.Cross( v13 );

	float d = Direction.Dot( TriNormal );

	// If denominator is zero, ray is parallel to triangle
	if( d == 0.0f )
	{
		return false;
	}

	// If denominator is less than zero, ray points away from triangle
	if( d < 0.0f )
	{
		return false;
	}

	Vector v1p = m_Point - t.m_Vec1;
	float dt = v1p.Dot( TriNormal );

	if( dt < 0.0f )
	{
		return false;
	}

	Vector e = Direction.Cross( v1p );
	float v = v13.Dot( e );

	if( v < 0.0f || v > d )
	{
		return false;
	}

	float w = -( v12.Dot( e ) );

	if( w < 0.0f || v + w > d )
	{
		return false;
	}

	if( pInfo )
	{
		pInfo->m_Collision		= true;
		pInfo->m_HitT			= dt / d;
		pInfo->m_Intersection	= m_Point + pInfo->m_HitT * m_Direction;
		pInfo->m_Plane			= t.GetPlane();
	}

	return true;
}

// "Slab" test
// See Real-Time Collision Detection p.180
bool Ray::Intersects( const AABB& a, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	float tMin = 0.0f;
	float tMax = FLT_MAX;

	int IntersectingAxis = -1;

	// Maybe an optimization?
	static const Vector kUnits = Vector( 1.0f, 1.0f, 1.0f );
	const Vector OneOverD = kUnits / m_Direction;

	for( uint i = 0; i < 3; ++i )
	{
		if( Abs( m_Direction.v[i] ) < SMALLER_EPSILON )
		{
			// Ray is (close to) parallel to slab in ith dimension. No intersection if origin is outside slab.
			if( m_Point.v[i] < a.m_Min.v[i] || m_Point.v[i] > a.m_Max.v[i] )
			{
				// But warn if there might be an intersection that we're ignoring (maybe epsilon is too large)
				DEVASSERT(
					( m_Point.v[i] < a.m_Min.v[i] && m_Point.v[i] + m_Direction.v[i] < a.m_Min.v[i] ) ||
					( m_Point.v[i] > a.m_Max.v[i] && m_Point.v[i] + m_Direction.v[i] > a.m_Max.v[i] ) );
				return false;
			}
		}
		else
		{
			float t1 = ( a.m_Min.v[i] - m_Point.v[i] ) * OneOverD.v[i];
			float t2 = ( a.m_Max.v[i] - m_Point.v[i] ) * OneOverD.v[i];
			if( t1 > t2 )
			{
				Swap( t1, t2 );
			}

			tMin = Max( tMin, t1 );
			tMax = Min( tMax, t2 );

			if( tMin == t1 )
			{
				IntersectingAxis = i;
			}

			if( tMin > tMax )
			{
				return false;
			}
		}
	}

	if( pInfo )
	{
		Vector Normal;
		if( IntersectingAxis >= 0 )
		{
			Normal.v[ IntersectingAxis ] = -Sign( m_Direction.v[ IntersectingAxis ] );
		}

		pInfo->m_Collision		= true;
		pInfo->m_HitT			= tMin;
		pInfo->m_Intersection	= m_Point + pInfo->m_HitT * m_Direction;
		pInfo->m_Plane			= Plane( Normal, pInfo->m_Intersection );
	}

	return true;
}

bool Ray::Intersects( const CollisionMesh& m, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return m.Intersects( *this, pInfo );
}

bool Ray::Intersects( const Sphere& s, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return s.Intersects( *this, pInfo );
}

// Works for either side of a plane; this wasn't always the case
bool Ray::Intersects( const Plane& p, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	float d = m_Direction.Dot( p.m_Normal );
	if( Abs( d ) > EPSILON )	// Is ray not parallel to the plane?
	{
		// Test the t-value where the line crosses the plane to see if the
		// ray intersects or lies completely on one side (accounting for
		// the possibility that the ray faces the other way now).
		float t = -p.DistanceTo( m_Point ) / d;

		if( t >= 0.0f )
		{
			if( pInfo )
			{
				pInfo->m_Collision		= true;
				pInfo->m_Intersection	= m_Point + t * m_Direction;
				pInfo->m_Plane			= p;
				pInfo->m_HitT			= t;
			}
			return true;
		}
	}
	return false;
}

bool Ray::Intersects( const Ellipsoid& e, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return e.Intersects( *this, pInfo );
}