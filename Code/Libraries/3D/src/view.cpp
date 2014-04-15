#include "core.h"
#include "3d.h"
#include "view.h"
#include "matrix.h"
#include "irenderer.h"
#include "frustum.h"
#include "mathcore.h"
#include "vector4.h"
#include "vector2.h"
#include "box2d.h"
#include "segment2d.h"
#include "collisioninfo2d.h"
#include "mathcore.h"

View::View()
:	m_Location( 0.f, 0.f, 0.f )
,	m_Rotation( 0.f, 0.f, 0.f )
,	m_NearClip( .01f )
,	m_FarClip( 3000.f )
,	m_InvRange( 0.0f )
,	m_VerticalFOV( 90.f )
,	m_AspectRatio( kAspect_4_3 )
,	m_OrthoMode( false )
,	m_Bounds( SRect( 0, 0, 640, -480 ) )
{
	UpdateInvRange();
}

View::View( const Vector& Location, const Vector& Direction, float VerticalFOV, float AspectRatio, float NearClip, float FarClip )
:	m_Location( Location )
,	m_Rotation( Direction.ToAngles() )
,	m_NearClip( NearClip )
,	m_FarClip( FarClip )
,	m_InvRange( 0.0f )
,	m_VerticalFOV( VerticalFOV )
,	m_AspectRatio( AspectRatio )
,	m_OrthoMode( false )
,	m_Bounds( SRect( 0, 0, 640, -480 ) )
{
	UpdateInvRange();
}

View::View( const Vector& Location, const Angles& Rotation, float VerticalFOV, float AspectRatio, float NearClip, float FarClip )
:	m_Location( Location )
,	m_Rotation( Rotation )
,	m_NearClip( NearClip )
,	m_FarClip( FarClip )
,	m_InvRange( 0.0f )
,	m_VerticalFOV( VerticalFOV )
,	m_AspectRatio( AspectRatio )
,	m_OrthoMode( false )
,	m_Bounds( SRect( 0, 0, 640, -480 ) )
{
	UpdateInvRange();
}

View::View( const Vector& Location, const Vector& Direction, const SRect& Bounds, float NearClip, float FarClip )
:	m_Location( Location )
,	m_Rotation( Direction.ToAngles() )
,	m_NearClip( NearClip )
,	m_FarClip( FarClip )
,	m_InvRange( 0.0f )
,	m_VerticalFOV( 90.f )
,	m_AspectRatio( kAspect_4_3 )
,	m_OrthoMode( true )
,	m_Bounds( Bounds )
{
	UpdateInvRange();
}

View::View( const Vector& Location, const Angles& Rotation, const SRect& Bounds, float NearClip, float FarClip )
:	m_Location( Location )
,	m_Rotation( Rotation )
,	m_NearClip( NearClip )
,	m_FarClip( FarClip )
,	m_InvRange( 0.0f )
,	m_VerticalFOV( 90.f )
,	m_AspectRatio( kAspect_4_3 )
,	m_OrthoMode( true )
,	m_Bounds( Bounds )
{
	UpdateInvRange();
}

void View::ApplyToRenderer( IRenderer& Renderer ) const
{
	XTRACE_FUNCTION;

	Renderer.SetViewMatrix( GetViewMatrix() );
	Renderer.SetProjectionMatrix( GetProjectionMatrix() );
}

void View::ApplyToFrustum( Frustum& f ) const
{
	f.InitWith( GetViewMatrix() * GetProjectionMatrix() );
}

Matrix View::GetViewMatrix() const
{
	Vector X, Y, Z;
	m_Rotation.GetAxes( X, Y, Z );
	return Matrix::CreateViewMatrixCoords( m_Location, X, Y, Z );
}

Matrix View::GetProjectionMatrix() const
{
	if( m_OrthoMode )
	{
		return Matrix::CreateOrthoProjectionMatrix( m_Bounds.m_Left, m_Bounds.m_Top, m_Bounds.m_Right, m_Bounds.m_Bottom, m_NearClip, m_InvRange );
	}
	else
	{
		return Matrix::CreateProjectionMatrix( m_VerticalFOV, m_NearClip, m_FarClip, m_InvRange, m_AspectRatio );
	}
}

Vector4 View::Project( const Vector& Location ) const
{
	const Matrix	ViewProjectionMatrix	= GetViewMatrix() * GetProjectionMatrix();
	const Vector4	ProjectedLocation		= Vector4( Location ) * ViewProjectionMatrix;

	return ProjectedLocation;
}

Vector2 View::ProjectAndClipToScreen( const Vector& Location ) const
{
	const Vector4			ProjectedLocation4D	= Project( Location );
	Vector2					ProjectedLocation2D;

	if( Abs( ProjectedLocation4D.w ) < EPSILON )
	{
		ProjectedLocation2D = ProjectedLocation4D;
	}
	else
	{
		ProjectedLocation2D = ProjectedLocation4D / ProjectedLocation4D.w;

		if( ProjectedLocation4D.w < 0.0f )
		{
			// Project beyond [-1,1] if location is behind view.
			ProjectedLocation2D = ProjectedLocation2D.GetNormalized() * -2.0f;
		}
	}

	static const Vector2	kScreenScale		= Vector2( 0.5f, -0.5f );
	static const Vector2	kScreenOffset		= Vector2( 0.5f, 0.5f );
	static const Vector2	kScreenMin			= Vector2( 0.0f, 0.0f );
	static const Vector2	kScreenMid			= Vector2( 0.5f, 0.5f );
	static const Vector2	kScreenMax			= Vector2( 1.0f, 1.0f );
	static const Box2D		kScreenBox			= Box2D( kScreenMin, kScreenMax );
	Vector2					ScreenLocation		= ( ProjectedLocation2D * kScreenScale ) + kScreenOffset;
	const Segment2D			ScreenSegment		= Segment2D( ScreenLocation, kScreenMid );

	CollisionInfo2D			Info;
	if( ScreenSegment.Intersects( kScreenBox, &Info ) )
	{
		ScreenLocation = Info.m_Intersection;
	}

	ScreenLocation.x = Clamp( ScreenLocation.x, 0.0f, 1.0f );
	ScreenLocation.y = Clamp( ScreenLocation.y, 0.0f, 1.0f );

	return ScreenLocation;
}

void View::SetOrthoBounds( const SRect& Bounds )
{
	ASSERT( m_OrthoMode );
	m_Bounds = Bounds;
}

void View::SetClipPlanes( float NearClip, float FarClip )
{
	m_NearClip = NearClip;
	m_FarClip = FarClip;
	UpdateInvRange();
}

void View::UpdateInvRange()
{
	m_InvRange = 1.0f / ( m_FarClip - m_NearClip );
}