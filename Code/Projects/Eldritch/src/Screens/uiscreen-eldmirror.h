#ifndef UISCREENELDMIRROR_H
#define UISCREENELDMIRROR_H

#include "uiscreen.h"
#include "iwbeventobserver.h"
#include "vector4.h"
#include "eldritchirradiance.h"

class Mesh;

class UIScreenEldMirror : public UIScreen, public IWBEventObserver
{
public:
	UIScreenEldMirror();
	virtual ~UIScreenEldMirror();

	DEFINE_UISCREEN_FACTORY( EldMirror );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual ETickReturn	Tick( float DeltaTime, bool HasFocus );
	virtual void		Render( bool HasFocus );
	virtual void		Pushed();

	uint				GetMirrorRTWidth() const		{ return m_MirrorRTWidth; }
	uint				GetMirrorRTHeight() const		{ return m_MirrorRTHeight; }
	float				GetMirrorYaw() const			{ return m_MirrorYaw; }
	float				GetMirrorViewFOV() const		{ return m_MirrorViewFOV; }
	float				GetMirrorViewDistance() const	{ return m_MirrorViewDistance; }
	float				GetMirrorViewHeight() const		{ return m_MirrorViewHeight; }
	float				GetMirrorViewNearClip() const	{ return m_MirrorViewNearClip; }
	float				GetMirrorViewFarClip() const	{ return m_MirrorViewFarClip; }

	void				OnMirrorRTUpdated();

	// IWBEventObserver
	virtual void		HandleEvent( const WBEvent& Event );

protected:
	void				SetRigMesh( const SimpleString& MeshName );
	void				SetHeadMesh( const SimpleString& MeshName, const SimpleString& TextureName );
	void				SetBodyMesh( const SimpleString& MeshName, const SimpleString& TextureName );

	Mesh*				CreateRigMesh( const SimpleString& MeshName );
	Mesh*				CreateMesh( const SimpleString& MeshName, const SimpleString& TextureName );
	void				CreateBackdropMesh();

	void				RenderMesh( Mesh* const pMesh );
	void				PlayAnimation();

	Mesh*			m_RigMesh;
	Mesh*			m_HeadMesh;
	Mesh*			m_BodyMesh;
	Mesh*			m_BackdropMesh;

	uint			m_MirrorRTWidth;
	uint			m_MirrorRTHeight;

	HashedString	m_MirrorAnimation;

	float			m_MirrorYaw;

	float			m_MirrorViewFOV;
	float			m_MirrorViewDistance;
	float			m_MirrorViewHeight;
	float			m_MirrorViewNearClip;
	float			m_MirrorViewFarClip;

	uint			m_MirrorBackdropTile;
	Vector4			m_MirrorBackdropColor;
	float			m_MirrorBackdropDistance;
	float			m_MirrorBackdropExtents;

	SVoxelIrradiance	m_MirrorIrradiance;
};

#endif // UISCREENELDMIRROR_H