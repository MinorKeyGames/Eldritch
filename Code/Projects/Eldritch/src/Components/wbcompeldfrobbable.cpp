#include "core.h"
#include "wbcompeldfrobbable.h"
#include "aabb.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcollision.h"
#include "wbcompeldmesh.h"
#include "wbentity.h"
#include "eldritchframework.h"
#include "irenderer.h"
#include "configmanager.h"
#include "wbeventmanager.h"
#include "eldritchmesh.h"
#include "idatastream.h"
#include "inputsystem.h"
#include "Common/uimanagercommon.h"

WBCompEldFrobbable::WBCompEldFrobbable()
:	m_IsFrobbable( false )
,	m_IsProbableFrobbable( false )
,	m_HoldReleaseMode( false )
,	m_HandleHoldRelease( false )
,	m_UseCollisionExtents( false )
,	m_UseMeshExtents( false )
,	m_ExtentsFatten( 0.0f )
,	m_BoundOffset()
,	m_BoundExtents()
,	m_Highlight()
,	m_FriendlyName()
,	m_FrobVerb()
,	m_HoldVerb()
{
}

WBCompEldFrobbable::~WBCompEldFrobbable()
{
}

/*virtual*/ void WBCompEldFrobbable::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( EldFrobbable );

	STATICHASH( IsFrobbable );
	m_IsFrobbable = ConfigManager::GetInheritedBool( sIsFrobbable, true, sDefinitionName );

	STATICHASH( HoldReleaseMode );
	m_HoldReleaseMode = ConfigManager::GetInheritedBool( sHoldReleaseMode, false, sDefinitionName );

	STATICHASH( UseCollisionExtents );
	m_UseCollisionExtents = ConfigManager::GetInheritedBool( sUseCollisionExtents, false, sDefinitionName );

	STATICHASH( UseMeshExtents );
	m_UseMeshExtents = ConfigManager::GetInheritedBool( sUseMeshExtents, false, sDefinitionName );

	STATICHASH( ExtentsFatten );
	m_ExtentsFatten = ConfigManager::GetInheritedFloat( sExtentsFatten, 0.0f, sDefinitionName );

	STATICHASH( ExtentsX );
	m_BoundExtents.x = ConfigManager::GetInheritedFloat( sExtentsX, 0.0f, sDefinitionName );

	STATICHASH( ExtentsY );
	m_BoundExtents.y = ConfigManager::GetInheritedFloat( sExtentsY, 0.0f, sDefinitionName );

	STATICHASH( ExtentsZ );
	m_BoundExtents.z = ConfigManager::GetInheritedFloat( sExtentsZ, 0.0f, sDefinitionName );

	STATICHASH( OffsetZ );
	m_BoundOffset.z = ConfigManager::GetInheritedFloat( sOffsetZ, 0.0f, sDefinitionName );

	STATICHASH( HighlightR );
	const float DefaultHighlightR = ConfigManager::GetFloat( sHighlightR, 0.0f, sEldFrobbable );
	m_Highlight.r = ConfigManager::GetInheritedFloat( sHighlightR, DefaultHighlightR, sDefinitionName );

	STATICHASH( HighlightG );
	const float DefaultHighlightG = ConfigManager::GetFloat( sHighlightG, 0.0f, sEldFrobbable );
	m_Highlight.g = ConfigManager::GetInheritedFloat( sHighlightG, DefaultHighlightG, sDefinitionName );

	STATICHASH( HighlightB );
	const float DefaultHighlightB = ConfigManager::GetFloat( sHighlightB, 0.0f, sEldFrobbable );
	m_Highlight.b = ConfigManager::GetInheritedFloat( sHighlightB, DefaultHighlightB, sDefinitionName );

	STATICHASH( FriendlyName );
	m_FriendlyName = ConfigManager::GetInheritedString( sFriendlyName, GetEntity()->GetName().CStr(), sDefinitionName );

	STATICHASH( FrobVerb );
	m_FrobVerb = ConfigManager::GetInheritedString( sFrobVerb, "", sDefinitionName );

	STATICHASH( HoldVerb );
	m_HoldVerb = ConfigManager::GetInheritedString( sHoldVerb, "", sDefinitionName );
}

/*virtual*/ void WBCompEldFrobbable::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( MarshalFrob );
	STATIC_HASHED_STRING( OnInitialized );
	STATIC_HASHED_STRING( OnDestroyed );
	STATIC_HASHED_STRING( OnMeshUpdated );
	STATIC_HASHED_STRING( SetIsFrobbable );
	STATIC_HASHED_STRING( BecomeFrobbable );
	STATIC_HASHED_STRING( BecomeNonFrobbable );
	STATIC_HASHED_STRING( SetHoldReleaseMode );
	STATIC_HASHED_STRING( SetFriendlyName );
	STATIC_HASHED_STRING( SetFrobVerb );
	STATIC_HASHED_STRING( SetBoundExtents );
	STATIC_HASHED_STRING( SetBoundOffsetZ );

	const HashedString EventName = Event.GetEventName();

	if( EventName == sMarshalFrob )
	{
		STATIC_HASHED_STRING( Frobber );
		WBEntity* const pFrobber = Event.GetEntity( sFrobber );

		STATIC_HASHED_STRING( InputEdge );
		const int InputEdge = Event.GetInt( sInputEdge );

		MarshalFrob( pFrobber, InputEdge );
	}
	else if( EventName == sOnInitialized )
	{
		if( m_UseCollisionExtents )
		{
			WBCompEldCollision* const pCollision = GET_WBCOMP( GetEntity(), EldCollision );
			if( pCollision )
			{
				m_BoundExtents = pCollision->GetExtents() + Vector( m_ExtentsFatten, m_ExtentsFatten, m_ExtentsFatten );
			}
		}
	}
	else if( EventName == sOnDestroyed )
	{
		if( m_IsProbableFrobbable )
		{
			SetHUDHidden( true );
		}
	}
	else if( EventName == sOnMeshUpdated )
	{
		ASSERT( m_UseMeshExtents );

		WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
		DEVASSERT( pTransform );

		WBCompEldMesh* const pMeshComponent = GET_WBCOMP( GetEntity(), EldMesh );
		ASSERT( pMeshComponent );

		EldritchMesh* const pMesh = pMeshComponent->GetMesh();
		ASSERT( pMesh );

		m_BoundExtents	= pMesh->m_AABB.GetExtents() + Vector( m_ExtentsFatten, m_ExtentsFatten, m_ExtentsFatten );
		m_BoundOffset	= pMesh->m_AABB.GetCenter() - pTransform->GetLocation();
	}
	else if( EventName == sSetIsFrobbable )
	{
		STATIC_HASHED_STRING( IsFrobbable );
		m_IsFrobbable = Event.GetBool( sIsFrobbable );
	}
	else if( EventName == sBecomeFrobbable )
	{
		m_IsFrobbable = true;
	}
	else if( EventName == sBecomeNonFrobbable )
	{
		m_IsFrobbable = false;
	}
	else if( EventName == sSetHoldReleaseMode )
	{
		const bool WasHoldReleaseMode = m_HoldReleaseMode;

		STATIC_HASHED_STRING( HoldReleaseMode );
		m_HoldReleaseMode	= Event.GetBool( sHoldReleaseMode );

		if( m_HoldReleaseMode && !WasHoldReleaseMode )
		{
			m_HandleHoldRelease	= false;
		}

		if( GetIsFrobTarget() )
		{
			PublishToHUD();
		}
	}
	else if( EventName == sSetFriendlyName )
	{
		STATIC_HASHED_STRING( FriendlyName );
		m_FriendlyName = Event.GetString( sFriendlyName );

		if( GetIsFrobTarget() )
		{
			PublishToHUD();
		}
	}
	else if( EventName == sSetFrobVerb )
	{
		STATIC_HASHED_STRING( FrobVerb );
		m_FrobVerb = Event.GetString( sFrobVerb );

		if( GetIsFrobTarget() )
		{
			PublishToHUD();
		}
	}
	else if( EventName == sSetBoundExtents )
	{
		STATIC_HASHED_STRING( BoundExtents );
		m_BoundExtents = Event.GetVector( sBoundExtents );
	}
	else if ( EventName == sSetBoundOffsetZ )
	{
		STATIC_HASHED_STRING( BoundOffsetZ );
		m_BoundOffset.z = Event.GetFloat( sBoundOffsetZ );
	}
}

/*virtual*/ void WBCompEldFrobbable::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Bool, IsFrobbable, m_IsFrobbable );
}

AABB WBCompEldFrobbable::GetBound() const
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	return AABB::CreateFromCenterAndExtents( pTransform->GetLocation() + m_BoundOffset, m_BoundExtents );
}

#if BUILD_DEV
/*virtual*/ void WBCompEldFrobbable::DebugRender() const
{
	WBCompEldTransform* const pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector Location = pTransform->GetLocation() + m_BoundOffset;
	GetFramework()->GetRenderer()->DEBUGDrawBox( Location - m_BoundExtents, Location + m_BoundExtents, ARGB_TO_COLOR( 255, 255, 128, 0 ) );
}
#endif

void WBCompEldFrobbable::SetIsFrobTarget( const bool IsFrobTarget, WBEntity* const pFrobber )
{
	m_IsProbableFrobbable = IsFrobTarget;

	WB_MAKE_EVENT( OnBecameFrobTarget, GetEntity() );
	WB_SET_AUTO( OnBecameFrobTarget, Bool, IsFrobTarget, m_IsProbableFrobbable );
	WB_SET_AUTO( OnBecameFrobTarget, Entity, Frobber, pFrobber );
	WB_SET_AUTO( OnBecameFrobTarget, Vector, Highlight, m_Highlight );
	WB_DISPATCH_EVENT( GetEventManager(), OnBecameFrobTarget, GetEntity() );

	if( IsFrobTarget )
	{
		PublishToHUD();
	}
	else
	{
		SetHUDHidden( true );
	}
}

void WBCompEldFrobbable::PublishToHUD() const
{
	STATICHASH( HUD );

	STATICHASH( FrobName );
	ConfigManager::SetString( sFrobName, m_FriendlyName.CStr(), sHUD );

	STATICHASH( FrobVerb );
	ConfigManager::SetString( sFrobVerb, m_FrobVerb.CStr(), sHUD );

	STATICHASH( HoldVerb );
	ConfigManager::SetString( sHoldVerb, m_HoldVerb.CStr(), sHUD );

	SetHUDHidden( false );
}

void WBCompEldFrobbable::SetHUDHidden( const bool Hidden ) const
{
	UIManager* const pUIManager = GetFramework()->GetUIManager();
	ASSERT( pUIManager );

	STATIC_HASHED_STRING( HUD );
	STATIC_HASHED_STRING( FrobName );
	STATIC_HASHED_STRING( FrobVerb );
	STATIC_HASHED_STRING( FrobHold );

	{
		WB_MAKE_EVENT( SetWidgetHidden, GetEntity() );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sFrobName );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, Hidden );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetHidden, pUIManager );
	}

	{
		WB_MAKE_EVENT( SetWidgetHidden, GetEntity() );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sFrobVerb );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, Hidden );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetHidden, pUIManager );
	}

	{
		WB_MAKE_EVENT( SetWidgetHidden, GetEntity() );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sFrobHold );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, Hidden || !m_HoldReleaseMode );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetHidden, pUIManager );
	}
}

void WBCompEldFrobbable::MarshalFrob( WBEntity* const pFrobber, const int InputEdge )
{
	if( m_HoldReleaseMode )
	{
		if( InputEdge == InputSystem::EIE_OnRise )
		{
			m_HandleHoldRelease = true;
		}
		else if( InputEdge == InputSystem::EIE_OnHold && m_HandleHoldRelease )
		{
			SendOnFrobbedHeldEvent( pFrobber );
			m_HandleHoldRelease = false;	// NOTE: This means we won't get the OnRelease event for this input! That's what I want currently, but maybe not always.
		}
		else if( InputEdge == InputSystem::EIE_OnFall && m_HandleHoldRelease )
		{
			SendOnFrobbedEvent( pFrobber );
			m_HandleHoldRelease = false;
		}
	}
	else
	{
		if( InputEdge == InputSystem::EIE_OnRise )
		{
			SendOnFrobbedEvent( pFrobber );
		}
	}
}

void WBCompEldFrobbable::SendOnFrobbedEvent( WBEntity* const pFrobber ) const
{
	WB_MAKE_EVENT( OnFrobbed, GetEntity() );
	WB_SET_AUTO( OnFrobbed, Entity, Frobber, pFrobber );
	WB_DISPATCH_EVENT( GetEventManager(), OnFrobbed, GetEntity() );
}

void WBCompEldFrobbable::SendOnFrobbedHeldEvent( WBEntity* const pFrobber ) const
{
	WB_MAKE_EVENT( OnFrobbedHeld, GetEntity() );
	WB_SET_AUTO( OnFrobbedHeld, Entity, Frobber, pFrobber );
	WB_DISPATCH_EVENT( GetEventManager(), OnFrobbedHeld, GetEntity() );
}

#define VERSION_EMPTY			0
#define VERSION_ISFROBBABLE		1
#define VERSION_HOLDRELEASEMODE	2
#define VERSION_FRIENDLYNAME	3
#define VERSION_FROBVERB		4
#define VERSION_BOUNDS			5
#define VERSION_CURRENT			5

uint WBCompEldFrobbable::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;	// Version
	Size += 1;	// m_IsFrobbable
	Size += 1;	// m_HoldReleaseMode
	Size += IDataStream::SizeForWriteString( m_FriendlyName );
	Size += IDataStream::SizeForWriteString( m_FrobVerb );

	Size += sizeof( Vector );	// m_BoundOffset
	Size += sizeof( Vector );	// m_BoundExtents

	return Size;
}

void WBCompEldFrobbable::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteBool( m_IsFrobbable );

	Stream.WriteBool( m_HoldReleaseMode );

	Stream.WriteString( m_FriendlyName );

	Stream.WriteString( m_FrobVerb );

	Stream.Write( sizeof( Vector ), &m_BoundOffset );
	Stream.Write( sizeof( Vector ), &m_BoundExtents );
}

void WBCompEldFrobbable::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_ISFROBBABLE )
	{
		m_IsFrobbable = Stream.ReadBool();
	}

	if( Version >= VERSION_HOLDRELEASEMODE )
	{
		m_HoldReleaseMode = Stream.ReadBool();
	}

	if( Version >= VERSION_FRIENDLYNAME )
	{
		m_FriendlyName = Stream.ReadString();
	}

	if( Version >= VERSION_FROBVERB )
	{
		m_FrobVerb = Stream.ReadString();
	}

	if( Version >= VERSION_BOUNDS )
	{
		Stream.Read( sizeof( Vector ), &m_BoundOffset );
		Stream.Read( sizeof( Vector ), &m_BoundExtents );
	}
}