#include "core.h"
#include "uiscreen-eldmirror.h"
#include "configmanager.h"
#include "Widgets/uiwidget-image.h"
#include "eldritchframework.h"
#include "eldritchtargetmanager.h"
#include "irendertarget.h"
#include "wbevent.h"
#include "eldritchmesh.h"
#include "irenderer.h"
#include "meshfactory.h"
#include "shadermanager.h"
#include "eldritchgame.h"
#include "eldritchirradiance.h"
#include "eldritchworld.h"
#include "texturemanager.h"
#include "ivertexbuffer.h"
#include "iindexbuffer.h"
#include "ivertexdeclaration.h"
#include "mathcore.h"

UIScreenEldMirror::UIScreenEldMirror()
:	m_RigMesh( NULL )
,	m_HeadMesh( NULL )
,	m_BodyMesh( NULL )
,	m_BackdropMesh( NULL )
,	m_MirrorRTWidth( 0 )
,	m_MirrorRTHeight( 0 )
,	m_MirrorAnimation()
,	m_MirrorYaw( 0.0f )
,	m_MirrorViewFOV( 0.0f )
,	m_MirrorViewDistance( 0.0f )
,	m_MirrorViewHeight( 0.0f )
,	m_MirrorViewNearClip( 0.0f )
,	m_MirrorViewFarClip( 0.0f )
,	m_MirrorBackdropTile( 0 )
,	m_MirrorBackdropColor()
,	m_MirrorBackdropDistance( 0.0f )
,	m_MirrorBackdropExtents( 0.0f )
,	m_MirrorIrradiance()
{
}

UIScreenEldMirror::~UIScreenEldMirror()
{
	SafeDelete( m_RigMesh );
	SafeDelete( m_HeadMesh );
	SafeDelete( m_BodyMesh );
	SafeDelete( m_BackdropMesh );
}

/*virtual*/ void UIScreenEldMirror::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	UIScreen::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( MirrorRigMesh );
	const SimpleString MirrorRigMesh = ConfigManager::GetString( sMirrorRigMesh, "", sDefinitionName );
	SetRigMesh( MirrorRigMesh );

	STATICHASH( MirrorAnimation );
	m_MirrorAnimation = ConfigManager::GetHash( sMirrorAnimation, HashedString::NullString, sDefinitionName );

	STATICHASH( MirrorRTWidth );
	m_MirrorRTWidth = ConfigManager::GetInt( sMirrorRTWidth, 0, sDefinitionName );

	STATICHASH( MirrorRTHeight );
	m_MirrorRTHeight = ConfigManager::GetInt( sMirrorRTHeight, 0, sDefinitionName );

	STATICHASH( MirrorYaw );
	m_MirrorYaw = DEGREES_TO_RADIANS( ConfigManager::GetFloat( sMirrorYaw, 0.0f, sDefinitionName ) );

	STATICHASH( MirrorViewFOV );
	m_MirrorViewFOV = ConfigManager::GetFloat( sMirrorViewFOV, 0.0f, sDefinitionName );

	STATICHASH( MirrorViewDistance );
	m_MirrorViewDistance = ConfigManager::GetFloat( sMirrorViewDistance, 0.0f, sDefinitionName );

	STATICHASH( MirrorViewHeight );
	m_MirrorViewHeight = ConfigManager::GetFloat( sMirrorViewHeight, 0.0f, sDefinitionName );

	STATICHASH( MirrorViewNearClip );
	m_MirrorViewNearClip = ConfigManager::GetFloat( sMirrorViewNearClip, 0.0f, sDefinitionName );

	STATICHASH( MirrorViewFarClip );
	m_MirrorViewFarClip = ConfigManager::GetFloat( sMirrorViewFarClip, 0.0f, sDefinitionName );

	STATICHASH( MirrorBackdropTile );
	m_MirrorBackdropTile = ConfigManager::GetInt( sMirrorBackdropTile, 0, sDefinitionName );

	STATICHASH( MirrorBackdropR );
	m_MirrorBackdropColor.r = ConfigManager::GetFloat( sMirrorBackdropR, 0.0f, sDefinitionName );

	STATICHASH( MirrorBackdropG );
	m_MirrorBackdropColor.g = ConfigManager::GetFloat( sMirrorBackdropG, 0.0f, sDefinitionName );

	STATICHASH( MirrorBackdropB );
	m_MirrorBackdropColor.b = ConfigManager::GetFloat( sMirrorBackdropB, 0.0f, sDefinitionName );

	m_MirrorBackdropColor.a = 1.0f;

	STATICHASH( MirrorBackdropDist );
	m_MirrorBackdropDistance = ConfigManager::GetFloat( sMirrorBackdropDist, 0.0f, sDefinitionName );

	STATICHASH( MirrorBackdropSize );
	m_MirrorBackdropExtents = 0.5f * ConfigManager::GetFloat( sMirrorBackdropSize, 0.0f, sDefinitionName );

	STATICHASH( MirrorLightR );
	const float MirrorLightR = ConfigManager::GetFloat( sMirrorLightR, 0.0f, sDefinitionName );

	STATICHASH( MirrorLightG );
	const float MirrorLightG = ConfigManager::GetFloat( sMirrorLightG, 0.0f, sDefinitionName );

	STATICHASH( MirrorLightB );
	const float MirrorLightB = ConfigManager::GetFloat( sMirrorLightB, 0.0f, sDefinitionName );

	STATICHASH( MirrorLightX );
	const float MirrorLightX = ConfigManager::GetFloat( sMirrorLightX, 0.0f, sDefinitionName );

	STATICHASH( MirrorLightY );
	const float MirrorLightY = ConfigManager::GetFloat( sMirrorLightY, 0.0f, sDefinitionName );

	STATICHASH( MirrorLightZ );
	const float MirrorLightZ = ConfigManager::GetFloat( sMirrorLightZ, 0.0f, sDefinitionName );

	const Vector4	MirrorLightColor	= Vector4( MirrorLightR, MirrorLightG, MirrorLightB, 1.0f );
	const Vector	MirrorLightDir		= Vector( MirrorLightX, MirrorLightY, MirrorLightZ ).GetNormalized();

	m_MirrorIrradiance.m_Light[ IRRDIR_Right ]	= MirrorLightColor * Saturate( MirrorLightDir.Dot( Vector( -1.0f, 0.0f, 0.0f ) ) );
	m_MirrorIrradiance.m_Light[ IRRDIR_Left ]	= MirrorLightColor * Saturate( MirrorLightDir.Dot( Vector( 1.0f, 0.0f, 0.0f ) ) );
	m_MirrorIrradiance.m_Light[ IRRDIR_Front ]	= MirrorLightColor * Saturate( MirrorLightDir.Dot( Vector( 0.0f, -1.0f, 0.0f ) ) );
	m_MirrorIrradiance.m_Light[ IRRDIR_Back ]	= MirrorLightColor * Saturate( MirrorLightDir.Dot( Vector( 0.0f, 1.0f, 0.0f ) ) );
	m_MirrorIrradiance.m_Light[ IRRDIR_Up ]		= MirrorLightColor * Saturate( MirrorLightDir.Dot( Vector( 0.0f, 0.0f, -1.0f ) ) );
	m_MirrorIrradiance.m_Light[ IRRDIR_Down ]	= MirrorLightColor * Saturate( MirrorLightDir.Dot( Vector( 0.0f, 0.0f, 1.0f ) ) );
}

/*virtual*/ void UIScreenEldMirror::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	STATIC_HASHED_STRING( SetHeadMesh );
	STATIC_HASHED_STRING( SetBodyMesh );

	const HashedString EventName = Event.GetEventName();

	if( EventName == sSetHeadMesh )
	{
		STATIC_HASHED_STRING( HeadMesh );
		const SimpleString HeadMesh = Event.GetString( sHeadMesh );

		STATIC_HASHED_STRING( HeadTexture );
		const SimpleString HeadTexture = Event.GetString( sHeadTexture );

		SetHeadMesh( HeadMesh, HeadTexture );
	}
	else if( EventName == sSetBodyMesh )
	{
		STATIC_HASHED_STRING( BodyMesh );
		const SimpleString BodyMesh = Event.GetString( sBodyMesh );

		STATIC_HASHED_STRING( BodyTexture );
		const SimpleString BodyTexture = Event.GetString( sBodyTexture );

		SetBodyMesh( BodyMesh, BodyTexture );
	}
}

void UIScreenEldMirror::SetRigMesh( const SimpleString& MeshName )
{
	SafeDelete( m_RigMesh );
	m_RigMesh = CreateRigMesh( MeshName );
}

void UIScreenEldMirror::SetHeadMesh( const SimpleString& MeshName, const SimpleString& TextureName )
{
	SafeDelete( m_HeadMesh );
	m_HeadMesh = CreateMesh( MeshName, TextureName );

	ASSERT( m_RigMesh );
	m_HeadMesh->CopyAnimationsFrom( m_RigMesh );

	PlayAnimation();
}

void UIScreenEldMirror::SetBodyMesh( const SimpleString& MeshName, const SimpleString& TextureName )
{
	SafeDelete( m_BodyMesh );
	m_BodyMesh = CreateMesh( MeshName, TextureName );

	ASSERT( m_RigMesh );
	m_BodyMesh->CopyAnimationsFrom( m_RigMesh );

	PlayAnimation();
}

// Rig isn't meant to be rendered, so don't bother with the other stuff.
Mesh* UIScreenEldMirror::CreateRigMesh( const SimpleString& MeshName )
{
	EldritchFramework* const	pFramework	= EldritchFramework::GetInstance();
	IRenderer* const			pRenderer	= pFramework->GetRenderer();

	EldritchMesh* const pMesh = new EldritchMesh();
	pRenderer->GetMeshFactory()->GetDynamicMesh( MeshName.CStr(), pMesh );

	return pMesh;
}

Mesh* UIScreenEldMirror::CreateMesh( const SimpleString& MeshName, const SimpleString& TextureName )
{
	EldritchFramework* const	pFramework		= EldritchFramework::GetInstance();
	IRenderer* const			pRenderer		= pFramework->GetRenderer();
	TextureManager* const		pTextureManager	= pRenderer->GetTextureManager();
	EldritchWorld* const		pWorld			= pFramework->GetWorld();

	EldritchMesh* const pMesh = new EldritchMesh();
	pRenderer->GetMeshFactory()->GetDynamicMesh( MeshName.CStr(), pMesh );

	if( TextureName != "" )
	{
		pMesh->SetTexture( 0, pTextureManager->GetTexture( TextureName.CStr() ) );
	}

	// Null out the default normal and spec textures; shader code will populate with the current fog lookup texture, etc.
	pMesh->SetTexture( 1, NULL );
	pMesh->SetTexture( 2, NULL );

	pMesh->SetMaterialDefinition( "Material_EntityAnimated", pRenderer );
	pMesh->SetMaterialFlags( MAT_OFFSCREEN_0 );

	ASSERT( pWorld );
	// SetIrradianceCube automatically adds global light too.
	pMesh->SetIrradianceCube( m_MirrorIrradiance );

	// Orient everything in the mirror off axis to get better beauty lighting from cube lights
	pMesh->m_Rotation.Yaw = m_MirrorYaw;

	return pMesh;
}

void UIScreenEldMirror::CreateBackdropMesh()
{
	ASSERT( !m_BackdropMesh );

	EldritchFramework* const	pFramework		= EldritchFramework::GetInstance();
	EldritchWorld* const		pWorld			= pFramework->GetWorld();
	IRenderer* const			pRenderer		= pFramework->GetRenderer();

	ASSERT( pWorld );

	static const uint kNumVertices	= 4;
	static const uint kNumIndices	= 6;

	Vector2 UVMin;
	Vector2 UVMax;
	pWorld->GetTileUVs( m_MirrorBackdropTile, UVMin, UVMax );

	Array<Vector>	Positions;
	Array<Vector4>	Colors;
	Array<Vector2>	UVs;
	Array<index_t>	Indices;

	// For beauty lighting with cube lights
	const Matrix	MirrorRotation	= Matrix::CreateRotationAboutZ( m_MirrorYaw );

	Positions.PushBack( MirrorRotation * Vector( m_MirrorBackdropExtents,	-m_MirrorBackdropDistance, -m_MirrorBackdropExtents + m_MirrorViewHeight ) );
	Positions.PushBack( MirrorRotation * Vector( -m_MirrorBackdropExtents,	-m_MirrorBackdropDistance, -m_MirrorBackdropExtents + m_MirrorViewHeight ) );
	Positions.PushBack( MirrorRotation * Vector( m_MirrorBackdropExtents,	-m_MirrorBackdropDistance, m_MirrorBackdropExtents + m_MirrorViewHeight ) );
	Positions.PushBack( MirrorRotation * Vector( -m_MirrorBackdropExtents,	-m_MirrorBackdropDistance, m_MirrorBackdropExtents + m_MirrorViewHeight ) );

	Colors.PushBack( m_MirrorBackdropColor );
	Colors.PushBack( m_MirrorBackdropColor );
	Colors.PushBack( m_MirrorBackdropColor );
	Colors.PushBack( m_MirrorBackdropColor );

	UVs.PushBack( Vector2( UVMin.uv_u, UVMax.uv_v ) );
	UVs.PushBack( Vector2( UVMax.uv_u, UVMax.uv_v ) );
	UVs.PushBack( Vector2( UVMin.uv_u, UVMin.uv_v ) );
	UVs.PushBack( Vector2( UVMax.uv_u, UVMin.uv_v ) );

	Indices.PushBack( 0 );
	Indices.PushBack( 1 );
	Indices.PushBack( 3 );
	Indices.PushBack( 0 );
	Indices.PushBack( 3 );
	Indices.PushBack( 2 );

	IVertexBuffer* const		pVertexBuffer		= pRenderer->CreateVertexBuffer();
	IVertexDeclaration* const	pVertexDeclaration	= pRenderer->GetVertexDeclaration( VD_POSITIONS | VD_FLOATCOLORS_SM2 | VD_UVS );
	IIndexBuffer* const			pIndexBuffer		= pRenderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices	= kNumVertices;
	InitStruct.Positions	= Positions.GetData();
	InitStruct.FloatColors1	= Colors.GetData();
	InitStruct.UVs			= UVs.GetData();
	pVertexBuffer->Init( InitStruct );
	pIndexBuffer->Init( kNumIndices, Indices.GetData() );
	pIndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );

	m_BackdropMesh = new Mesh( pVertexBuffer, pVertexDeclaration, pIndexBuffer );

	m_BackdropMesh->SetTexture( 0, pWorld->GetTileTexture() );
	m_BackdropMesh->SetMaterialDefinition( "Material_World", pRenderer );
	m_BackdropMesh->SetMaterialFlags( MAT_OFFSCREEN_0 | MAT_ALWAYS );
}

/*virtual*/ void UIScreenEldMirror::Pushed()
{
	UIScreen::Pushed();

	PlayAnimation();

	if( !m_BackdropMesh )
	{
		CreateBackdropMesh();
	}
}

void UIScreenEldMirror::PlayAnimation()
{
	AnimationState::SPlayAnimationParams Params;
	Params.m_EndBehavior			= AnimationState::EAEB_Loop;
	Params.m_IgnoreIfAlreadyPlaying	= true;

	ASSERT( m_RigMesh );
	m_RigMesh->PlayAnimation( m_MirrorAnimation, Params );
	const float AnimationTime = m_RigMesh->GetAnimationTime();

	if( m_HeadMesh )
	{
		m_HeadMesh->PlayAnimation( m_MirrorAnimation, Params );
		m_HeadMesh->SetAnimationTime( AnimationTime );
	}

	if( m_BodyMesh )
	{
		m_BodyMesh->PlayAnimation( m_MirrorAnimation, Params );
		m_BodyMesh->SetAnimationTime( AnimationTime );
	}
}

/*virtual*/ UIScreen::ETickReturn UIScreenEldMirror::Tick( float DeltaTime, bool HasFocus )
{
	XTRACE_FUNCTION;

	if( m_RigMesh )
	{
		m_RigMesh->Tick( DeltaTime );
	}

	if( m_HeadMesh )
	{
		m_HeadMesh->Tick( DeltaTime );
	}

	if( m_BodyMesh )
	{
		m_BodyMesh->Tick( DeltaTime );
	}

	return UIScreen::Tick( DeltaTime, HasFocus );
}

/*virtual*/ void UIScreenEldMirror::Render( bool HasFocus )
{
	XTRACE_FUNCTION;

	UIScreen::Render( HasFocus );

	RenderMesh( m_HeadMesh );
	RenderMesh( m_BodyMesh );
	RenderMesh( m_BackdropMesh );
}

void UIScreenEldMirror::RenderMesh( Mesh* const pMesh )
{
	ASSERT( pMesh );

	EldritchFramework* const	pFramework	= EldritchFramework::GetInstance();
	IRenderer* const			pRenderer	= pFramework->GetRenderer();

	if( pMesh->IsAnimated() )
	{
		pMesh->UpdateBones();
	}

	if( !pMesh->GetTexture( 1 ) )
	{
		ITexture* const pFogTexture = pFramework->GetGame()->GetFogTexture();
		pMesh->SetTexture( 1, pFogTexture );
	}

	pRenderer->AddMesh( pMesh );
}

void UIScreenEldMirror::OnMirrorRTUpdated()
{
	EldritchFramework* const		pFramework		= EldritchFramework::GetInstance();
	ASSERT( pFramework );

	EldritchTargetManager* const	pTargetManager	= pFramework->GetTargetManager();
	ASSERT( pTargetManager );

	IRenderTarget* const			pMirrorRT		= pTargetManager->GetMirrorRenderTarget();
	ASSERT( pMirrorRT );

	ITexture* const					pTexture		= pMirrorRT->GetColorTextureHandle();
	ASSERT( pTexture );

	STATIC_HASHED_STRING( MirrorImage );
	UIWidgetImage* const pMirrorImage = GetWidget<UIWidgetImage>( sMirrorImage );
	ASSERT( pMirrorImage );

	pMirrorImage->SetTexture( pTexture );
}