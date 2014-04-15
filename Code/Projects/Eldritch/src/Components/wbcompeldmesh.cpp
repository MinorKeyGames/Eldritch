#include "core.h"
#include "wbcompeldmesh.h"
#include "configmanager.h"
#include "eldritchmesh.h"
#include "eldritchframework.h"
#include "irenderer.h"
#include "meshfactory.h"
#include "wbcompeldtransform.h"
#include "wbentity.h"
#include "eldritchworld.h"
#include "shadermanager.h"
#include "idatastream.h"
#include "texturemanager.h"
#include "mathcore.h"
#include "wbeventmanager.h"
#include "animation.h"
#include "eldritchgame.h"
#include "wbcompelddropshadow.h"
#include "wbcompeldfrobbable.h"
#include "plane.h"

WBCompEldMesh::WBCompEldMesh()
:	m_Mesh( NULL )
,	m_MeshOriginalAABB()
,	m_Hidden( false )
,	m_Offset()
,	m_IrradianceOffsetZ( 0.0f )
,	m_SendUpdatedEvent( false )
,	m_DependentDropShadow( NULL )
,	m_DependentFrobbable( NULL )
,	m_ForceUpdateTransform( false )
,	m_OldTransform_Location()
,	m_OldTransform_Rotation()
,	m_OldTransform_Scale()
,	m_MeshName()
,	m_TextureName()
,	m_TextureOverride()
,	m_MaterialOverride()
,	m_DrawForeground( false )
,	m_UseTwoPointIrradiance( false )
,	m_TwoPointIrradianceOffset()
,	m_UseBlendedIrradiance( false )
,	m_BlendRate( 0.0f )
,	m_BlendedIrradiance()
,	m_CurrentHighlight()
,	m_ConstantIrradiance()
,	m_CullDistanceSq( 0.0f )
{
}

WBCompEldMesh::~WBCompEldMesh()
{
	SafeDelete( m_Mesh );
}

void WBCompEldMesh::SetMesh( const SimpleString& Mesh )
{
	if( Mesh == "" )
	{
		return;
	}

	m_MeshName = Mesh;

	SafeDelete( m_Mesh );

	IRenderer* const pRenderer = GetFramework()->GetRenderer();

	m_Mesh = new EldritchMesh();
	m_Mesh->SetEntity( GetEntity() );

	pRenderer->GetMeshFactory()->GetDynamicMesh( Mesh.CStr(), m_Mesh );

	// Null out the default normal and spec textures; shader code will populate with the current fog lookup texture, etc.
	m_Mesh->SetTexture( 1, NULL );
	m_Mesh->SetTexture( 2, NULL );

	m_MeshOriginalAABB = m_Mesh->m_AABB;

	if( m_Mesh->IsAnimated() )
	{
		SAnimationListener AnimationListener;
		AnimationListener.m_NotifyFinishedFunc = NotifyAnimationFinished;
		AnimationListener.m_Void = this;
		m_Mesh->AddAnimationListener( AnimationListener );
	}

	// TODO: Rename this to avoid confusion with new material system
	if( m_TextureOverride != "" )
	{
		m_Mesh->SetTexture( 0, pRenderer->GetTextureManager()->GetTexture( m_TextureOverride.CStr() ) );
	}

	if( m_MaterialOverride != "" )
	{
		m_Mesh->SetMaterialDefinition( m_MaterialOverride, pRenderer );
	}
	else if( m_Mesh->IsAnimated() )
	{
		m_Mesh->SetMaterialDefinition( "Material_EntityAnimated", pRenderer );
	}
	else
	{
		m_Mesh->SetMaterialDefinition( "Material_EntityStatic", pRenderer );
	}

	m_Mesh->SetMaterialFlags( ( m_DrawForeground ? MAT_FOREGROUND : MAT_WORLD ) | MAT_DYNAMIC );

	m_ForceUpdateTransform = true;
}

void WBCompEldMesh::SetTexture( const SimpleString& Texture )
{
	if( Texture == "" )
	{
		return;
	}

	m_TextureName = Texture;

	IRenderer* const pRenderer = GetFramework()->GetRenderer();

	if( Texture != "" )
	{
		m_Mesh->SetTexture( 0, pRenderer->GetTextureManager()->GetTexture( Texture.CStr() ) );
	}
}

/*virtual*/ void WBCompEldMesh::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( EldMesh );

	STATICHASH( TextureOverride );
	m_TextureOverride = ConfigManager::GetInheritedString( sTextureOverride, "", sDefinitionName );

	STATICHASH( MaterialOverride );
	m_MaterialOverride = ConfigManager::GetInheritedString( sMaterialOverride, "", sDefinitionName );

	STATICHASH( DrawForeground );
	m_DrawForeground = ConfigManager::GetInheritedBool( sDrawForeground, false, sDefinitionName );

	STATICHASH( Mesh );
	const SimpleString Mesh = ConfigManager::GetInheritedString( sMesh, "", sDefinitionName );

	SetMesh( Mesh );

	STATICHASH( OffsetZ );
	m_Offset.z = ConfigManager::GetInheritedFloat( sOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( IrradianceOffsetZ );
	m_IrradianceOffsetZ = ConfigManager::GetInheritedFloat( sIrradianceOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( UseTwoPointIrradiance );
	m_UseTwoPointIrradiance = ConfigManager::GetInheritedBool( sUseTwoPointIrradiance, false, sDefinitionName );

	STATICHASH( UseBlendedIrradiance );
	const bool DefaultUseBlendedIrradiance = ConfigManager::GetBool( sUseBlendedIrradiance, false, sEldMesh );
	m_UseBlendedIrradiance = ConfigManager::GetInheritedBool( sUseBlendedIrradiance, DefaultUseBlendedIrradiance, sDefinitionName );

	STATICHASH( BlendRate );
	const float DefaultBlendRate = ConfigManager::GetFloat( sBlendRate, 0.0f, sEldMesh );
	m_BlendRate = ConfigManager::GetInheritedFloat( sBlendRate, DefaultBlendRate, sDefinitionName );

	STATICHASH( ConstantIrradianceR );
	m_ConstantIrradiance.r = ConfigManager::GetInheritedFloat( sConstantIrradianceR, 0.0f, sDefinitionName );

	STATICHASH( ConstantIrradianceG );
	m_ConstantIrradiance.g = ConfigManager::GetInheritedFloat( sConstantIrradianceG, 0.0f, sDefinitionName );

	STATICHASH( ConstantIrradianceB );
	m_ConstantIrradiance.b = ConfigManager::GetInheritedFloat( sConstantIrradianceB, 0.0f, sDefinitionName );

	STATICHASH( CullDistance );
	const float DefaultCullDistance = ConfigManager::GetFloat( sCullDistance, 0.0f, sEldMesh );
	m_CullDistanceSq = Square( ConfigManager::GetInheritedFloat( sCullDistance, DefaultCullDistance, sDefinitionName ) );
}

bool WBCompEldMesh::UpdateMeshTransform()
{
	ASSERT( m_Mesh );

	WBCompEldTransform* pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );	// Makes no sense to have a mesh and no transform

	const Vector EntityLocation		= pTransform->GetLocation();
	const Vector CurrentLocation	= EntityLocation + m_Offset;
	m_Mesh->m_Location = CurrentLocation;

	const Angles EntityOrientation	= pTransform->GetOrientation();
	m_Mesh->m_Rotation = EntityOrientation;

	// Optimization, avoid the matrix and AABB calcs if nothing has changed
	if(	m_OldTransform_Location	!= m_Mesh->m_Location	||
		m_OldTransform_Rotation	!= m_Mesh->m_Rotation	||
		m_OldTransform_Scale	!= m_Mesh->m_Scale		||
		m_ForceUpdateTransform )
	{
		m_OldTransform_Location	= m_Mesh->m_Location;
		m_OldTransform_Rotation	= m_Mesh->m_Rotation;
		m_OldTransform_Scale	= m_Mesh->m_Scale;
		m_ForceUpdateTransform	= false;

		// Seems like maybe this AABB stuff should be done routinely in the Mesh. :/
		const Matrix ScaleMatrix		= Matrix::CreateScale( m_Mesh->m_Scale );
		const Matrix RotationMatrix		= EntityOrientation.ToMatrix();
		const Matrix TranslationMatrix	= Matrix::CreateTranslation( CurrentLocation );
		const Matrix AABBTransform		= ScaleMatrix * RotationMatrix * TranslationMatrix;
		m_Mesh->m_AABB					= m_MeshOriginalAABB.GetTransformedBound( AABBTransform );

		return true;
	}
	else
	{
		return false;
	}
}

void WBCompEldMesh::UpdateMesh( const float DeltaTime )
{
	XTRACE_FUNCTION;

	ASSERT( m_Mesh );

	const bool TransformUpdated = UpdateMeshTransform();

	UpdateIrradiance( DeltaTime );

	if( m_Mesh->IsAnimated() )
	{
		m_Mesh->Tick( DeltaTime );
	}

	// HACK: Optimization: only send event if transform was updated.
	if( TransformUpdated && m_SendUpdatedEvent )
	{
		WB_MAKE_EVENT( OnMeshUpdated, NULL );	// HACK: Optimization: don't provide entity for context, clients of OnMeshUpdated don't need it

		// HACK: Optimization: bypass event manager and send event directly to dependent components
		if( m_DependentDropShadow )
		{
			m_DependentDropShadow->HandleEvent( WB_AUTO_EVENT( OnMeshUpdated ) );
		}

		if( m_DependentFrobbable )
		{
			m_DependentFrobbable->HandleEvent( WB_AUTO_EVENT( OnMeshUpdated ) );
		}
	}
}

/*virtual*/ void WBCompEldMesh::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	UpdateMesh( DeltaTime );

	// Add pseudo root motion. Hack from Couriers.
	if( m_Mesh && m_Mesh->IsAnimated() )
	{
		Vector AnimationVelocity;
		Angles AnimationRotationalVelocity;
		GetAnimationVelocity( AnimationVelocity, AnimationRotationalVelocity );

		if( AnimationVelocity.LengthSquared() > 0.0f || !AnimationRotationalVelocity.IsZero() )
		{
			WBCompEldTransform* pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
			DEVASSERT( pTransform );

			// Kill velocity in direction of movement.
			if( AnimationVelocity.LengthSquared() > 0.0f )
			{
				Plane MovementPlane( AnimationVelocity.GetNormalized(), 0.0f );
				pTransform->SetVelocity( MovementPlane.ProjectVector( pTransform->GetVelocity() ) );
			}

			pTransform->ApplyImpulse( AnimationVelocity );
			pTransform->ApplyRotationalImpulse( AnimationRotationalVelocity );
		}
	}
}

void WBCompEldMesh::UpdateIrradiance( const float DeltaTime )
{
	ASSERT( m_Mesh );

	// If needed for optimization, only do this on certain events (loaded, moved, etc.)

	WBCompEldTransform* pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );	// Makes no sense to have a mesh and no transform

	const Vector EntityLocation = pTransform->GetLocation();

	EldritchWorld* const	pWorld				= GetWorld();
	const Vector			IrradianceOffset	= Vector( 0.0f, 0.0f, m_IrradianceOffsetZ );

	SVoxelIrradiance CurrentIrradiance;
	if( m_UseTwoPointIrradiance )
	{
		const Vector LocationA = EntityLocation + IrradianceOffset;
		const Vector LocationB = LocationA + m_TwoPointIrradianceOffset;
		CurrentIrradiance = pWorld->BlendIrradiances( LocationA, LocationB );
	}
	else
	{
		CurrentIrradiance = pWorld->GetIrradianceAt( EntityLocation + IrradianceOffset );
	}

	for( uint IrrDir = 0; IrrDir < 6; ++IrrDir )
	{
		Vector4& DirLight = CurrentIrradiance.m_Light[ IrrDir ];
		DirLight += m_CurrentHighlight;
		DirLight += m_ConstantIrradiance;
	}

	if( m_UseBlendedIrradiance )
	{
		const float BlendTime = Saturate( DeltaTime * m_BlendRate );
		m_BlendedIrradiance = SVoxelIrradiance::Lerp( m_BlendedIrradiance, CurrentIrradiance, BlendTime );
		m_Mesh->SetIrradianceCube( m_BlendedIrradiance );
	}
	else
	{
		m_Mesh->SetIrradianceCube( CurrentIrradiance );
	}
}

void WBCompEldMesh::ImmediateUpdateBlendedIrradiance()
{
	if( m_UseBlendedIrradiance )
	{
		UpdateIrradiance( FLT_MAX );
	}
}

void WBCompEldMesh::SetSendUpdatedEvent()
{
	// HACK: For optimization, only enable this event on entities that need it.
	WBEntity* const				pEntity		= GetEntity();
	WBCompEldDropShadow* const	pDropShadow	= GET_WBCOMP( pEntity, EldDropShadow );
	WBCompEldFrobbable* const	pFrobbable	= GET_WBCOMP( pEntity, EldFrobbable );

	if( pDropShadow && pDropShadow->GetUseMeshCenter() )
	{
		m_DependentDropShadow = pDropShadow;
	}

	if( pFrobbable && pFrobbable->GetUseMeshExtents() )
	{
		m_DependentFrobbable = pFrobbable;
	}

	if( m_DependentDropShadow || m_DependentFrobbable )
	{
		m_SendUpdatedEvent = true;
	}
}

/*virtual*/ void WBCompEldMesh::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnInitialized );
	STATIC_HASHED_STRING( OnSpawnedQueued );
	STATIC_HASHED_STRING( OnLoaded );
	STATIC_HASHED_STRING( OnBecameFrobTarget );
	STATIC_HASHED_STRING( Hide );
	STATIC_HASHED_STRING( Show );
	STATIC_HASHED_STRING( HideMesh );
	STATIC_HASHED_STRING( ShowMesh );
	STATIC_HASHED_STRING( PlayAnim );
	STATIC_HASHED_STRING( SetAnim );
	STATIC_HASHED_STRING( CopyAnimations );
	STATIC_HASHED_STRING( SetMesh );
	STATIC_HASHED_STRING( SetTexture );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnInitialized )
	{
		SetSendUpdatedEvent();
	}
	else if( EventName == sOnSpawnedQueued || EventName == sOnLoaded )	// Need to have a valid transform
	{
		ImmediateUpdateBlendedIrradiance();
	}
	else if( EventName == sOnBecameFrobTarget )
	{
		STATIC_HASHED_STRING( IsFrobTarget );
		const bool IsFrobTarget = Event.GetBool( sIsFrobTarget );

		if( IsFrobTarget )
		{
			STATIC_HASHED_STRING( Highlight );
			m_CurrentHighlight = Event.GetVector( sHighlight );
		}
		else
		{
			m_CurrentHighlight.Zero();
		}
	}
	else if( EventName == sHide || EventName == sHideMesh )
	{
		m_Hidden = true;
	}
	else if( EventName == sShow || EventName == sShowMesh )
	{
		m_Hidden = false;
	}
	else if( EventName == sPlayAnim )
	{
		STATIC_HASHED_STRING( AnimationName );
		const HashedString AnimationName = Event.GetHash( sAnimationName );

		STATIC_HASHED_STRING( Loop );
		const bool Loop = Event.GetBool( sLoop );

		STATIC_HASHED_STRING( IgnoreIfAlreadyPlaying );
		const bool IgnoreIfAlreadyPlaying = Event.GetBool( sIgnoreIfAlreadyPlaying );

		STATIC_HASHED_STRING( PlayRate );
		const float PlayRate = Event.GetFloat( sPlayRate );

		PlayAnimation( AnimationName, Loop, IgnoreIfAlreadyPlaying, PlayRate );
	}
	else if( EventName == sSetAnim )
	{
		STATIC_HASHED_STRING( AnimationIndex );
		const int AnimationIndex = Event.GetInt( sAnimationIndex );

		STATIC_HASHED_STRING( AnimationTime );
		const float AnimationTime = Event.GetFloat( sAnimationTime );

		STATIC_HASHED_STRING( AnimationEndBehavior );
		const int AnimationEndBehavior = Event.GetInt( sAnimationEndBehavior );

		STATIC_HASHED_STRING( AnimationPlayRate );
		const float AnimationPlayRate = Event.GetFloat( sAnimationPlayRate );

		AnimationState::SPlayAnimationParams PlayParams;
		PlayParams.m_EndBehavior = static_cast<AnimationState::EAnimationEndBehavior>( AnimationEndBehavior );

		m_Mesh->SetAnimation( AnimationIndex, PlayParams );
		m_Mesh->SetAnimationTime( AnimationTime );
		m_Mesh->SetAnimationPlayRate( AnimationPlayRate > 0.0f ? AnimationPlayRate : 1.0f );
	}
	else if( EventName == sCopyAnimations )
	{
		STATIC_HASHED_STRING( SourceEntity );
		WBEntity* const pSourceEntity = Event.GetEntity( sSourceEntity );

		STATIC_HASHED_STRING( SuppressAnimEvents );
		const bool SuppressAnimEvents = Event.GetBool( sSuppressAnimEvents );

		CopyAnimationsFrom( pSourceEntity, SuppressAnimEvents );
	}
	else if( EventName == sSetMesh )
	{
		STATIC_HASHED_STRING( Mesh );
		const SimpleString Mesh = Event.GetString( sMesh );

		STATIC_HASHED_STRING( Texture );
		const SimpleString Texture = Event.GetString( sTexture );

		SetMesh( Mesh );
		SetTexture( Texture );
		UpdateMesh( 0.0f );
	}
	else if( EventName == sSetTexture )
	{
		STATIC_HASHED_STRING( Texture );
		const SimpleString Texture = Event.GetString( sTexture );

		SetTexture( Texture );
	}
}

/*virtual*/ void WBCompEldMesh::Render()
{
	XTRACE_FUNCTION;

	ASSERT( m_Mesh );

	if( m_Hidden )
	{
		return;
	}

	if( m_CullDistanceSq > 0.0f )
	{
		const Vector	ViewLocation	= EldritchGame::GetPlayerViewLocation();
		const Vector	ViewOffset		= m_Mesh->m_Location - ViewLocation;
		const float		DistanceSq		= ViewOffset.LengthSquared();

		if( DistanceSq > m_CullDistanceSq )
		{
			return;
		}
	}

	if( m_Mesh->IsAnimated() )
	{
		m_Mesh->UpdateBones();
	}

	if( !m_Mesh->GetTexture( 1 ) )
	{
		ITexture* const pFogTexture = GetGame()->GetFogTexture();
		m_Mesh->SetTexture( 1, pFogTexture );
	}

	GetFramework()->GetRenderer()->AddMesh( m_Mesh );
}

#if BUILD_DEV
/*virtual*/ void WBCompEldMesh::DebugRender() const
{
	WBCompEldTransform* pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	IRenderer* const pRenderer = GetFramework()->GetRenderer();
	const Vector EntityLocation = pTransform->GetLocation();

	pRenderer->DEBUGDrawCross( EntityLocation + m_Offset, 0.25f, ARGB_TO_COLOR( 255, 255, 255, 0 ) );

	const Vector IrradianceOffset	= Vector( 0.0f, 0.0f, m_IrradianceOffsetZ );
	const Vector IrradianceLocation	= EntityLocation + IrradianceOffset;
	pRenderer->DEBUGDrawCross( IrradianceLocation, 0.25f, ARGB_TO_COLOR( 255, 0, 255, 255 ) );

	if( m_UseTwoPointIrradiance )
	{
		pRenderer->DEBUGDrawCross( IrradianceLocation + m_TwoPointIrradianceOffset, 0.25f, ARGB_TO_COLOR( 255, 0, 255, 255 ) );
	}

	pRenderer->DEBUGDrawBox( m_Mesh->m_AABB.m_Min, m_Mesh->m_AABB.m_Max, ARGB_TO_COLOR( 255, 255, 0, 255 ) );
}
#endif

void WBCompEldMesh::SetMeshScale( const Vector& Scale )
{
	ASSERT( m_Mesh );
	m_Mesh->m_Scale = Scale;
}

void WBCompEldMesh::PlayAnimation( const HashedString& AnimationName, const bool Loop, const bool IgnoreIfAlreadyPlaying, const float PlayRate ) const
{
	ASSERT( m_Mesh );

	AnimationState::SPlayAnimationParams PlayParams;
	PlayParams.m_EndBehavior			= Loop ? AnimationState::EAEB_Loop : AnimationState::EAEB_Stop;
	PlayParams.m_IgnoreIfAlreadyPlaying	= IgnoreIfAlreadyPlaying;
	PlayParams.m_PlayRate				= ( PlayRate > 0.0f ) ? PlayRate : 1.0f;

	m_Mesh->PlayAnimation( AnimationName, PlayParams );
}

void WBCompEldMesh::GetAnimationVelocity( Vector& OutVelocity, Angles& OutRotationalVelocity )
{
	ASSERT( m_Mesh );

	WBCompEldTransform* pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	m_Mesh->GetAnimationVelocity( OutVelocity, OutRotationalVelocity );
	OutVelocity = OutVelocity.RotateBy( pTransform->GetOrientation() );
}

void WBCompEldMesh::CopyAnimationsFrom( WBEntity* const pSourceEntity, const bool SuppressAnimEvents ) const
{
	ASSERT( pSourceEntity );
	ASSERT( m_Mesh );

	WBCompEldMesh* const pOtherMesh = GET_WBCOMP( pSourceEntity, EldMesh );
	ASSERT( pOtherMesh );

	m_Mesh->CopyAnimationsFrom( pOtherMesh->GetMesh() );
	m_Mesh->SuppressAnimEvents( SuppressAnimEvents );
}

/*static*/ void WBCompEldMesh::NotifyAnimationFinished( void* pVoid, class Mesh* pMesh, class Animation* pAnimation, bool Interrupted )
{
	WBCompEldMesh* const pThis = static_cast<WBCompEldMesh*>( pVoid );
	ASSERT( pThis );
	pThis->OnAnimationFinished( pMesh, pAnimation, Interrupted );
}

void WBCompEldMesh::OnAnimationFinished( class Mesh* pMesh, class Animation* pAnimation, bool Interrupted )
{
	Unused( pMesh );

	ASSERT( pAnimation );
	ASSERT( pMesh == m_Mesh );

	WB_MAKE_EVENT( OnAnimationFinished, GetEntity() );
	WB_SET_AUTO( OnAnimationFinished, Hash, AnimationName, pAnimation->m_HashedName );
	WB_SET_AUTO( OnAnimationFinished, Bool, Interrupted, Interrupted );
	WB_DISPATCH_EVENT( GetEventManager(), OnAnimationFinished, GetEntity() );
}

#define VERSION_EMPTY		0
#define VERSION_MESHSCALE	1
#define VERSION_HIDDEN		2
#define VERSION_ANIMATION	3
#define VERSION_MESHNAME	4
#define VERSION_TEXTURENAME	5
#define VERSION_ANIMRATE	6
#define VERSION_CURRENT		6

uint WBCompEldMesh::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;													// Version
	Size += IDataStream::SizeForWriteString( m_MeshName );		// m_MeshName
	Size += IDataStream::SizeForWriteString( m_TextureName );	// m_TextureName
	Size += sizeof( Vector );									// m_Mesh->m_Scale
	Size += 1;													// m_Hidden
	Size += 16;													// Animation state

	return Size;
}

void WBCompEldMesh::Save( const IDataStream& Stream )
{
	ASSERT( m_Mesh );

	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteString( m_MeshName );
	Stream.WriteString( m_TextureName );

	Stream.Write( sizeof( Vector ), &m_Mesh->m_Scale );

	Stream.WriteBool( m_Hidden );

	Stream.WriteInt32( m_Mesh->GetAnimationIndex() );
	Stream.WriteFloat( m_Mesh->GetAnimationTime() );
	Stream.WriteInt32( m_Mesh->GetAnimationEndBehavior() );
	Stream.WriteFloat( m_Mesh->GetAnimationPlayRate() );
}

void WBCompEldMesh::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	ASSERT( m_Mesh );

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_MESHNAME )
	{
		const SimpleString MeshName = Stream.ReadString();
		if( MeshName != m_MeshName )
		{
			SetMesh( MeshName );
		}
	}

	if( Version >= VERSION_TEXTURENAME )
	{
		const SimpleString TextureName = Stream.ReadString();
		if( TextureName != m_TextureName )
		{
			SetTexture( TextureName );
		}
	}

	if( Version >= VERSION_MESHSCALE )
	{
		Stream.Read( sizeof( Vector ), &m_Mesh->m_Scale );
	}

	if( Version >= VERSION_HIDDEN )
	{
		m_Hidden = Stream.ReadBool();
	}

	if( Version >= VERSION_ANIMATION )
	{
		const int	AnimationIndex			= Stream.ReadInt32();
		const float	AnimationTime			= Stream.ReadFloat();
		const int	AnimationEndBehavior	= Stream.ReadInt32();
		const float	AnimationPlayRate		= ( Version >= VERSION_ANIMRATE ) ? Stream.ReadFloat() : 1.0f;

		if( m_Mesh->IsAnimated() )
		{
			WB_MAKE_EVENT( SetAnim, GetEntity() );
			WB_SET_AUTO( SetAnim, Int, AnimationIndex, AnimationIndex );
			WB_SET_AUTO( SetAnim, Float, AnimationTime, AnimationTime );
			WB_SET_AUTO( SetAnim, Int, AnimationEndBehavior, AnimationEndBehavior );
			WB_SET_AUTO( SetAnim, Float, AnimationPlayRate, AnimationPlayRate );
			WB_QUEUE_EVENT( GetEventManager(), SetAnim, GetEntity() );
		}
	}
}