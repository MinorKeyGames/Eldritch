#include "core.h"
#include "wbcompeldwallet.h"
#include "configmanager.h"
#include "idatastream.h"
#include "mathcore.h"
#include "eldritchgame.h"

WBCompEldWallet::WBCompEldWallet()
:	m_Money( 0 )
,	m_Limit( 0 )
,	m_HidePickupScreenDelay( 0.0f )
,	m_HidePickupScreenUID( 0 )
{
}

WBCompEldWallet::~WBCompEldWallet()
{
}

/*virtual*/ void WBCompEldWallet::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Limit );
	m_Limit = ConfigManager::GetInheritedInt( sLimit, 0, sDefinitionName );

	STATICHASH( HidePickupScreenDelay );
	m_HidePickupScreenDelay = ConfigManager::GetInheritedFloat( sHidePickupScreenDelay, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldWallet::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( AddMoney );
	STATIC_HASHED_STRING( RemoveMoney );
	STATIC_HASHED_STRING( OnInitialized );
	STATIC_HASHED_STRING( PushPersistence );
	STATIC_HASHED_STRING( PullPersistence );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnInitialized )
	{
		PublishToHUD();
	}
	else if( EventName == sAddMoney )
	{
		STATIC_HASHED_STRING( Money );
		const uint Money = Event.GetInt( sMoney );

		STATIC_HASHED_STRING( ShowPickupScreen );
		const bool ShowPickupScreen = Event.GetBool( sShowPickupScreen );

		AddMoney( Money, ShowPickupScreen );
	}
	else if( EventName == sRemoveMoney )
	{
		STATIC_HASHED_STRING( Money );
		const uint Money = Event.GetInt( sMoney );
		RemoveMoney( Money );
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

/*virtual*/ void WBCompEldWallet::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Int, Money, m_Money );
}

void WBCompEldWallet::AddMoney( const uint Money, const bool ShowPickupScreen )
{
	m_Money = Min( m_Money + Money, m_Limit );

	PublishToHUD();

	if( ShowPickupScreen )
	{
		// Show the money pickup screen and hide it after some time

		STATICHASH( MoneyPickup );
		STATICHASH( Money );
		ConfigManager::SetInt( sMoney, Money, sMoneyPickup );

		STATIC_HASHED_STRING( MoneyPickupScreen );

		{
			WB_MAKE_EVENT( PushUIScreen, NULL );
			WB_SET_AUTO( PushUIScreen, Hash, Screen, sMoneyPickupScreen );
			WB_DISPATCH_EVENT( GetEventManager(), PushUIScreen, NULL );
		}

		{
			// Remove previously queued hide event if any
			GetEventManager()->UnqueueEvent( m_HidePickupScreenUID );

			WB_MAKE_EVENT( RemoveUIScreen, NULL );
			WB_SET_AUTO( RemoveUIScreen, Hash, Screen, sMoneyPickupScreen );
			m_HidePickupScreenUID = WB_QUEUE_EVENT_DELAY( GetEventManager(), RemoveUIScreen, NULL, m_HidePickupScreenDelay );
		}
	}
}

void WBCompEldWallet::RemoveMoney( const uint Money )
{
	ASSERT( HasMoney( Money ) );

	m_Money -= Money;

	PublishToHUD();
}

void WBCompEldWallet::PublishToHUD() const
{
	STATICHASH( HUD );
	STATICHASH( Money );
	ConfigManager::SetInt( sMoney, m_Money, sHUD );

	STATICHASH( Bank );
	STATICHASH( WalletMoney );
	ConfigManager::SetInt( sWalletMoney, m_Money, sBank );
}

void WBCompEldWallet::PushPersistence() const
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( Money );
	Persistence.SetInt( sMoney, m_Money );
}

void WBCompEldWallet::PullPersistence()
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( Money );
	m_Money = Persistence.GetInt( sMoney );

	PublishToHUD();
}

#define VERSION_EMPTY	0
#define VERSION_MONEY	1
#define VERSION_CURRENT	1

uint WBCompEldWallet::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;				// Version
	Size += sizeof( int );	// m_Money

	return Size;
}

void WBCompEldWallet::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.WriteUInt32( m_Money );
}

void WBCompEldWallet::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_MONEY )
	{
		m_Money = Stream.ReadUInt32();
	}
}