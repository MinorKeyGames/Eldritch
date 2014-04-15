#include "core.h"
#include "sphere.h"
#include "plane.h"
#include "frustum.h"
#include "line.h"
#include "triangle.h"
#include "collisioninfo.h"
#include "aabb.h"
#include "collisionmesh.h"
#include "mathcore.h"
#include "ray.h"
#include "segment.h"
#include "cylinder.h"

Sphere::Sphere()
:	m_Center( Vector(0,0,0) ),
	m_Radius( 1.f ) {}

Sphere::Sphere( const Vector& Center, float Radius )
:	m_Center( Center ),
	m_Radius( Radius ) {}

bool Sphere::Intersects( const Vector& v ) const
{
	return ( ( v - m_Center ).LengthSquared() <= ( m_Radius * m_Radius ) );
}

bool Sphere::Intersects( const Plane& p ) const
{
	return ( Abs( p.DistanceTo( m_Center ) ) <= m_Radius );
}

bool Sphere::Intersects( const Frustum& f ) const
{
	return f.Intersects( *this );
}

bool Sphere::Intersects( const Sphere& s ) const
{
	return ( ( s.m_Center - m_Center ).LengthSquared() <= ( ( s.m_Radius + m_Radius ) * ( s.m_Radius + m_Radius ) ) );
}

bool Sphere::Intersects( const Line& l ) const
{
	return l.Intersects( *this );
}

bool Sphere::Intersects( const Triangle& t, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	Vector p = t.GetClosestPoint( m_Center );
	Vector v = p - m_Center;

	if( v.Dot( v ) <= m_Radius * m_Radius )
	{
		if( pInfo )
		{
			pInfo->m_Collision = true;
			pInfo->m_Intersection = t.GetClosestPoint( m_Center );
			pInfo->m_Plane = t.GetPlane();	// NOTE: There's no plane defined for inside the sphere, so just use the triangle
		}

		return true;
	}

	return false;
}

bool Sphere::Intersects( const AABB& a ) const
{
	return ( a.GetSquaredDistance( m_Center ) < ( m_Radius * m_Radius ) );
}

bool Sphere::Intersects( const CollisionMesh& m, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	return m.Intersects( *this, pInfo );
}

bool Sphere::Intersects( const Ray& r, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	Vector Offset = r.m_Point - m_Center;
	float b = Offset.Dot( r.m_Direction );
	float c = Offset.Dot( Offset ) - m_Radius * m_Radius;

	if( c > 0.0f && b > 0.0f )
	{
		return false;
	}

	float Discriminant = b * b - c;

	if( Discriminant < 0.0f )
	{
		return false;
	}

	float t = Max( 0.0f, -b - SqRt( Discriminant ) );

	if( pInfo )
	{
		pInfo->m_Collision = true;
		pInfo->m_HitT = t;
		pInfo->m_Intersection = r.m_Point + t * r.m_Direction;
	}

	return true;
}

bool Sphere::SweepAgainst( const AABB& a, const Vector& v ) const
{
	// Cheap test: just intersect against the expanded AABB instead of the proper
	// Minkowski volume formed by sweeping the sphere around the AABB. This is
	// a liberal test and may return true when the objects do not actually intersect.
	// That's fine for now, because I just use this as a rough test.
	AABB ExpandedAABB = a;
	ExpandedAABB.m_Min.x -= m_Radius;
	ExpandedAABB.m_Min.y -= m_Radius;
	ExpandedAABB.m_Min.z -= m_Radius;
	ExpandedAABB.m_Max.x += m_Radius;
	ExpandedAABB.m_Max.y += m_Radius;
	ExpandedAABB.m_Max.z += m_Radius;

	return Segment( m_Center, m_Center + v ).Intersects( ExpandedAABB );
}

bool Sphere::SweepAgainst( const Plane& p, const Vector& v, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	float Dist = p.DistanceTo( m_Center );
	float Denom = p.m_Normal.Dot( v );

	// If sphere is already intersecting plane, there's no single
	// point of intersection and this returns false.
	if( Denom * Dist >= 0.0f )
	{
		return false;
	}
	else
	{
		float r = ( Dist > 0.0f ) ? m_Radius : -m_Radius;
		float t = Max( 0.0f, ( r - Dist ) / Denom );
		if( t <= 1.0f )
		{
			if( pInfo )
			{
				pInfo->m_Collision = true;
				pInfo->m_HitT = t;
				pInfo->m_Intersection = m_Center + t * v - r * p.m_Normal;
				pInfo->m_Plane = p;
			}
			return true;
		}
		return false;
	}
}

bool Sphere::SweepAgainst( const Triangle& t, const Vector& v, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	CollisionInfo Info;
	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}
	Plane TriPlane( t.GetPlane() );

	// Now handling case of sphere already in plane but not intersecting triangle
	bool PlaneSweep = SweepAgainst( TriPlane, v, &Info );
	if( PlaneSweep || Intersects( TriPlane ) )
	{
		if( PlaneSweep && t.Contains( Info.m_Intersection ) )
		{
			if( pInfo )
			{
				pInfo->CopyOutParametersFrom( Info );
			}
			return true;
		}
		else
		{
			Segment TestSeg( m_Center, m_Center + v );
			Cylinder Edge12Cylinder( t.m_Vec1, t.m_Vec2, m_Radius );
			Cylinder Edge13Cylinder( t.m_Vec1, t.m_Vec3, m_Radius );
			Cylinder Edge23Cylinder( t.m_Vec2, t.m_Vec3, m_Radius );

			// Test swept sphere against triangle edges (segment-cylinder tests) (return closest)
			// NOTE: Don't test against endcaps of cylinder, because segment would hit a vert-sphere first
			CollisionInfo MinInfo;
			if( Edge12Cylinder.Intersects( TestSeg, &Info ) )
			{
				MinInfo = Info;
			}
			if( Edge13Cylinder.Intersects( TestSeg, &Info ) )
			{
				if( !MinInfo.m_Collision || Info.m_HitT < MinInfo.m_HitT )
				{
					MinInfo = Info;
				}
			}
			if( Edge23Cylinder.Intersects( TestSeg, &Info ) )
			{
				if( !MinInfo.m_Collision || Info.m_HitT < MinInfo.m_HitT )
				{
					MinInfo = Info;
				}
			}

			if( MinInfo.m_Collision )
			{
				if( pInfo )
				{
					pInfo->CopyOutParametersFrom( MinInfo );
					// Set the plane as the sphere's tangent plane at the point
					// of intersection, pointing *towards* the sphere
					//pInfo->m_Plane = Plane( ( m_Center - pInfo->m_Intersection ).GetNormalized(), pInfo->m_Intersection );
					pInfo->m_Intersection -= pInfo->m_Plane.m_Normal.GetNormalized() * m_Radius;
				}
				return true;
			}

			// If nothing, test swept sphere against triangles verts (segment-sphere tests) (return closest)

			Sphere Vert1Sphere( t.m_Vec1, m_Radius );
			Sphere Vert2Sphere( t.m_Vec2, m_Radius );
			Sphere Vert3Sphere( t.m_Vec3, m_Radius );

			if( Vert1Sphere.Intersects( TestSeg, &Info ) )
			{
				MinInfo = Info;
			}
			if( Vert2Sphere.Intersects( TestSeg, &Info ) )
			{
				if( !MinInfo.m_Collision || Info.m_HitT < MinInfo.m_HitT )
				{
					MinInfo = Info;
				}
			}
			if( Vert3Sphere.Intersects( TestSeg, &Info ) )
			{
				if( !MinInfo.m_Collision || Info.m_HitT < MinInfo.m_HitT )
				{
					MinInfo = Info;
				}
			}

			if( MinInfo.m_Collision )
			{
				if( pInfo )
				{
					pInfo->CopyOutParametersFrom( MinInfo );
					// Set the plane as the sphere's tangent plane at the point
					// of intersection, pointing *towards* the sphere
					//pInfo->m_Plane = Plane( ( m_Center - pInfo->m_Intersection ).GetNormalized(), pInfo->m_Intersection );
					pInfo->m_Intersection -= pInfo->m_Plane.m_Normal.GetNormalized() * m_Radius;
				}
				return true;
			}

			// If still nothing, no intersection
			
			return false;
		}
	}
	return false;
}

bool Sphere::Intersects( const Segment& s, CollisionInfo* const pInfo /*= NULL*/ ) const
{
	Vector SegmentVec = s.m_Point2 - s.m_Point1;
	Vector Direction = SegmentVec.GetNormalized();
	Vector Offset = s.m_Point1 - m_Center;
	float b = Offset.Dot( Direction );
	float c = Offset.Dot( Offset ) - m_Radius * m_Radius;

	if( c > 0.0f && b > 0.0f )
	{
		return false;
	}

	float Discriminant = b * b - c;

	if( Discriminant < 0.0f )
	{
		return false;
	}

	float t = Max( 0.0f, -b - SqRt( Discriminant ) );

	t /= SegmentVec.Length();

	if( t <= 1.0f )
	{
		if( pInfo )
		{
			Vector Intersection = s.m_Point1 + t * SegmentVec;
			Vector Normal = ( Intersection - m_Center ).GetNormalized();

			pInfo->m_Collision = true;
			pInfo->m_Intersection = Intersection;
			pInfo->m_HitT = t;
			pInfo->m_Plane = Plane( Normal, m_Center );
		}

		return true;
	}

	return false;
}