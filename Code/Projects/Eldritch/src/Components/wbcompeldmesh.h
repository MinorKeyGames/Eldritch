#ifndef WBCOMPELDMESH_H
#define WBCOMPELDMESH_H

#include "wbeldritchcomponent.h"
#include "vector.h"
#include "angles.h"
#include "eldritchirradiance.h"
#include "aabb.h"
#include "simplestring.h"

class EldritchMesh;

class WBCompEldMesh : public WBEldritchComponent
{
public:
	WBCompEldMesh();
	virtual ~WBCompEldMesh();

	DEFINE_WBCOMP( EldMesh, WBEldritchComponent );

	virtual void	Tick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickDefault; }	// Should tick after transform

	virtual void	HandleEvent( const WBEvent& Event );

	virtual bool	IsRenderable() { return true; }
	virtual void	Render();

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

	EldritchMesh*	GetMesh() const { return m_Mesh; }
	void			SetMeshOffset( const Vector& Offset ) { m_Offset = Offset; }
	const Vector&	GetMeshOffset() const { return m_Offset; }
	void			SetMeshScale( const Vector& Scale );
	void			SetBlendedIrradianceOffset( const Vector& Offset ) { m_TwoPointIrradianceOffset = Offset; }

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	static void		NotifyAnimationFinished( void* pVoid, class Mesh* pMesh, class Animation* pAnimation, bool Interrupted );
	void			OnAnimationFinished( class Mesh* pMesh, class Animation* pAnimation, bool Interrupted );

	bool			IsHidden() const { return m_Hidden; }

	// For pseudo root motion hack from Couriers
	void			GetAnimationVelocity( Vector& OutVelocity, Angles& OutRotationalVelocity );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			SetSendUpdatedEvent();

	void			SetMesh( const SimpleString& Mesh );
	void			SetTexture( const SimpleString& Texture );

	void			UpdateMesh( const float DeltaTime );
	bool			UpdateMeshTransform();
	void			UpdateIrradiance( const float DeltaTime );
	void			ImmediateUpdateBlendedIrradiance();

	void			PlayAnimation( const HashedString& AnimationName, const bool Loop, const bool IgnoreIfAlreadyPlaying, const float PlayRate ) const;
	void			CopyAnimationsFrom( WBEntity* const pSourceEntity, const bool SuppressAnimEvents ) const;

	EldritchMesh*		m_Mesh;
	AABB				m_MeshOriginalAABB;
	bool				m_Hidden;
	Vector				m_Offset;
	float				m_IrradianceOffsetZ;

	bool				m_SendUpdatedEvent;			// Transient, optimization
	// Optimization, cache components which are dependent on the updated event
	WBComponent*		m_DependentDropShadow;		// Transient
	WBComponent*		m_DependentFrobbable;		// Transient

	// Optimization, avoid recalculating matrices for AABB unless needed
	bool				m_ForceUpdateTransform;
	Vector				m_OldTransform_Location;
	Angles				m_OldTransform_Rotation;
	Vector				m_OldTransform_Scale;

	SimpleString		m_MeshName;
	SimpleString		m_TextureName;				// Serialized
	SimpleString		m_TextureOverride;			// Config
	SimpleString		m_MaterialOverride;
	bool				m_DrawForeground;

	bool				m_UseTwoPointIrradiance;	// Config: Uses a secondary offset and blends the two irradiances
	Vector				m_TwoPointIrradianceOffset;	// Transient

	bool				m_UseBlendedIrradiance;		// Config: Blends irradiance over time
	float				m_BlendRate;				// Config
	SVoxelIrradiance	m_BlendedIrradiance;		// Transient

	Vector4				m_CurrentHighlight;			// This couples meshes and frobbables more than I'd like. :(
	Vector4				m_ConstantIrradiance;

	float				m_CullDistanceSq;			// Config: If non-zero, mesh is only drawn within this distance from camera
};

#endif // WBCOMPELDMESH_H