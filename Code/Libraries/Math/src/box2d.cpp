#include "core.h"
#include "box2d.h"
#include "mathcore.h"
#include "segment2d.h"
#include "triangle2d.h"
#include "collisioninfo2d.h"

Box2D::Box2D()
:	m_Min()
,	m_Max()
{
}

Box2D::Box2D( const Vector2& InMin, const Vector2& InMax )
:	m_Min( InMin )
,	m_Max( InMax )
{
}

float Box2D::GetArea() const
{
	DEBUGASSERT( m_Max.x >= m_Min.x );
	DEBUGASSERT( m_Max.y >= m_Min.y );

	Vector2 Extents = m_Max - m_Min;
	return Extents.x * Extents.y;
}

bool Box2D::Intersects( const Box2D& Box ) const
{
	if( m_Max.x < Box.m_Min.x || m_Min.x > Box.m_Max.x ||
		m_Max.y < Box.m_Min.y || m_Min.y > Box.m_Max.y )
	{
		return false;
	}

	return true;
}

bool Box2D::Contains( const Vector2& Point ) const
{
	if( m_Max.x < Point.x || m_Min.x > Point.x ||
		m_Max.y < Point.y || m_Min.y > Point.y )
	{
		return false;
	}

	return true;
}

bool Box2D::SweepAgainst( const Segment2D& Offset, const Array<Triangle2D>& Tris, CollisionInfo2D* const pInfo /*= NULL*/ ) const
{
	CollisionInfo2D Info;
	CollisionInfo2D MinInfo;

	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}

	const uint NumTris = Tris.Size();
	for( uint TriangleIndex = 0; TriangleIndex < NumTris; ++TriangleIndex )
	{
		if( SweepAgainst( Offset, Tris[ TriangleIndex ], &Info ) )
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

// Helper function, assumes CCW winding of segments
Segment2D ExpandSegment( const Vector2& Extents, const Vector2& Start, const Vector2& End )
{
	const Vector2 EdgeDir = End - Start;
	Segment2D TempSegment = Segment2D( Start, End );

	if( EdgeDir.x > 0.0f ) { TempSegment.m_Start.y -= Extents.y; TempSegment.m_End.y -= Extents.y; }
	if( EdgeDir.x < 0.0f ) { TempSegment.m_Start.y += Extents.y; TempSegment.m_End.y += Extents.y; }
	if( EdgeDir.y > 0.0f ) { TempSegment.m_Start.x += Extents.x; TempSegment.m_End.x += Extents.x; }
	if( EdgeDir.y < 0.0f ) { TempSegment.m_Start.x -= Extents.x; TempSegment.m_End.x -= Extents.x; }

	return TempSegment;
}

bool Box2D::SweepAgainst( const Segment2D& Offset, const Triangle2D& Tri, CollisionInfo2D* const pInfo /*= NULL*/ ) const
{
	CollisionInfo2D Info;
	CollisionInfo2D MinInfo;

	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}

	const Vector2 Extents = GetExtents();

	const Box2D VertexBoxA = Box2D( Tri.m_Vec1 - Extents, Tri.m_Vec1 + Extents );
	const Box2D VertexBoxB = Box2D( Tri.m_Vec2 - Extents, Tri.m_Vec2 + Extents );
	const Box2D VertexBoxC = Box2D( Tri.m_Vec3 - Extents, Tri.m_Vec3 + Extents );

	Segment2D EdgeSegmentA = ExpandSegment( Extents, Tri.m_Vec1, Tri.m_Vec2 );
	Segment2D EdgeSegmentB = ExpandSegment( Extents, Tri.m_Vec2, Tri.m_Vec3 );
	Segment2D EdgeSegmentC = ExpandSegment( Extents, Tri.m_Vec3, Tri.m_Vec1 );

	if( Offset.Intersects( EdgeSegmentA, &Info ) ) { if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision ) { MinInfo = Info; } }
	if( Offset.Intersects( EdgeSegmentB, &Info ) ) { if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision ) { MinInfo = Info; } }
	if( Offset.Intersects( EdgeSegmentC, &Info ) ) { if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision ) { MinInfo = Info; } }
	if( Offset.Intersects( VertexBoxA, &Info ) ) { if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision ) { MinInfo = Info; } }
	if( Offset.Intersects( VertexBoxB, &Info ) ) { if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision ) { MinInfo = Info; } }
	if( Offset.Intersects( VertexBoxC, &Info ) ) { if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision ) { MinInfo = Info; } }

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

void Box2D::ExpandTo( const Vector2& v )
{
	for( uint i = 0; i < 2; ++i )
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

void Box2D::ExpandBy( const Vector2& v )
{
	for( uint i = 0; i < 2; ++i )
	{
		m_Min.v[i] -= v.v[i];
		m_Max.v[i] += v.v[i];
	}
}

Vector2 Box2D::GetCenter() const
{
	const Vector2 Extents = ( m_Max - m_Min ) * 0.5f;
	return m_Max - Extents;
}

Vector2 Box2D::GetExtents() const
{
	return ( m_Max - m_Min ) * 0.5f;
}

void Box2D::GetCenterAndExtents( Vector2& OutCenter, Vector2& OutExtents ) const
{
	OutExtents = ( m_Max - m_Min ) * 0.5f;
	OutCenter = m_Max - OutExtents;
}

Box2D Box2D::operator+( const Vector2& Offset ) const
{
	return Box2D( m_Min + Offset, m_Max + Offset );
}

Box2D& Box2D::operator+=( const Vector2& Offset )
{
	m_Min += Offset;
	m_Max += Offset;

	return *this;
}

Box2D Box2D::operator-( const Vector2& Offset ) const
{
	return Box2D( m_Min - Offset, m_Max - Offset );
}

Box2D& Box2D::operator-=( const Vector2& Offset )
{
	m_Min -= Offset;
	m_Max -= Offset;

	return *this;
}

/*static*/ Box2D Box2D::GetBound( const Box2D& BoxA, const Box2D& BoxB )
{
	const float MinX = Min( BoxA.m_Min.x, BoxB.m_Min.x );
	const float MinY = Min( BoxA.m_Min.y, BoxB.m_Min.y );
	const float MaxX = Max( BoxA.m_Max.x, BoxB.m_Max.x );
	const float MaxY = Max( BoxA.m_Max.y, BoxB.m_Max.y );

	return Box2D( Vector2( MinX, MinY ), Vector2( MaxX, MaxY ) );
}