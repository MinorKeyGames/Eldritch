#include "core.h"
#include "plane.h"
#include "sphere.h"
#include "aabb.h"
#include "ray.h"
#include "segment.h"
#include "mathcore.h"

Plane::Plane()
:	m_Normal()
,	m_Distance( 0.0f )
{
}

Plane::Plane( const Vector& Normal, float Distance )
:	m_Normal( Normal )
,	m_Distance( Distance )
{
}

Plane::Plane( const Vector& Normal, const Vector& Point )
:	m_Normal( Normal )
,	m_Distance( -Normal.Dot( Point ) )
{
}

Plane::Plane( const Plane& p )
:	m_Normal( p.m_Normal )
,	m_Distance( p.m_Distance )
{
}

Plane& Plane::operator=( const Plane& p )
{
	m_Normal = p.m_Normal;
	m_Distance = p.m_Distance;
	return *this;
}

void Plane::Negate()
{
	m_Normal.Negate();
	m_Distance = -m_Distance;
}

void Plane::Normalize()
{
	float RecL = 1.f / m_Normal.Length();
	m_Normal *= RecL;
	m_Distance *= RecL;
}

void Plane::FastNormalize()
{
	float RecL = FastInvSqRt( m_Normal.LengthSquared() );
	m_Normal *= RecL;
	m_Distance *= RecL;
}

Plane Plane::GetNormalized() const
{
	float RecL = 1.f / m_Normal.Length();
	return Plane( m_Normal * RecL, m_Distance * RecL );
}

Plane Plane::GetFastNormalized() const
{
	float RecL = FastInvSqRt( m_Normal.LengthSquared() );
	return Plane( m_Normal * RecL, m_Distance * RecL );
}

float Plane::DistanceTo( const Vector& v ) const
{
	return m_Normal.Dot( v ) + m_Distance;
}

// The only difference between ProjectPoint and
// ProjectVector is the addition of m_Distance...
Vector Plane::ProjectPoint( const Vector& v ) const
{
	return ( v - ( m_Normal * DistanceTo( v ) ) );
}

Vector Plane::ProjectVector( const Vector& v ) const
{
	return ( v - ( m_Normal * m_Normal.Dot( v ) ) );
}

Vector Plane::Reflect( const Vector& v ) const
{
	return ( v - ( 2.0f * m_Normal * DistanceTo( v ) ) );
}

bool Plane::OnFrontSide( const Vector& v ) const
{
	return ( DistanceTo( v ) > -EPSILON );
}

bool Plane::OnBackSide( const Vector& v ) const
{
	return ( DistanceTo( v ) < EPSILON );
}

bool Plane::Contains( const Vector& v ) const
{
	return ( Abs( DistanceTo( v ) ) < EPSILON );
}

bool Plane::Intersects( const Sphere& s ) const
{
	return s.Intersects( *this );
}

bool Plane::Intersects( const Plane& p ) const
{
	// Stupidly simple, but assumes normalized planes
	return ( !Equal( Abs( m_Normal.Dot( p.m_Normal ) ), 1.f ) );
}

bool Plane::Intersects( const Line& l ) const
{
	return l.Intersects( *this );
}

Line Plane::GetIntersection( const Plane& p ) const
{
	// Assuming normalized planes--if not, there's more to calculate
	// See http://local.wasp.uwa.edu.au/~pbourke/geometry/planeplane/
	float NDotN = m_Normal.Dot( p.m_Normal );
	float RecDeterminant = 1.f / ( 1.f - ( NDotN * NDotN ) );
	float c1 = ( -m_Distance + ( p.m_Distance * NDotN ) ) * RecDeterminant;
	float c2 = ( -p.m_Distance + ( m_Distance * NDotN ) ) * RecDeterminant;

	return Line( ( c1 * m_Normal ) + ( c2 * p.m_Normal ), m_Normal.Cross( p.m_Normal ).GetNormalized() );
}

Vector Plane::GetIntersection( const Line& l ) const
{
	return l.GetIntersection( *this );
}

bool Plane::IsCoplanarWith( const Plane& p ) const
{
	return ( ( ( m_Normal == p.m_Normal ) && ( Abs( m_Distance - p.m_Distance ) < EPSILON ) )
		|| ( ( m_Normal == -p.m_Normal ) && ( Abs( m_Distance + p.m_Distance ) < EPSILON ) ) );
}

bool Plane::IsFacingSameAs( const Plane& p ) const
{
	return ( m_Normal.Dot( p.m_Normal ) > 0.f );
}

bool Plane::Intersects( const AABB& a ) const
{
	Vector BoxCenter = ( a.m_Min + a.m_Max ) * 0.5f;
	Vector BoxHalfLengths = a.m_Max - BoxCenter;

	float Radius =	BoxHalfLengths.x * Abs( m_Normal.x ) +
					BoxHalfLengths.y * Abs( m_Normal.y ) +
					BoxHalfLengths.z * Abs( m_Normal.z );

	float Offset = m_Normal.Dot( BoxCenter ) + m_Distance;

	return Abs( Offset ) <= Radius;
}

bool Plane::Intersects( const Ray& r, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return r.Intersects( *this, pInfo );
}

bool Plane::Intersects( const Segment& s, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return s.Intersects( *this, pInfo );
}