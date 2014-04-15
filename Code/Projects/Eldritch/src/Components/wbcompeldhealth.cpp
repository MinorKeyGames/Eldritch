#include "core.h"
#include "wbcompeldhealth.h"
#include "wbeventmanager.h"
#include "configmanager.h"
#include "idatastream.h"
#include "mathcore.h"
#include "eldritchgame.h"
#include "Components/wbcompstatmod.h"

WBCompEldHealth::WBCompEldHealth()
:	m_Health( 0 )
,	m_MaxHealth( 0 )
,	m_InitialHealth( 0 )
,	m_PublishToHUD( false )
,	m_DamageTimeout( 0.0f )
,	m_NextDamageTime( 0.0f )
,	m_Invulnerable( false )
,	m_HidePickupScreenDelay( 0.0f )
,	m_HidePickupScreenUID( 0 )
,	m_DefaultDamageMod()
,	m_DamageTypeMods()
{
}

WBCompEldHealth::~WBCompEldHealth()
{
}

/*virtual*/ void WBCompEldHealth::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Health );
	m_Health = ConfigManager::GetInheritedInt( sHealth, 0, sDefinitionName );
	m_InitialHealth = m_Health;

	STATICHASH( MaxHealth );
	m_MaxHealth = ConfigManager::GetInheritedInt( sMaxHealth, m_Health, sDefinitionName );

	STATICHASH( PublishToHUD );
	m_PublishToHUD = ConfigManager::GetInheritedBool( sPublishToHUD, false, sDefinitionName );

	STATICHASH( DamageTimeout );
	m_DamageTimeout = ConfigManager::GetInheritedFloat( sDamageTimeout, 0.0f, sDefinitionName );

	STATICHASH( HidePickupScreenDelay );
	m_HidePickupScreenDelay = ConfigManager::GetInheritedFloat( sHidePickupScreenDelay, 0.0f, sDefinitionName );

	STATICHASH( DefaultDamageModMul );
	m_DefaultDamageMod.m_Multiply = ConfigManager::GetInheritedFloat( sDefaultDamageModMul, 1.0f, sDefinitionName );

	STATICHASH( DefaultDamageModAdd );
	m_DefaultDamageMod.m_Add = ConfigManager::GetInheritedFloat( sDefaultDamageModAdd, 0.0f, sDefinitionName );

	STATICHASH( NumDamageTypeMods );
	const uint NumDamageTypeMods = ConfigManager::GetInheritedInt( sNumDamageTypeMods, 0, sDefinitionName );
	for( uint DamageTypeModIndex = 0; DamageTypeModIndex < NumDamageTypeMods; ++DamageTypeModIndex )
	{
		const HashedString DamageType = ConfigManager::GetInheritedSequenceHash( "DamageTypeMod%dType", DamageTypeModIndex, HashedString::NullString, sDefinitionName );

		SDamageTypeMod& DamageTypeMod = m_DamageTypeMods[ DamageType ];

		DamageTypeMod.m_Multiply = ConfigManager::GetInheritedSequenceFloat( "DamageTypeMod%dMul", DamageTypeModIndex, 1.0f, sDefinitionName );
		DamageTypeMod.m_Add = ConfigManager::GetInheritedSequenceFloat( "DamageTypeMod%dAdd", DamageTypeModIndex, 0.0f, sDefinitionName );
	}
}

/*virtual*/ void WBCompEldHealth::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( TakeDamage );
	STATIC_HASHED_STRING( GiveHealth );
	STATIC_HASHED_STRING( RestoreHealth );
	STATIC_HASHED_STRING( GiveMaxHealth );
	STATIC_HASHED_STRING( SetInvulnerable );
	STATIC_HASHED_STRING( SetVulnerable );
	STATIC_HASHED_STRING( OnInitialized );
	STATIC_HASHED_STRING( PushPersistence );
	STATIC_HASHED_STRING( PullPersistence );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnInitialized )
	{
		if( m_PublishToHUD )
		{
			PublishToHUD();
		}
	}
	else if( EventName == sTakeDamage )
	{
		STATIC_HASHED_STRING( DamageAmount );
		const int DamageAmount = Event.GetInt( sDamageAmount );

		STATIC_HASHED_STRING( Damager );
		WBEntity* const pDamager = Event.GetEntity( sDamager );

		STATIC_HASHED_STRING( DamageType );
		const HashedString DamageType = Event.GetHash( sDamageType );

		TakeDamage( DamageAmount, pDamager, DamageType );
	}
	else if( EventName == sGiveHealth )
	{
		STATIC_HASHED_STRING( HealthAmount );
		const int HealthAmount = Event.GetInt( sHealthAmount );

		GiveHealth( HealthAmount );
	}
	else if( EventName == sGiveMaxHealth )
	{
		STATIC_HASHED_STRING( MaxHealthAmount );
		const int MaxHealthAmount = Event.GetInt( sMaxHealthAmount );

		STATIC_HASHED_STRING( HealthAmount );
		const int HealthAmount = Event.GetInt( sHealthAmount );

		GiveMaxHealth( MaxHealthAmount, HealthAmount );
	}
	else if( EventName == sRestoreHealth )
	{
		STATIC_HASHED_STRING( TargetHealth );
		const int TargetHealth = Event.GetInt( sTargetHealth );

		RestoreHealth( TargetHealth );
	}
	else if( EventName == sSetInvulnerable )
	{
		m_Invulnerable = true;
	}
	else if( EventName == sSetVulnerable )
	{
		m_Invulnerable = false;
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

/*virtual*/ void WBCompEldHealth::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Bool, IsAlive, IsAlive() );
	WB_SET_CONTEXT( Event, Bool, IsDead, IsDead() );
}

void WBCompEldHealth::TakeDamage( const int DamageAmount, WBEntity* const pDamager, const HashedString& DamageType )
{
	XTRACE_FUNCTION;

	if( m_Invulnerable )
	{
		return;
	}

	const float CurrentTime = GetTime();
	if( CurrentTime < m_NextDamageTime )
	{
		return;
	}

	const int ModifiedDamageAmount = ModifyDamageAmount( DamageAmount, DamageType );
	if( ModifiedDamageAmount <= 0 )
	{
		return;
	}

	// Send the event before subtracting health. This way, the event arrives
	// while the entity is still alive, if this is the killing blow.
	WB_MAKE_EVENT( OnDamaged, GetEntity() );
	WB_LOG_EVENT( OnDamaged );
	WB_SET_AUTO( OnDamaged, Entity, Damager, pDamager );
	WB_SET_AUTO( OnDamaged, Hash, DamageType, DamageType );
	WB_DISPATCH_EVENT( GetEventManager(), OnDamaged, GetEntity() );

	const int PreviousHealth = m_Health;

	m_Health			-= ModifiedDamageAmount;
	m_NextDamageTime	= CurrentTime + m_DamageTimeout;

	if( m_Health <= 0 && PreviousHealth > 0 )
	{
		WB_MAKE_EVENT( OnDied, GetEntity() );
		WB_LOG_EVENT( OnDied );
		WB_SET_AUTO( OnDied, Entity, Killer, pDamager );
		WB_DISPATCH_EVENT( GetEventManager(), OnDied, GetEntity() );
	}

	if( m_PublishToHUD )
	{
		PublishToHUD();
	}
}

int WBCompEldHealth::ModifyDamageAmount( const int DamageAmount, const HashedString& DamageType )
{
	const float					fDamageAmount			= static_cast<float>( DamageAmount );
	float						fModifiedDamageAmount	= fDamageAmount;

	Map<HashedString, SDamageTypeMod>::Iterator ModIter = m_DamageTypeMods.Search( DamageType );
	if( ModIter.IsValid() )
	{
		const SDamageTypeMod&	DamageTypeMod			= ModIter.GetValue();
		fModifiedDamageAmount							= ( fModifiedDamageAmount * DamageTypeMod.m_Multiply ) + DamageTypeMod.m_Add;
	}
	else
	{
		fModifiedDamageAmount							= ( fModifiedDamageAmount * m_DefaultDamageMod.m_Multiply ) + m_DefaultDamageMod.m_Add;
	}

	WBCompStatMod* const	pStatMod					= GET_WBCOMP( GetEntity(), StatMod );
	if( pStatMod )
	{
		WB_MODIFY_FLOAT_SAFE( DamageTaken, fModifiedDamageAmount, pStatMod );
		fModifiedDamageAmount							= WB_MODDED( DamageTaken );
	}

	const int					ModifiedDamageAmount	= static_cast<int>( fModifiedDamageAmount );

	return ModifiedDamageAmount;
}

void WBCompEldHealth::GiveHealth( const int HealthAmount )
{
	XTRACE_FUNCTION;

	m_Health = Min( m_Health + HealthAmount, m_MaxHealth );

	if( m_PublishToHUD )
	{
		PublishToHUD();
	}

	// Show the health pickup screen and hide it after some time

	STATICHASH( HealthPickup );
	STATICHASH( Health );
	ConfigManager::SetInt( sHealth, HealthAmount, sHealthPickup );

	STATIC_HASHED_STRING( HealthPickupScreen );

	{
		WB_MAKE_EVENT( PushUIScreen, NULL );
		WB_SET_AUTO( PushUIScreen, Hash, Screen, sHealthPickupScreen );
		WB_DISPATCH_EVENT( GetEventManager(), PushUIScreen, NULL );
	}

	{
		// Remove previously queued hide event if any
		GetEventManager()->UnqueueEvent( m_HidePickupScreenUID );

		WB_MAKE_EVENT( RemoveUIScreen, NULL );
		WB_SET_AUTO( RemoveUIScreen, Hash, Screen, sHealthPickupScreen );
		m_HidePickupScreenUID = WB_QUEUE_EVENT_DELAY( GetEventManager(), RemoveUIScreen, NULL, m_HidePickupScreenDelay );
	}
}

void WBCompEldHealth::GiveMaxHealth( const int MaxHealthAmount, const int HealthAmount )
{
	m_MaxHealth += MaxHealthAmount;

	GiveHealth( HealthAmount );
}

void WBCompEldHealth::RestoreHealth( const int TargetHealth )
{
	const int ActualTargetHealth = ( TargetHealth > 0 ) ? TargetHealth : m_InitialHealth;
	m_Health = Max( ActualTargetHealth, m_Health );

	if( m_PublishToHUD )
	{
		PublishToHUD();
	}
}

void WBCompEldHealth::PublishToHUD() const
{
	ASSERT( m_PublishToHUD );

	STATICHASH( HUD );

	STATICHASH( Health );
	ConfigManager::SetInt( sHealth, Max( 0, m_Health ), sHUD );

	STATICHASH( MaxHealth );
	ConfigManager::SetInt( sMaxHealth, m_MaxHealth, sHUD );
}

void WBCompEldHealth::PushPersistence() const
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( Health );
	Persistence.SetInt( sHealth, m_Health );

	STATIC_HASHED_STRING( MaxHealth );
	Persistence.SetInt( sMaxHealth, m_MaxHealth );
}

void WBCompEldHealth::PullPersistence()
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( Health );
	m_Health = Persistence.GetInt( sHealth );

	STATIC_HASHED_STRING( MaxHealth );
	m_MaxHealth = Persistence.GetInt( sMaxHealth );

	if( m_PublishToHUD )
	{
		PublishToHUD();
	}
}

#define VERSION_EMPTY			0
#define VERSION_HEALTH			1
#define VERSION_NEXTDAMAGETIME	2
#define VERSION_MAXHEALTH		3
#define VERSION_INVULNERABLE	4
#define VERSION_CURRENT			4

uint WBCompEldHealth::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;	// Version
	Size += 4;	// m_Health
	Size += 4;	// m_NextDamageTime (as time remaining)
	Size += 4;	// m_MaxHealth
	Size += 1;	// m_Invulnerable

	return Size;
}

void WBCompEldHealth::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteInt32( m_Health );

	Stream.WriteFloat( m_NextDamageTime - GetTime() );

	Stream.WriteInt32( m_MaxHealth );

	Stream.WriteBool( m_Invulnerable );
}

void WBCompEldHealth::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_HEALTH )
	{
		m_Health = Stream.ReadInt32();
	}

	if( Version >= VERSION_NEXTDAMAGETIME )
	{
		m_NextDamageTime = GetTime() + Stream.ReadFloat();
	}

	if( Version >= VERSION_MAXHEALTH )
	{
		m_MaxHealth = Stream.ReadInt32();
	}

	if( Version >= VERSION_INVULNERABLE )
	{
		m_Invulnerable = Stream.ReadBool();
	}
}