#include "core.h"
#include "aabb.h"
#include "frustum.h"
#include "segment.h"
#include "plane.h"
#include "ray.h"
#include "mathcore.h"
#include "sphere.h"

AABB::AABB()
:	m_Min( 0.0f, 0.0f, 0.0f )
,	m_Max( 0.0f, 0.0f, 0.0f )
{
}

AABB::AABB( const Vector& Point )
:	m_Min( Point )
,	m_Max( Point )
{
}

AABB::AABB( const Vector& MinCorner, const Vector& MaxCorner )
:	m_Min( MinCorner )
,	m_Max( MaxCorner )
{
}

Vector AABB::GetClosestPoint( const Vector& v ) const
{
	Vector ClosestPoint;

	for( uint i = 0; i < 3; ++i )
	{
		ClosestPoint.v[i] = Min( Max( v.v[i], m_Min.v[i] ), m_Max.v[i] );
	}

	return ClosestPoint;
}

float AABB::GetSquaredDistance( const Vector& v ) const
{
	const Vector	ClosestPoint	= GetClosestPoint( v );
	const Vector	Offset			= ClosestPoint - v;
	const float		DistSq			= Offset.LengthSquared();

	return DistSq;
}

bool AABB::Equals( const AABB& a ) const
{
	return m_Min.Equals( a.m_Min, EPSILON ) && m_Max.Equals( a.m_Max, EPSILON );
}

float AABB::GetVolume() const
{
	return ( m_Max.x - m_Min.x ) * ( m_Max.y - m_Min.y ) * ( m_Max.z - m_Min.z );
}

bool AABB::IsZero() const
{
	return m_Min.IsZero() && m_Max.IsZero();
}

bool AABB::Intersects( const AABB& a ) const
{
	// Cases: this contains a, a contains this, other intersection, no intersection

	//return !( m_Min.x > a.m_Max.x ||
	//		  m_Min.y > a.m_Max.y ||
	//		  m_Min.z > a.m_Max.z ||
	//		  m_Max.x < a.m_Min.x ||
	//		  m_Max.y < a.m_Min.y ||
	//		  m_Max.z < a.m_Min.z );

	return ( a.m_Max.x >= m_Min.x &&
			 a.m_Max.y >= m_Min.y &&
			 a.m_Max.z >= m_Min.z &&
			 a.m_Min.x <= m_Max.x &&
			 a.m_Min.y <= m_Max.y &&
			 a.m_Min.z <= m_Max.z );
}

bool AABB::Intersects( const Vector& v ) const
{
	return ( v.x >= m_Min.x &&
			 v.y >= m_Min.y &&
			 v.z >= m_Min.z &&
			 v.x <= m_Max.x &&
			 v.y <= m_Max.y &&
			 v.z <= m_Max.z );
}

bool AABB::Intersects( const Frustum& f ) const
{
	return f.Intersects( *this );
}

bool AABB::Intersects( const Segment& s ) const
{
	return s.Intersects( *this );
}

bool AABB::IsOnFrontSide( const Plane& p ) const
{
	return ( m_Min.IsOnFrontSide( p ) && m_Max.IsOnFrontSide( p ) );
}

bool AABB::IsOnBackSide( const Plane& p ) const
{
	return ( m_Min.IsOnBackSide( p ) && m_Max.IsOnBackSide( p ) );
}

bool AABB::Intersects( const Plane& p ) const
{
	return p.Intersects( *this );
}

bool AABB::Intersects( const Ray& r ) const
{
	return r.Intersects( *this );
}

bool AABB::Intersects( const Sphere& s ) const
{
	return s.Intersects( *this );
}

void AABB::ExpandTo( const Vector& v )
{
	for( uint i = 0; i < 3; ++i )
	{
		if( v.v[i] < m_Min.v[i] )
		{
			m_Min.v[i] = v.v[i];
		}
		else if( v.v[i] > m_Max.v[i] )
		{
			m_Max.v[i] = v.v[i];
		}
	}
}

void AABB::ExpandBy( const Vector& v )
{
	for( uint i = 0; i < 3; ++i )
	{
		m_Min.v[i] -= v.v[i];
		m_Max.v[i] += v.v[i];
	}
}

void AABB::ExpandTo( const AABB& a )
{
	ExpandTo( a.m_Min );
	ExpandTo( a.m_Max );
}

void AABB::MoveBy( const Vector& v )
{
	m_Min += v;
	m_Max += v;
}

AABB AABB::GetTransformedBound( const Matrix& m ) const
{
	AABB RetVal;

	const Vector V0 = Vector( m_Min.x, m_Min.y, m_Min.z ) * m;
	const Vector V1 = Vector( m_Max.x, m_Min.y, m_Min.z ) * m;
	const Vector V2 = Vector( m_Min.x, m_Max.y, m_Min.z ) * m;
	const Vector V3 = Vector( m_Max.x, m_Max.y, m_Min.z ) * m;
	const Vector V4 = Vector( m_Min.x, m_Min.y, m_Max.z ) * m;
	const Vector V5 = Vector( m_Max.x, m_Min.y, m_Max.z ) * m;
	const Vector V6 = Vector( m_Min.x, m_Max.y, m_Max.z ) * m;
	const Vector V7 = Vector( m_Max.x, m_Max.y, m_Max.z ) * m;

	RetVal.m_Min.x = Min( Min( Min( V0.x, V1.x ), Min( V2.x, V3.x ) ), Min( Min( V4.x, V5.x ), Min( V6.x, V7.x ) ) );
	RetVal.m_Min.y = Min( Min( Min( V0.y, V1.y ), Min( V2.y, V3.y ) ), Min( Min( V4.y, V5.y ), Min( V6.y, V7.y ) ) );
	RetVal.m_Min.z = Min( Min( Min( V0.z, V1.z ), Min( V2.z, V3.z ) ), Min( Min( V4.z, V5.z ), Min( V6.z, V7.z ) ) );
	RetVal.m_Max.x = Max( Max( Max( V0.x, V1.x ), Max( V2.x, V3.x ) ), Max( Max( V4.x, V5.x ), Max( V6.x, V7.x ) ) );
	RetVal.m_Max.y = Max( Max( Max( V0.y, V1.y ), Max( V2.y, V3.y ) ), Max( Max( V4.y, V5.y ), Max( V6.y, V7.y ) ) );
	RetVal.m_Max.z = Max( Max( Max( V0.z, V1.z ), Max( V2.z, V3.z ) ), Max( Max( V4.z, V5.z ), Max( V6.z, V7.z ) ) );

	return RetVal;
}