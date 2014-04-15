#include "core.h"
#include "wbcompeldammobag.h"
#include "configmanager.h"
#include "idatastream.h"
#include "mathcore.h"
#include "eldritchgame.h"
#include "eldritchframework.h"
#include "Common/uimanagercommon.h"
#include "wbcompeldweapon.h"

WBCompEldAmmoBag::WBCompEldAmmoBag()
:	m_Ammo()
,	m_CurrentAmmo()
,	m_HideAmmoHUD( false )
,	m_DefaultAmmo()
,	m_HidePickupScreenDelay( 0.0f )
,	m_HidePickupScreenUID( 0 )
{
}

WBCompEldAmmoBag::~WBCompEldAmmoBag()
{
}

/*virtual*/ void WBCompEldAmmoBag::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( HideAmmoHUD );
	m_HideAmmoHUD = ConfigManager::GetInheritedBool( sHideAmmoHUD, false, sDefinitionName );

	STATICHASH( DefaultAmmo );
	m_DefaultAmmo = ConfigManager::GetInheritedHash( sDefaultAmmo, HashedString::NullString, sDefinitionName );

	STATICHASH( HidePickupScreenDelay );
	m_HidePickupScreenDelay = ConfigManager::GetInheritedFloat( sHidePickupScreenDelay, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldAmmoBag::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnInitialized );
	STATIC_HASHED_STRING( AddAmmo );
	STATIC_HASHED_STRING( RemoveAmmo );
	STATIC_HASHED_STRING( OnWeaponEquipped );
	STATIC_HASHED_STRING( OnWeaponUnequipped );
	STATIC_HASHED_STRING( PushPersistence );
	STATIC_HASHED_STRING( PullPersistence );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnInitialized )
	{
		ConditionalHideHUD();
	}
	else if( EventName == sAddAmmo )
	{
		STATIC_HASHED_STRING( AmmoType );
		const HashedString AmmoType = Event.GetHash( sAmmoType );

		STATIC_HASHED_STRING( AmmoValue );
		const uint AmmoValue = Event.GetInt( sAmmoValue );

		AddAmmo( AmmoType, AmmoValue );
	}
	else if( EventName == sRemoveAmmo )
	{
		STATIC_HASHED_STRING( AmmoType );
		const HashedString AmmoType = Event.GetHash( sAmmoType );

		RemoveAmmo( AmmoType );
	}
	else if( EventName == sOnWeaponEquipped )
	{
		STATIC_HASHED_STRING( Weapon );
		WBEntity* const pWeaponEntity = Event.GetEntity( sWeapon );
		ASSERT( pWeaponEntity );

		WBCompEldWeapon* const pWeapon = GET_WBCOMP( pWeaponEntity, EldWeapon );
		if( pWeapon && pWeapon->GetAmmo() != HashedString::NullString )
		{
			m_CurrentAmmo = pWeapon->GetAmmo();
			PublishToHUD();

			DEBUGPRINTF( "Current ammo: %d (OnWeaponEquipped %s)\n", m_CurrentAmmo.GetHash(), pWeaponEntity->GetName().CStr() );
		}
		else
		{
			m_CurrentAmmo = HashedString::NullString;
			ConditionalHideHUD();

			DEBUGPRINTF( "Current ammo: %d (OnWeaponEquipped NULL)\n", m_CurrentAmmo.GetHash() );
		}
	}
	else if( EventName == sOnWeaponUnequipped )
	{
		m_CurrentAmmo = HashedString::NullString;
		ConditionalHideHUD();

		DEBUGPRINTF( "Current ammo: %d (OnWeaponUnequipped)\n", m_CurrentAmmo.GetHash() );
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

void WBCompEldAmmoBag::ConditionalHideHUD()
{
	if( m_HideAmmoHUD )
	{
		SetHUDHidden( true );
	}
	else
	{
		PublishToHUD();
	}
}

/*virtual*/ void WBCompEldAmmoBag::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Bool, HasAmmo, HasAmmo( m_CurrentAmmo ) );
}

uint WBCompEldAmmoBag::GetAmmo( const HashedString& AmmoType ) const
{
	Map<HashedString, uint>::Iterator AmmoIter = m_Ammo.Search( AmmoType );
	const uint AmmoValue = AmmoIter.IsValid() ? AmmoIter.GetValue() : 0;
	return AmmoValue;
}

void WBCompEldAmmoBag::AddAmmo( const HashedString& AmmoType, uint Ammo )
{
	m_Ammo[ AmmoType ] += Ammo;

	//if( AmmoType == m_CurrentAmmo )
	{
		PublishToHUD();
	}

	{
		// Show the ammo pickup screen and hide it after some time

		STATICHASH( AmmoPickup );
		STATICHASH( Ammo );
		ConfigManager::SetInt( sAmmo, Ammo, sAmmoPickup );

		STATIC_HASHED_STRING( AmmoPickupScreen );

		{
			WB_MAKE_EVENT( PushUIScreen, NULL );
			WB_SET_AUTO( PushUIScreen, Hash, Screen, sAmmoPickupScreen );
			WB_DISPATCH_EVENT( GetEventManager(), PushUIScreen, NULL );
		}

		{
			// Remove previously queued hide event if any
			GetEventManager()->UnqueueEvent( m_HidePickupScreenUID );

			WB_MAKE_EVENT( RemoveUIScreen, NULL );
			WB_SET_AUTO( RemoveUIScreen, Hash, Screen, sAmmoPickupScreen );
			m_HidePickupScreenUID = WB_QUEUE_EVENT_DELAY( GetEventManager(), RemoveUIScreen, NULL, m_HidePickupScreenDelay );
		}
	}
}

void WBCompEldAmmoBag::RemoveAmmo( const HashedString& AmmoType )
{
	ASSERT( HasAmmo( AmmoType ) );

	--m_Ammo[ AmmoType ];

	DEVASSERT( AmmoType == m_CurrentAmmo );
	//if( AmmoType == m_CurrentAmmo )
	{
		PublishToHUD();
	}
}

void WBCompEldAmmoBag::PublishToHUD() const
{
	const HashedString& PublishAmmo = m_CurrentAmmo ? m_CurrentAmmo : m_DefaultAmmo;

	STATICHASH( HUD );
	STATICHASH( Ammo );
	ConfigManager::SetInt( sAmmo, GetAmmo( PublishAmmo ), sHUD );

	if( m_HideAmmoHUD )
	{
		SetHUDHidden( false );
	}
}

void WBCompEldAmmoBag::SetHUDHidden( const bool Hidden ) const
{
	ASSERT( m_HideAmmoHUD );

	UIManager* const pUIManager = GetFramework()->GetUIManager();
	ASSERT( pUIManager );

	STATIC_HASHED_STRING( HUD );
	STATIC_HASHED_STRING( AmmoImg );
	STATIC_HASHED_STRING( AmmoCounter );

	{
		WB_MAKE_EVENT( SetWidgetHidden, GetEntity() );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sAmmoImg );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, Hidden );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetHidden, pUIManager );
	}

	{
		WB_MAKE_EVENT( SetWidgetHidden, GetEntity() );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sAmmoCounter );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, Hidden );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetHidden, pUIManager );
	}
}

void WBCompEldAmmoBag::PushPersistence() const
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	const uint NumAmmoTypes = m_Ammo.Size();

	STATIC_HASHED_STRING( NumAmmoTypes );
	Persistence.SetInt( sNumAmmoTypes, NumAmmoTypes );

	uint AmmoIndex = 0;
	FOR_EACH_MAP( AmmoIter, m_Ammo, HashedString, uint )
	{
		Persistence.SetHash( SimpleString::PrintF( "Ammo%dKey", AmmoIndex ), AmmoIter.GetKey() );
		Persistence.SetInt( SimpleString::PrintF( "Ammo%dValue", AmmoIndex ), AmmoIter.GetValue() );
		++AmmoIndex;
	}
}

void WBCompEldAmmoBag::PullPersistence()
{
	m_Ammo.Clear();

	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( NumAmmoTypes );
	const uint NumAmmoTypes = Persistence.GetInt( sNumAmmoTypes );

	for( uint AmmoIndex = 0; AmmoIndex < NumAmmoTypes; ++AmmoIndex )
	{
		const HashedString	AmmoKey		= Persistence.GetHash(	SimpleString::PrintF( "Ammo%dKey",		AmmoIndex ) );
		const uint			AmmoValue	= Persistence.GetInt(	SimpleString::PrintF( "Ammo%dValue",	AmmoIndex ) );
		m_Ammo[ AmmoKey ] = AmmoValue;
	}

	PublishToHUD();
}

#define VERSION_EMPTY	0
#define VERSION_AMMO	1
#define VERSION_CURRENT	1

uint WBCompEldAmmoBag::GetSerializationSize()
{
	uint Size = 0;

	const uint AmmoSize = sizeof( HashedString ) + sizeof( uint );

	Size += 4;							// Version
	Size += 4;							// m_Ammo.Size()
	Size += AmmoSize * m_Ammo.Size();	// m_Ammo

	return Size;
}

void WBCompEldAmmoBag::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_Ammo.Size() );
	FOR_EACH_MAP( AmmoIter, m_Ammo, HashedString, uint )
	{
		Stream.WriteHashedString(	AmmoIter.GetKey() );
		Stream.WriteUInt32(			AmmoIter.GetValue() );
	}
}

void WBCompEldAmmoBag::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_AMMO )
	{
		ASSERT( m_Ammo.Empty() );
		const uint NumAmmoTypes = Stream.ReadUInt32();
		for( uint AmmoIndex = 0; AmmoIndex < NumAmmoTypes; ++AmmoIndex )
		{
			const HashedString	AmmoType	= Stream.ReadHashedString();
			const uint			AmmoValue	= Stream.ReadUInt32();
			m_Ammo[ AmmoType ] = AmmoValue;
		}
	}
}