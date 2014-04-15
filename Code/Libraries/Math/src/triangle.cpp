#include "core.h"
#include "triangle.h"
#include "line.h"
#include "segment.h"
#include "ray.h"
#include "sphere.h"
#include "mathcore.h"
#include "ellipsoid.h"

Triangle::Triangle()
:	m_Vec1()
,	m_Vec2()
,	m_Vec3()
{
}

Triangle::Triangle( const Vector& Vec1, const Vector& Vec2, const Vector& Vec3 )
:	m_Vec1( Vec1 )
,	m_Vec2( Vec2 )
,	m_Vec3( Vec3 )
{
}

Triangle Triangle::GetFlipped() const
{
	return Triangle( m_Vec1, m_Vec3, m_Vec2 );
}

Vector Triangle::GetNormal() const
{
	return ( ( m_Vec2 - m_Vec1 ).Cross( m_Vec3 - m_Vec1 ) ).GetNormalized();
}

Plane Triangle::GetPlane() const
{
	Vector Normal = ( ( m_Vec2 - m_Vec1 ).Cross( m_Vec3 - m_Vec1 ) ).GetNormalized();
	float Distance = -Normal.Dot( m_Vec1 );
	return Plane( Normal, Distance );
}

bool Triangle::IsInFrontOf( const Plane& P ) const
{
	return ( P.OnFrontSide( m_Vec1 ) && P.OnFrontSide( m_Vec2 ) && P.OnFrontSide( m_Vec3 ) );
}

bool Triangle::IsInBackOf( const Plane& P ) const
{
	return ( !P.OnFrontSide( m_Vec1 ) && !P.OnFrontSide( m_Vec2 ) && !P.OnFrontSide( m_Vec3 ) );
}

Vector Triangle::GetClosestPoint( const Vector& v ) const
{
	return GetClosestPoint( v, NULL );
}

// Real-Time Collision Detection p.139
Vector Triangle::GetClosestPoint( const Vector& v, Vector* OutBarycentricCoords ) const
{
	Vector ab = m_Vec2 - m_Vec1;
	Vector ac = m_Vec3 - m_Vec1;
	Vector bc = m_Vec3 - m_Vec2;
	Vector av = v - m_Vec1;
	Vector bv = v - m_Vec2;
	Vector cv = v - m_Vec3;
	Vector va = -av;
	Vector vb = -bv;
	Vector vc = -cv;

	float snom = av.Dot( ab );
	float sdenom = bv.Dot( m_Vec1 - m_Vec2 );

	float tnom = av.Dot( ac );
	float tdenom = cv.Dot( m_Vec1 - m_Vec3 );

	if( snom <= 0.0f && tnom <= 0.0f )
	{
		if( OutBarycentricCoords )
		{
			*OutBarycentricCoords = Vector( 1.0f, 0.0f, 0.0f );
		}
		return m_Vec1;
	}

	float unom = bv.Dot( bc );
	float udenom = cv.Dot( m_Vec2 - m_Vec3 );

	if( sdenom <= 0.0f && unom <= 0.0f )
	{
		if( OutBarycentricCoords )
		{
			*OutBarycentricCoords = Vector( 0.0f, 1.0f, 0.0f );
		}
		return m_Vec2;
	}

	if( tdenom <= 0.0f && udenom <= 0.0f )
	{
		if( OutBarycentricCoords )
		{
			*OutBarycentricCoords = Vector( 0.0f, 0.0f, 1.0f );
		}
		return m_Vec3;
	}

	Vector n = ab.Cross( ac );

	float c = n.Dot( va.Cross( vb ) );

	if( c < 0.0f && snom >= 0.0f && sdenom >= 0.0f )
	{
		const float baryV = snom / ( snom + sdenom );
		if( OutBarycentricCoords )
		{
			*OutBarycentricCoords = Vector( 1.0f - baryV, baryV, 0.0f );
		}
		return m_Vec1 + ab * baryV;
	}

	float a = n.Dot( vb.Cross( vc ) );

	if( a <= 0.0f && unom >= 0.0f && udenom >= 0.0f )
	{
		const float baryW = unom / ( unom + udenom );
		if( OutBarycentricCoords )
		{
			*OutBarycentricCoords = Vector( 0.0f, 1.0f - baryW, baryW );
		}
		return m_Vec2 + bc * baryW;
	}

	float b = n.Dot( vc.Cross( va ) );

	if( b <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f )
	{
		const float baryW = tnom / ( tnom + tdenom );
		if( OutBarycentricCoords )
		{
			*OutBarycentricCoords = Vector( 1.0f - baryW, 0.0f, baryW );
		}
		return m_Vec1 + ac * baryW;
	}

	float baryU = a / ( a + b + c );
	float baryV = b / ( a + b + c );
	float baryW = 1.0f - baryU - baryV;

	if( OutBarycentricCoords )
	{
		*OutBarycentricCoords = Vector( baryU, baryV, baryW );
	}

	return baryU * m_Vec1 + baryV * m_Vec2 + baryW * m_Vec3;
}

bool Triangle::Contains( const Vector& p ) const
{
	// If p isn't in plane of triangle, it can't be contained
	if( Abs( GetPlane().DistanceTo( p ) ) > EPSILON )
	{
		return false;
	}

	Vector a = m_Vec1 - p;
	Vector b = m_Vec2 - p;
	Vector c = m_Vec3 - p;

	Vector u = b.Cross( c );
	Vector v = c.Cross( a );

	if( u.Dot( v ) < 0.0f )
	{
		return false;
	}

	Vector w = a.Cross( b );

	if( u.Dot( w ) < 0.0f )
	{
		return false;
	}

	// NOTE: If point lies along the line of one of the triangle's edges,
	// the previous checks may not catch if it is outside the triangle.
	if( u.LengthSquared() < EPSILON )
	{
		if( b.Dot( c ) > 0.0f )
		{
			return false;
		}
	}

	if( v.LengthSquared() < EPSILON )
	{
		if( c.Dot( a ) > 0.0f )
		{
			return false;
		}
	}

	if( w.LengthSquared() < EPSILON )
	{
		if( a.Dot( b ) > 0.0f )
		{
			return false;
		}
	}

	return true;
}

bool Triangle::Intersects( const Line& l, bool TwoSided ) const
{
	return l.Intersects( *this, TwoSided );
}

bool Triangle::Intersects( const Segment& s, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return s.Intersects( *this, pInfo );
}

bool Triangle::Intersects( const Ray& r, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return r.Intersects( *this, pInfo );
}

bool Triangle::Intersects( const Sphere& s, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return s.Intersects( *this, pInfo );
}

bool Triangle::Intersects( const Ellipsoid& e, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return e.Intersects( *this, pInfo );
}

Vector Triangle::GetCentroid() const
{
	return ( m_Vec1 + m_Vec2 + m_Vec3 ) * ( 1.0f / 3.0f );
}

// Four square roots, not cheap
// Heron's formula
float Triangle::GetArea() const
{
	float A = ( m_Vec2 - m_Vec1 ).Length();
	float B = ( m_Vec3 - m_Vec2 ).Length();
	float C = ( m_Vec1 - m_Vec3 ).Length();
	float S = ( A + B + C ) / 2.0f;
	return SqRt( S * ( S - A ) * ( S - B ) * ( S - C ) );
}

AABB Triangle::GetAABB() const
{
	AABB TriangleAABB( m_Vec1, m_Vec1 );
	TriangleAABB.ExpandTo( m_Vec2 );
	TriangleAABB.ExpandTo( m_Vec3 );
	return TriangleAABB;
}