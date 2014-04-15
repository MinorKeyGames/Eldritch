#include "core.h"
#include "wbcompeldkeyring.h"
#include "configmanager.h"
#include "idatastream.h"
#include "mathcore.h"
#include "eldritchgame.h"

WBCompEldKeyRing::WBCompEldKeyRing()
:	m_Keys( 0 )
,	m_HidePickupScreenDelay( 0.0f )
,	m_HidePickupScreenUID( 0 )
{
}

WBCompEldKeyRing::~WBCompEldKeyRing()
{
}

/*virtual*/ void WBCompEldKeyRing::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( HidePickupScreenDelay );
	m_HidePickupScreenDelay = ConfigManager::GetInheritedFloat( sHidePickupScreenDelay, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldKeyRing::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( AddKey );
	STATIC_HASHED_STRING( RemoveKey );
	STATIC_HASHED_STRING( OnInitialized );
	STATIC_HASHED_STRING( PushPersistence );
	STATIC_HASHED_STRING( PullPersistence );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnInitialized )
	{
		PublishToHUD();
	}
	else if( EventName == sAddKey )
	{
		STATIC_HASHED_STRING( ShowPickupScreen );
		const bool ShowPickupScreen = Event.GetBool( sShowPickupScreen );

		AddKeys( 1, ShowPickupScreen );
	}
	else if( EventName == sRemoveKey )
	{
		RemoveKey();
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

/*virtual*/ void WBCompEldKeyRing::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Int, Keys, m_Keys );
}

void WBCompEldKeyRing::AddKeys( const uint Keys, const bool ShowPickupScreen )
{
	m_Keys += Keys;

	PublishToHUD();

	if( ShowPickupScreen )
	{
		// Show the money pickup screen and hide it after some time

		STATICHASH( KeyPickup );
		STATICHASH( Keys );
		ConfigManager::SetInt( sKeys, Keys, sKeyPickup );

		STATIC_HASHED_STRING( KeyPickupScreen );

		{
			WB_MAKE_EVENT( PushUIScreen, NULL );
			WB_SET_AUTO( PushUIScreen, Hash, Screen, sKeyPickupScreen );
			WB_DISPATCH_EVENT( GetEventManager(), PushUIScreen, NULL );
		}

		{
			// Remove previously queued hide event if any
			GetEventManager()->UnqueueEvent( m_HidePickupScreenUID );

			WB_MAKE_EVENT( RemoveUIScreen, NULL );
			WB_SET_AUTO( RemoveUIScreen, Hash, Screen, sKeyPickupScreen );
			m_HidePickupScreenUID = WB_QUEUE_EVENT_DELAY( GetEventManager(), RemoveUIScreen, NULL, m_HidePickupScreenDelay );
		}
	}
}

void WBCompEldKeyRing::RemoveKey( )
{
	ASSERT( HasKeys() );

	m_Keys -= 1;

	PublishToHUD();
}

void WBCompEldKeyRing::PublishToHUD() const
{
	STATICHASH( HUD );
	STATICHASH( Keys );
	ConfigManager::SetInt( sKeys, m_Keys, sHUD );
}

void WBCompEldKeyRing::PushPersistence() const
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( Keys );
	Persistence.SetInt( sKeys, m_Keys );
}

void WBCompEldKeyRing::PullPersistence()
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( Keys );
	m_Keys = Persistence.GetInt( sKeys );

	PublishToHUD();
}

#define VERSION_EMPTY	0
#define VERSION_KEYS	1
#define VERSION_CURRENT	1

uint WBCompEldKeyRing::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;	// Version
	Size += 4;	// m_Keys

	return Size;
}

void WBCompEldKeyRing::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.WriteUInt32( m_Keys );
}

void WBCompEldKeyRing::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_KEYS )
	{
		m_Keys = Stream.ReadUInt32();
	}
}