#include "core.h"
#include "segment2d.h"
#include "box2d.h"
#include "collisioninfo2d.h"
#include "triangle2d.h"
#include "mathcore.h"

Segment2D::Segment2D()
:	m_Start()
,	m_End()
{
}

Segment2D::Segment2D( const Vector2& InStart, const Vector2& InEnd )
:	m_Start( InStart )
,	m_End( InEnd )
{
}

bool Segment2D::Intersects( const Box2D& Box, CollisionInfo2D* const pInfo /*= NULL*/ ) const
{
	if( Box.Contains( m_Start ) )
	{
		if( pInfo )
		{
			pInfo->m_Collision = true;
			pInfo->m_EHitNormal = CollisionInfo2D::EHN_None;
			pInfo->m_HitNormal = Vector2();
			pInfo->m_HitT = 0.0f;
			pInfo->m_Intersection = m_Start;
		}

		return true;
	}

	float MinT = 0.0f;
	float MaxT = FLT_MAX;
	CollisionInfo2D::EHitNormal EnumHitNormal = CollisionInfo2D::EHN_None;
	Vector2 HitNormal;
	Vector2 Offset = m_End - m_Start;

	if( Abs( Offset.x ) < SMALLER_EPSILON )
	{
		if( m_Start.x < Box.m_Min.x || m_Start.x > Box.m_Max.x )
		{
			return false;
		}
	}
	else
	{
		float InvOffsetX = 1.0f / Offset.x;
		float T1 = ( Box.m_Min.x - m_Start.x ) * InvOffsetX;
		float T2 = ( Box.m_Max.x - m_Start.x ) * InvOffsetX;
		if( T1 > T2 )
		{
			Swap( T1, T2 );
		}
		MinT = Max( MinT, T1 );
		MaxT = Min( MaxT, T2 );
		if( MinT > MaxT )
		{
			return false;
		}

		if( MinT == T1 )
		{
			EnumHitNormal = ( Offset.x > 0.0f ) ? CollisionInfo2D::EHN_Left : CollisionInfo2D::EHN_Right;
			HitNormal.x = Sign( Offset.x );
		}
	}

	if( Abs( Offset.y ) < SMALLER_EPSILON )
	{
		if( m_Start.y < Box.m_Min.y || m_Start.y > Box.m_Max.y )
		{
			return false;
		}
	}
	else
	{
		float InvOffsetY = 1.0f / Offset.y;
		float T1 = ( Box.m_Min.y - m_Start.y ) * InvOffsetY;
		float T2 = ( Box.m_Max.y - m_Start.y ) * InvOffsetY;
		if( T1 > T2 )
		{
			Swap( T1, T2 );
		}
		MinT = Max( MinT, T1 );
		MaxT = Min( MaxT, T2 );
		if( MinT > MaxT )
		{
			return false;
		}

		if( MinT == T1 )
		{
			EnumHitNormal = ( Offset.y > 0.0f ) ? CollisionInfo2D::EHN_Up : CollisionInfo2D::EHN_Down;
			HitNormal.y = Sign( Offset.y );
		}
	}

	if( MinT > 1.0f )
	{
		return false;
	}

	ASSERT( EnumHitNormal != CollisionInfo2D::EHN_None );

	if( pInfo )
	{
		pInfo->m_Collision = true;
		pInfo->m_EHitNormal = EnumHitNormal;
		pInfo->m_HitNormal = HitNormal;
		pInfo->m_HitT = MinT;
		pInfo->m_Intersection = m_Start + Offset * MinT;
	}

	return true;
}

bool Segment2D::Intersects( const Array<Triangle2D>& Tris, CollisionInfo2D* const pInfo /*= NULL*/ ) const
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
		if( Intersects( Tris[ TriangleIndex ], &Info ) )
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

bool Segment2D::Intersects( const Triangle2D& Tri, CollisionInfo2D* const pInfo /*= NULL*/ ) const
{
	CollisionInfo2D Info;
	CollisionInfo2D MinInfo;

	if( pInfo )
	{
		Info.CopyInParametersFrom( *pInfo );
	}

	const Segment2D SegmentA = Segment2D( Tri.m_Vec1, Tri.m_Vec2 );
	const Segment2D SegmentB = Segment2D( Tri.m_Vec1, Tri.m_Vec3 );
	const Segment2D SegmentC = Segment2D( Tri.m_Vec2, Tri.m_Vec3 );

	if( Intersects( SegmentA, &Info ) )
	{
		if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
		{
			MinInfo = Info;
		}
	}

	if( Intersects( SegmentB, &Info ) )
	{
		if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
		{
			MinInfo = Info;
		}
	}

	if( Intersects( SegmentC, &Info ) )
	{
		if( Info.m_HitT < MinInfo.m_HitT || !MinInfo.m_Collision )
		{
			MinInfo = Info;
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

// Adapted from http://paulbourke.net/geometry/lineline2d/
bool Segment2D::Intersects( const Segment2D& Other, CollisionInfo2D* const pInfo /*= NULL*/ ) const
{
	const Vector2 Offset	= m_Start - Other.m_Start;
	const Vector2 Dir		= m_End - m_Start;
	const Vector2 OtherDir	= Other.m_End - Other.m_Start;

	const float Denom	= OtherDir.y	* Dir.x		- OtherDir.x	* Dir.y;
	const float NumerA	= OtherDir.x	* Offset.y	- OtherDir.y	* Offset.x;
	const float NumerB	= Dir.x			* Offset.y	- Dir.y			* Offset.x;

	// NOTE: I'm ignoring coincident segments for now.
	//// Are the segments coincident?
	//if( Abs( NumerA ) < EPSILON && Abs( NumerB ) < EPSILON && Abs( Denom ) < EPSILON )
	//{
	//	// TODO: Fill in pInfo
	//	return true;
	//}

	// Are the segments parallel?
	if( Abs( Denom ) < EPSILON )
	{
		return false;
	}

	const float UA = NumerA / Denom;
	const float UB = NumerB / Denom;
	if( UA < 0.0f || UA > 1.0f || UB < 0.0f || UB > 1.0f )
	{
		return false;
	}

	if( pInfo )
	{
		pInfo->m_Collision = true;
		pInfo->m_HitT = UA;
		pInfo->m_Intersection = m_Start + UA * Dir;

		// Create a normal perpendicular to the right of the other segment.
		// This works if the segment is part of a CCW-wound triangle, for example.
		pInfo->m_HitNormal = Vector2( OtherDir.y, -OtherDir.x ).GetNormalized();
	}

	return true;
}

Segment2D Segment2D::operator+( const Vector2& Offset ) const
{
	return Segment2D( m_Start + Offset, m_End + Offset );
}

Segment2D& Segment2D::operator+=( const Vector2& Offset )
{
	m_Start += Offset;
	m_End += Offset;

	return *this;
}

Segment2D Segment2D::operator-( const Vector2& Offset ) const
{
	return Segment2D( m_Start - Offset, m_End - Offset );
}

Segment2D& Segment2D::operator-=( const Vector2& Offset )
{
	m_Start -= Offset;
	m_End -= Offset;

	return *this;
}