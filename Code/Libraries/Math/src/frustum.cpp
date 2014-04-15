#include "core.h"
#include "frustum.h"
#include "vector.h"
#include "aabb.h"
#include "matrix.h"
#include "sphere.h"

Frustum::Frustum()
{
	m_Planes[ EFS_RIGHT ].m_Normal = Vector( -1, 0, 0 );
	m_Planes[ EFS_RIGHT ].m_Distance = 1.f;
	m_Planes[ EFS_LEFT ].m_Normal = Vector( 1, 0, 0 );
	m_Planes[ EFS_LEFT ].m_Distance = 1.f;
	m_Planes[ EFS_BOTTOM ].m_Normal = Vector( 0, 0, 1 );
	m_Planes[ EFS_BOTTOM ].m_Distance = 1.f;
	m_Planes[ EFS_TOP ].m_Normal = Vector( 0, 0, -1 );
	m_Planes[ EFS_TOP ].m_Distance = 1.f;
	m_Planes[ EFS_NEAR ].m_Normal = Vector( 0, 1, 0 );
	m_Planes[ EFS_NEAR ].m_Distance = 1.f;
	m_Planes[ EFS_FAR ].m_Normal = Vector( 0, -1, 0 );
	m_Planes[ EFS_FAR ].m_Distance = 1.f;
}

Frustum::Frustum( const Matrix& m )
{
	InitWith( m );
}

Frustum::Frustum( const Frustum& f )
{
	m_Planes[ EFS_RIGHT ] = f.m_Planes[ EFS_RIGHT ];
	m_Planes[ EFS_LEFT ] = f.m_Planes[ EFS_LEFT ];
	m_Planes[ EFS_BOTTOM ] = f.m_Planes[ EFS_BOTTOM ];
	m_Planes[ EFS_TOP ] = f.m_Planes[ EFS_TOP ];
	m_Planes[ EFS_NEAR ] = f.m_Planes[ EFS_NEAR ];
	m_Planes[ EFS_FAR ] = f.m_Planes[ EFS_FAR ];
}

void Frustum::InitWith( const Matrix& m )
{
	m_Planes[ EFS_RIGHT ].m_Normal.x = m.m[0][3] - m.m[0][0];
	m_Planes[ EFS_RIGHT ].m_Normal.y = m.m[1][3] - m.m[1][0];
	m_Planes[ EFS_RIGHT ].m_Normal.z = m.m[2][3] - m.m[2][0];
	m_Planes[ EFS_RIGHT ].m_Distance = m.m[3][3] - m.m[3][0];
	m_Planes[ EFS_RIGHT ].Normalize();

	m_Planes[ EFS_LEFT ].m_Normal.x = m.m[0][3] + m.m[0][0];
	m_Planes[ EFS_LEFT ].m_Normal.y = m.m[1][3] + m.m[1][0];
	m_Planes[ EFS_LEFT ].m_Normal.z = m.m[2][3] + m.m[2][0];
	m_Planes[ EFS_LEFT ].m_Distance = m.m[3][3] + m.m[3][0];
	m_Planes[ EFS_LEFT ].Normalize();

	m_Planes[ EFS_BOTTOM ].m_Normal.x = m.m[0][3] + m.m[0][1];
	m_Planes[ EFS_BOTTOM ].m_Normal.y = m.m[1][3] + m.m[1][1];
	m_Planes[ EFS_BOTTOM ].m_Normal.z = m.m[2][3] + m.m[2][1];
	m_Planes[ EFS_BOTTOM ].m_Distance = m.m[3][3] + m.m[3][1];
	m_Planes[ EFS_BOTTOM ].Normalize();

	m_Planes[ EFS_TOP ].m_Normal.x = m.m[0][3] - m.m[0][1];
	m_Planes[ EFS_TOP ].m_Normal.y = m.m[1][3] - m.m[1][1];
	m_Planes[ EFS_TOP ].m_Normal.z = m.m[2][3] - m.m[2][1];
	m_Planes[ EFS_TOP ].m_Distance = m.m[3][3] - m.m[3][1];
	m_Planes[ EFS_TOP ].Normalize();

	m_Planes[ EFS_FAR ].m_Normal.x = m.m[0][3] - m.m[0][2];
	m_Planes[ EFS_FAR ].m_Normal.y = m.m[1][3] - m.m[1][2];
	m_Planes[ EFS_FAR ].m_Normal.z = m.m[2][3] - m.m[2][2];
	m_Planes[ EFS_FAR ].m_Distance = m.m[3][3] - m.m[3][2];
	m_Planes[ EFS_FAR ].Normalize();

	m_Planes[ EFS_NEAR ].m_Normal.x = m.m[0][3] + m.m[0][2];
	m_Planes[ EFS_NEAR ].m_Normal.y = m.m[1][3] + m.m[1][2];
	m_Planes[ EFS_NEAR ].m_Normal.z = m.m[2][3] + m.m[2][2];
	m_Planes[ EFS_NEAR ].m_Distance = m.m[3][3] + m.m[3][2];
	m_Planes[ EFS_NEAR ].Normalize();
}

bool Frustum::Intersects( const Vector& Point ) const
{
	for( int i = 0; i < 6; ++i )
	{
		if( !m_Planes[i].OnFrontSide( Point ) )
		{
			return false;
		}
	}
	return true;
}

// For future reference, this IS correct--look at what it's doing.
// If (and only if) all the points are on the back side of any one
// plane, the box is outside.
bool Frustum::Intersects( const AABB& Box ) const
{
	for( int i = 0; i < 6; ++i )
	{
		// This is effectively a plane-point side test: dot( normal, point ) + distance > 0 <=> point on front side of plane
		if( m_Planes[i].m_Normal.x * Box.m_Min.x + m_Planes[i].m_Normal.y * Box.m_Min.y + m_Planes[i].m_Normal.z * Box.m_Min.z + m_Planes[i].m_Distance > 0.0f ) continue;
		if( m_Planes[i].m_Normal.x * Box.m_Min.x + m_Planes[i].m_Normal.y * Box.m_Min.y + m_Planes[i].m_Normal.z * Box.m_Max.z + m_Planes[i].m_Distance > 0.0f ) continue;
		if( m_Planes[i].m_Normal.x * Box.m_Min.x + m_Planes[i].m_Normal.y * Box.m_Max.y + m_Planes[i].m_Normal.z * Box.m_Min.z + m_Planes[i].m_Distance > 0.0f ) continue;
		if( m_Planes[i].m_Normal.x * Box.m_Min.x + m_Planes[i].m_Normal.y * Box.m_Max.y + m_Planes[i].m_Normal.z * Box.m_Max.z + m_Planes[i].m_Distance > 0.0f ) continue;
		if( m_Planes[i].m_Normal.x * Box.m_Max.x + m_Planes[i].m_Normal.y * Box.m_Min.y + m_Planes[i].m_Normal.z * Box.m_Min.z + m_Planes[i].m_Distance > 0.0f ) continue;
		if( m_Planes[i].m_Normal.x * Box.m_Max.x + m_Planes[i].m_Normal.y * Box.m_Min.y + m_Planes[i].m_Normal.z * Box.m_Max.z + m_Planes[i].m_Distance > 0.0f ) continue;
		if( m_Planes[i].m_Normal.x * Box.m_Max.x + m_Planes[i].m_Normal.y * Box.m_Max.y + m_Planes[i].m_Normal.z * Box.m_Min.z + m_Planes[i].m_Distance > 0.0f ) continue;
		if( m_Planes[i].m_Normal.x * Box.m_Max.x + m_Planes[i].m_Normal.y * Box.m_Max.y + m_Planes[i].m_Normal.z * Box.m_Max.z + m_Planes[i].m_Distance > 0.0f ) continue;
		return false;
	}
	return true;
}

bool Frustum::Intersects( const Sphere& s ) const
{
	for( int i = 0; i < 6; ++i )
	{
		// If the sphere lies outside any one plane, it's not an intersection
		if( m_Planes[i].DistanceTo( s.m_Center ) < -s.m_Radius )
		{
			return false;
		}
	}
	return true;
}

// Very slow
// Like the AABB test, this IS correct--look at what it's doing.
// If (and only if) all the points are on the back side of any one
// plane, the box is outside.
bool Frustum::Intersects( const Frustum& f ) const
{
	Vector Corners[8];
	f.GetCorners( Corners );
	for( int i = 0; i < 6; ++i )
	{
		if( m_Planes[i].OnFrontSide( Corners[0] ) ) continue;
		if( m_Planes[i].OnFrontSide( Corners[1] ) ) continue;
		if( m_Planes[i].OnFrontSide( Corners[2] ) ) continue;
		if( m_Planes[i].OnFrontSide( Corners[3] ) ) continue;
		if( m_Planes[i].OnFrontSide( Corners[4] ) ) continue;
		if( m_Planes[i].OnFrontSide( Corners[5] ) ) continue;
		if( m_Planes[i].OnFrontSide( Corners[6] ) ) continue;
		if( m_Planes[i].OnFrontSide( Corners[7] ) ) continue;
		return false;
	}
	return true;
}

void Frustum::GetCorners( Vector Corners[8] ) const
{
	// Ordering:
	// f/n, t/b, l/r,
	// or:	0---1
	//		|\  |\
	//		2-4-3-5
	//		 \|  \|
	//		  6---7

	Line FarLeft = m_Planes[ EFS_LEFT ].GetIntersection( m_Planes[ EFS_FAR ] );
	Line FarRight = m_Planes[ EFS_RIGHT ].GetIntersection( m_Planes[ EFS_FAR ] );
	Line NearLeft = m_Planes[ EFS_LEFT ].GetIntersection( m_Planes[ EFS_NEAR ] );
	Line NearRight = m_Planes[ EFS_RIGHT ].GetIntersection( m_Planes[ EFS_NEAR ] );

	Corners[0] = FarLeft.GetIntersection( m_Planes[ EFS_TOP ] );
	Corners[1] = FarRight.GetIntersection( m_Planes[ EFS_TOP ] );
	Corners[2] = FarLeft.GetIntersection( m_Planes[ EFS_BOTTOM ] );
	Corners[3] = FarRight.GetIntersection( m_Planes[ EFS_BOTTOM ] );
	Corners[4] = NearLeft.GetIntersection( m_Planes[ EFS_TOP ] );
	Corners[5] = NearRight.GetIntersection( m_Planes[ EFS_TOP ] );
	Corners[6] = NearLeft.GetIntersection( m_Planes[ EFS_BOTTOM ] );
	Corners[7] = NearRight.GetIntersection( m_Planes[ EFS_BOTTOM ] );
}