#include "core.h"
#include "cylinder.h"
#include "segment.h"
#include "collisioninfo.h"
#include "mathcore.h"

Cylinder::Cylinder()
:	m_Point1( Vector( 0.0f, 0.0f, 0.0f ) ),
	m_Point2( Vector( 0.0f, 0.0f, 0.0f ) ),
	m_Radius( 0.0f ) {}

Cylinder::Cylinder( const Vector& Point1, const Vector& Point2, float Radius )
:	m_Point1( Point1 ),
	m_Point2( Point2 ),
	m_Radius( Radius ) {}

bool Cylinder::Intersects( const Segment& s, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	Vector d = m_Point2 - m_Point1;		// Cylinder axis
	Vector m = s.m_Point1 - m_Point1;	// Vector from cylinder base to segment base?
	Vector n = s.m_Point2 - s.m_Point1;	// Segment vector

	float md = m.Dot( d );
	float nd = n.Dot( d );
	float dd = d.Dot( d );

	if( md < 0.0f && md + nd < 0.0f )
	{
		return false;
	}

	if( md > dd && md + nd > dd )
	{
		return false;
	}

	float nn = n.Dot( n );
	float mn = m.Dot( n );
	float a = dd * nn - nd * nd;
	float k = m.Dot( m ) - m_Radius * m_Radius;
	float c = dd * k - md * md;
	float t = 0.0f;

	if( Abs( a ) < SMALLER_EPSILON )
	{
		// Segment (n) runs parallel to cylinder axis (d)

		if( c > 0.0f )
		{
			return false;
		}

		if( md < 0.0f )
		{
			t = -mn / nn;
		}
		else if( md > dd )
		{
			t = ( nd - mn ) / nn;
		}
		else
		{
			// TODO: This seems to be problematic (or getting here is indicative of an earlier problem)
			WARNDESC( "THAT CYLINDER COLLISION BUG" );
			t = 0.0f;
		}

		if( pInfo )
		{
			Vector Intersection = s.m_Point1 + t * n;
			Vector PointOnLine = Line( m_Point1, m_Point2 - m_Point1 ).NearestPointTo( pInfo->m_Intersection );
			Vector Normal = ( Intersection - PointOnLine ).GetNormalized();

			pInfo->m_Collision = true;
			pInfo->m_Intersection = Intersection;
			pInfo->m_HitT = t;
			pInfo->m_Plane = Plane( Normal, PointOnLine );
		}
		return true;
	}

	float b = dd * mn - nd * md;
	float Discr = b * b - a * c;

	if( Discr < 0.0f )
	{
		return false;
	}

	t = ( -b - SqRt( Discr ) ) / a;

	if( t < 0.0f || t > 1.0f )
	{
		return false;
	}

	// Test endcaps--if we collide with them, count it as not colliding with cylinder
	if( md + t * nd < 0.0f )
	{
		return false;
		//// Segment outside cylinder on first side
		//if( nd <= 0.0f )
		//{
		//	// Segment pointing away from endcap
		//	return false;
		//}
		//float t2 = -md / nd;
		//if( k + t2 * ( 2.0f * mn + t2 * nn ) > 0.0f )
		//{
		//	return false;
		//}
	}
	else if( md + t * nd > dd )
	{
		return false;
		//// Segment outside cylinder on second side
		//if( nd >= 0.0f )
		//{
		//	// Segment pointing away from endcap
		//	return false;
		//}
		//float t2 = ( dd - md ) / nd;
		//if( k + dd - 2.0f * md + t2 * ( 2.0f * ( mn - nd ) + t2 * nn ) > 0.0f )
		//{
		//	return false;
		//}
	}

	if( pInfo )
	{
		Vector Intersection = s.m_Point1 + t * n;
		Vector PointOnLine = Line( m_Point1, m_Point2 - m_Point1 ).NearestPointTo( Intersection );
		Vector Normal = ( Intersection - PointOnLine ).GetNormalized();

		pInfo->m_Collision = true;
		pInfo->m_Intersection = Intersection;
		pInfo->m_HitT = t;
		pInfo->m_Plane = Plane( Normal, PointOnLine );
	}
	return true;
}