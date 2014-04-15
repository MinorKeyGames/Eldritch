#include "core.h"
#include "wbcompelddropshadow.h"
#include "configmanager.h"
#include "mesh.h"
#include "eldritchframework.h"
#include "irenderer.h"
#include "meshfactory.h"
#include "wbcompeldtransform.h"
#include "wbcompeldmesh.h"
#include "eldritchmesh.h"
#include "aabb.h"
#include "wbentity.h"
#include "eldritchworld.h"
#include "shadermanager.h"
#include "wbeventmanager.h"
#include "idatastream.h"
#include "mathcore.h"
#include "texturemanager.h"
#include "collisioninfo.h"
#include "eldritchgame.h"

WBCompEldDropShadow::WBCompEldDropShadow()
:	m_Mesh( NULL )
,	m_MeshOriginalAABB()
,	m_VoxelCheckOffsetZ( 0.0f )
,	m_ShadowFloatZ( 0.0f )
,	m_Hidden( false )
,	m_ScriptHidden( false )
,	m_UseMeshCenter( false )
{
	STATIC_HASHED_STRING( OnWorldChanged );
	GetEventManager()->AddObserver( sOnWorldChanged, this );
}

WBCompEldDropShadow::~WBCompEldDropShadow()
{
	SafeDelete( m_Mesh );

	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( OnWorldChanged );
		pEventManager->RemoveObserver( sOnWorldChanged, this );
	}
}

/*virtual*/ void WBCompEldDropShadow::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	IRenderer* const pRenderer = GetFramework()->GetRenderer();

	MAKEHASH( DefinitionName );

	STATICHASH( Radius );
	const float Radius = ConfigManager::GetInheritedFloat( sRadius, 0.0f, sDefinitionName );

	STATICHASH( Alpha );
	const float Alpha = ConfigManager::GetInheritedFloat( sAlpha, 0.0f, sDefinitionName );

	m_Mesh = pRenderer->GetMeshFactory()->CreateQuad( Radius * 2.0f, XY_PLANE, false );
	m_Mesh->SetTexture( 0, pRenderer->GetTextureManager()->GetTexture( "Textures/drop-shadow_NODXT.tga" ) );
	m_Mesh->SetTexture( 1, NULL );
	m_Mesh->SetTexture( 2, NULL );
	m_Mesh->SetMaterialDefinition( "Material_DropShadow", pRenderer );
	m_Mesh->SetMaterialFlags( MAT_WORLD | MAT_DYNAMIC | MAT_ALPHA );
	m_Mesh->m_ConstantColor = GetWorld()->GetShadowColor();
	m_Mesh->m_ConstantColor.a = Saturate( Alpha );

	m_MeshOriginalAABB = m_Mesh->m_AABB;

	STATICHASH( VoxelCheckOffsetZ );
	m_VoxelCheckOffsetZ = ConfigManager::GetInheritedFloat( sVoxelCheckOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( ShadowFloatZ );
	m_ShadowFloatZ = ConfigManager::GetInheritedFloat( sShadowFloatZ, 0.0f, sDefinitionName );

	STATICHASH( UseMeshCenter );
	m_UseMeshCenter = ConfigManager::GetInheritedBool( sUseMeshCenter, false, sDefinitionName );
}

bool WBCompEldDropShadow::ShouldBeHidden() const
{
	if( m_ScriptHidden )
	{
		return true;
	}

	WBCompEldTransform* pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );	// Makes no sense to have a shadow and no transform

	const Vector VoxelCheckOffset = Vector( 0.0f, 0.0f, m_VoxelCheckOffsetZ );
	const Vector VoxelCheckBelowOffset = Vector( 0.0f, 0.0f, m_VoxelCheckOffsetZ - 1.0f );

	CollisionInfo Info;
	Info.m_CollideWorld = true;

	if( GetWorld()->PointCheck( pTransform->GetLocation() + VoxelCheckOffset, Info ) )
	{
		// We're inside collision, hide mesh
		return true;
	}

	if( !GetWorld()->PointCheck( pTransform->GetLocation() + VoxelCheckBelowOffset, Info ) )
	{
		// We're floating in the air, hide mesh
		return true;
	}

	return false;
}

void WBCompEldDropShadow::Update()
{
	m_Hidden = ShouldBeHidden();

	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );	// Makes no sense to have a shadow and no transform

	WBCompEldMesh* const pMesh = GET_WBCOMP( pEntity, EldMesh );
	ASSERT( pMesh || !m_UseMeshCenter );

	const Vector	DropLocation		= m_UseMeshCenter ? ( pMesh->GetMesh()->m_AABB.GetCenter() ) : pTransform->GetLocation();
	const Vector	VoxelCheckOffset	= Vector( 0.0f, 0.0f, m_VoxelCheckOffsetZ );
	const Vector	VoxelCheckLocation	= DropLocation + VoxelCheckOffset;
	const float		ShadowZ				= Floor( VoxelCheckLocation.z ) + m_ShadowFloatZ;

	m_Mesh->m_Location		= DropLocation;
	m_Mesh->m_Location.z	= ShadowZ;
	m_Mesh->m_AABB			= m_MeshOriginalAABB;
	m_Mesh->m_AABB.MoveBy( m_Mesh->m_Location );
}

/*virtual*/ void WBCompEldDropShadow::Render()
{
	XTRACE_FUNCTION;

	if( m_Hidden )
	{
		return;
	}

	if( !m_Mesh->GetTexture( 1 ) )
	{
		ITexture* const pFogTexture = GetGame()->GetFogTexture();
		m_Mesh->SetTexture( 1, pFogTexture );
	}

	GetFramework()->GetRenderer()->AddMesh( m_Mesh );
}

#if BUILD_DEV
/*virtual*/ void WBCompEldDropShadow::DebugRender() const
{
	GetFramework()->GetRenderer()->DEBUGDrawBox( m_Mesh->m_AABB.m_Min, m_Mesh->m_AABB.m_Max, ARGB_TO_COLOR( 255, 0, 128, 255 ) );
}
#endif

/*virtual*/ void WBCompEldDropShadow::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnMoved );
	STATIC_HASHED_STRING( OnLoaded );
	STATIC_HASHED_STRING( OnWorldChanged );
	STATIC_HASHED_STRING( OnMeshUpdated );
	STATIC_HASHED_STRING( Hide );
	STATIC_HASHED_STRING( Show );
	STATIC_HASHED_STRING( HideShadow );
	STATIC_HASHED_STRING( ShowShadow );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnMoved || EventName == sOnLoaded || EventName == sOnWorldChanged )
	{
		Update();
	}
	else if( EventName == sOnMeshUpdated )
	{
		ASSERT( m_UseMeshCenter );

		Update();
	}
	else if( EventName == sHide || EventName == sHideShadow )
	{
		m_ScriptHidden = true;
		Update();
	}
	else if( EventName == sShow || EventName == sShowShadow )
	{
		m_ScriptHidden = false;
		Update();
	}
}

#define VERSION_EMPTY			0
#define VERSION_HIDDEN			1
#define VERSION_MESHLOCATION	2
#define VERSION_SCRIPTHIDDEN	3
#define VERSION_CURRENT			3

uint WBCompEldDropShadow::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += 1;					// m_Hidden
	Size += 1;					// m_ScriptHidden
	Size += sizeof( Vector );	// m_Mesh->m_Location

	return Size;
}

void WBCompEldDropShadow::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.WriteBool( m_Hidden );
	Stream.WriteBool( m_ScriptHidden );
	Stream.Write( sizeof( m_Mesh->m_Location ), &m_Mesh->m_Location );
}

void WBCompEldDropShadow::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_HIDDEN )
	{
		m_Hidden = Stream.ReadBool();
	}

	if( Version >= VERSION_SCRIPTHIDDEN )
	{
		m_ScriptHidden = Stream.ReadBool();
	}

	if( Version >= VERSION_MESHLOCATION )
	{
		ASSERT( m_Mesh );
		Stream.Read( sizeof( m_Mesh->m_Location ), &m_Mesh->m_Location );
	}
}