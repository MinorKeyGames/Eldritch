#include "core.h"
#include "wbcompeldmapmarker.h"
#include "configmanager.h"
#include "wbeventmanager.h"

WBCompEldMapMarker::WBCompEldMapMarker()
:	m_MarkAsRoom( false )
,	m_Texture()
{
}

WBCompEldMapMarker::~WBCompEldMapMarker()
{
}

/*virtual*/ void WBCompEldMapMarker::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( MarkAsRoom );
	m_MarkAsRoom = ConfigManager::GetInheritedBool( sMarkAsRoom, false, sDefinitionName );

	STATICHASH( Texture );
	m_Texture = ConfigManager::GetInheritedString( sTexture, "", sDefinitionName );
}

/*virtual*/ void WBCompEldMapMarker::HandleEvent( const WBEvent& Event )
{
	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnMoved );
	STATIC_HASHED_STRING( OnTurned );
	STATIC_HASHED_STRING( OnInitializedQueued );
	STATIC_HASHED_STRING( OnDestroyed );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnInitializedQueued )
	{
		AddMarker();
		UpdateMarker();
	}
	else if( EventName == sOnMoved || sOnTurned )
	{
		UpdateMarker();
	}
	else if( EventName == sOnDestroyed )
	{
		RemoveMarker();
	}
}

void WBCompEldMapMarker::AddMarker()
{
	WB_MAKE_EVENT( AddMapMarker, GetEntity() );
	WB_SET_AUTO( AddMapMarker, Hash, Texture, m_Texture );
	WB_DISPATCH_EVENT( GetEventManager(), AddMapMarker, NULL );
}

void WBCompEldMapMarker::RemoveMarker()
{
	WB_MAKE_EVENT( RemoveMapMarker, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), RemoveMapMarker, NULL );
}

void WBCompEldMapMarker::UpdateMarker()
{
	WB_MAKE_EVENT( UpdateMapMarker, GetEntity() );
	WB_SET_AUTO( UpdateMapMarker, Bool, MarkAsRoom, m_MarkAsRoom );
	WB_DISPATCH_EVENT( GetEventManager(), UpdateMapMarker, NULL );
}