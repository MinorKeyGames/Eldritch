#include "core.h"
#include "line.h"
#include "plane.h"
#include "sphere.h"
#include "triangle.h"
#include "mathcore.h"

Line::Line()
:	m_Point( Vector( 0, 0, 0 ) ),
	m_Direction( Vector( 0, 0, 1 ) ) {}

Line::Line( const Vector& Point, const Vector& Direction )
:	m_Point( Point ),
	m_Direction( Direction ) {}

Vector Line::NearestPointTo( const Vector& v ) const
{
	Vector Offset = v - m_Point;
	return m_Point + Offset.ProjectionOnto( m_Direction );
}

float Line::DistanceTo( const Vector& v ) const
{
	return ( v - NearestPointTo( v ) ).Length();
}

bool Line::Intersects( const Vector& v ) const
{
	return ( DistanceTo( v ) < EPSILON );
}

bool Line::Intersects( const Plane& p ) const
{
	return ( !Equal( m_Direction.Dot( p.m_Normal ), 0.f ) ||	// If the line isn't parallel to the plane,
			 m_Point == p.ProjectPoint( m_Point ) );			// or it is, but the point is on the plane, it intersects
}

bool Line::Intersects( const Sphere& s ) const
{
	return ( DistanceTo( s.m_Center ) <= s.m_Radius );
}

bool Line::Intersects( const Triangle& t, bool TwoSided ) const
{
	// This could be optimized a good bit, and should be if I use it a lot
	// See Real-Time Collision Detection p. 184

	Vector v1 = t.m_Vec1 - m_Point;
	Vector v2 = t.m_Vec2 - m_Point;
	Vector v3 = t.m_Vec3 - m_Point;

	float u = m_Direction.Dot( v3.Cross( v2 ) );
	float v = m_Direction.Dot( v1.Cross( v3 ) );
	float w = m_Direction.Dot( v2.Cross( v1 ) );

	if( TwoSided )
	{
		return ( ( u > 0.0f && v > 0.0f && w > 0.0f ) || ( u < 0.0f && v < 0.0f && w < 0.0f ) );
	}
	else
	{
		return ( u > 0.0f && v > 0.0f && w > 0.0f );
	}
}

Vector Line::GetIntersection( const Plane& p ) const
{
	float Dot = m_Direction.Dot( p.m_Normal );
	ASSERT( !Equal( Dot, 0.f ) );
	float Distance = p.DistanceTo( m_Point ) / Dot;
	return ( m_Point - ( Distance * m_Direction ) );
}