#include "core.h"
#include "wbcompeldendgamecounter.h"
#include "configmanager.h"
#include "idatastream.h"
#include "mathcore.h"
#include "eldritchgame.h"
#include "eldritchframework.h"
#include "Common/uimanagercommon.h"

WBCompEldEndgameCounter::WBCompEldEndgameCounter()
:	m_Count( 0 )
{
}

WBCompEldEndgameCounter::~WBCompEldEndgameCounter()
{
}

/*virtual*/ void WBCompEldEndgameCounter::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnInitialized );
	STATIC_HASHED_STRING( IncrementEndgameCount );
	STATIC_HASHED_STRING( PushPersistence );
	STATIC_HASHED_STRING( PullPersistence );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnInitialized )
	{
		if( m_Count > 0 )
		{
			PublishToHUD();
		}
		else
		{
			SetHUDHidden( true );
		}
	}
	else if( EventName == sIncrementEndgameCount )
	{
		++m_Count;

		PublishToHUD();
	}
	else if( EventName == sPushPersistence )
	{
		PushPersistence();
	}
	else if( EventName == sPullPersistence )
	{
		PullPersistence();
	}
}

/*virtual*/ void WBCompEldEndgameCounter::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Int, EndgameCount, m_Count );
}

void WBCompEldEndgameCounter::PublishToHUD() const
{
	STATICHASH( HUD );
	STATICHASH( EndgameCount );
	ConfigManager::SetInt( sEndgameCount, m_Count, sHUD );

	SetHUDHidden( false );
}

void WBCompEldEndgameCounter::SetHUDHidden( const bool Hidden ) const
{
	UIManager* const pUIManager = GetFramework()->GetUIManager();
	ASSERT( pUIManager );

	STATIC_HASHED_STRING( HUD );
	STATIC_HASHED_STRING( EndgameImg );
	STATIC_HASHED_STRING( EndgameCounter );

	{
		WB_MAKE_EVENT( SetWidgetHidden, GetEntity() );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sEndgameImg );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, Hidden );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetHidden, pUIManager );
	}

	{
		WB_MAKE_EVENT( SetWidgetHidden, GetEntity() );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sEndgameCounter );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, Hidden );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetHidden, pUIManager );
	}
}

void WBCompEldEndgameCounter::PushPersistence() const
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( EndgameCount );
	Persistence.SetInt( sEndgameCount, m_Count );
}

void WBCompEldEndgameCounter::PullPersistence()
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( EndgameCount );
	m_Count = Persistence.GetInt( sEndgameCount );

	if( m_Count > 0 )
	{
		PublishToHUD();
	}
}

#define VERSION_EMPTY	0
#define VERSION_COUNT	1
#define VERSION_CURRENT	1

uint WBCompEldEndgameCounter::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;	// Version
	Size += 4;	// m_Money

	return Size;
}

void WBCompEldEndgameCounter::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_Count );
}

void WBCompEldEndgameCounter::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_COUNT )
	{
		m_Count = Stream.ReadUInt32();
	}
}