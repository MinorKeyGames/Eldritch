#ifndef VIEW_H
#define VIEW_H

#include "matrix.h"
#include "vector.h"
#include "angles.h"
#include "3d.h"

class IRenderer;
class Frustum;
class Vector4;

// A view describes the view and projection matrices for
// a 3D scene--could be called a camera, but that name
// is better reserved for a gameplay device.

class View
{
public:
	View();

	// Perspective mode constructors
	View( const Vector& Location, const Vector& Direction, float VerticalFOV, float AspectRatio, float NearClip, float FarClip );
	View( const Vector& Location, const Angles& Rotation, float VerticalFOV, float AspectRatio, float NearClip, float FarClip );

	// Ortho mode constructors
	View( const Vector& Location, const Vector& Direction, const SRect& Bounds, float NearClip, float FarClip );
	View( const Vector& Location, const Angles& Rotation, const SRect& Bounds, float NearClip, float FarClip );

	float	GetFOV() const { return m_VerticalFOV; }
	void	SetFOV( const float FOV ) { m_VerticalFOV = FOV; }

	float	GetAspectRatio() const { return m_AspectRatio; }
	void	SetAspectRatio( const float AspectRatio ) { m_AspectRatio = AspectRatio; }

	void	SetOrthoBounds( const SRect& Bounds );

	void	SetClipPlanes( float NearClip, float FarClip );

	void	ApplyToRenderer( IRenderer& Renderer ) const;
	void	ApplyToFrustum( Frustum& f ) const;

	Matrix	GetViewMatrix() const;
	Matrix	GetProjectionMatrix() const;
	Vector4	Project( const Vector& Location ) const;
	Vector2	ProjectAndClipToScreen( const Vector& Location ) const;	// Returns X,Y in range [0,1]

	// I'm letting all of these be public because they'd all
	// have set* functions anyway--more user-friendly controls
	// will be put in a game-side camera class that uses the
	// View to communicate with the Renderer. It's a bit
	// abstracted and maybe redundant, but should be fine.
	Vector	m_Location;
	Angles	m_Rotation;
	float	m_NearClip;
	float	m_FarClip;
	float	m_InvRange;	// 1/(far-near)

	// Perspective mode
	float	m_VerticalFOV;
	float	m_AspectRatio;

	// Ortho mode
	bool	m_OrthoMode;
	SRect	m_Bounds;

private:
	void	UpdateInvRange();
};

#endif // VIEW_H