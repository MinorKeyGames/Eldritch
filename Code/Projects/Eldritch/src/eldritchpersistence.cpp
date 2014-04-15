#include "core.h"
#include "eldritchpersistence.h"
#include "idatastream.h"
#include "wbeventmanager.h"

EldritchPersistence::EldritchPersistence()
:	m_BankMoney( 0 )
,	m_OpenLocks()
,	m_CharacterHeadIndex( 0 )
,	m_CharacterBodyIndex( 0 )
,	m_VariableMap()
{
	RegisterForEvents();
}

EldritchPersistence::~EldritchPersistence()
{
	// No need to unregister events; this object outlives the event manager.
}

void EldritchPersistence::RegisterForEvents()
{
	STATIC_HASHED_STRING( SetPersistentVar );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sSetPersistentVar, this, NULL );
}

/*virtual*/ void EldritchPersistence::HandleEvent( const WBEvent& Event )
{
	STATIC_HASHED_STRING( SetPersistentVar );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sSetPersistentVar )
	{
		STATIC_HASHED_STRING( Name );
		const HashedString Name = Event.GetHash( sName );

		STATIC_HASHED_STRING( Value );
		const WBEvent::SParameter* const pParameter = Event.GetParameter( sValue );
		m_VariableMap.Set( Name, pParameter );
	}
}

bool EldritchPersistence::IsOpenLock( const HashedString& Lock ) const
{
	return m_OpenLocks.Search( Lock ).IsValid();
}

void EldritchPersistence::AddOpenLock( const HashedString& Lock )
{
	m_OpenLocks.Insert( Lock );
}

#define VERSION_EMPTY		0
#define VERSION_BANK		1
#define VERSION_OPENLOCKS	2
#define VERSION_CHARACTER	3
#define VERSION_VARIABLEMAP	4
#define VERSION_CURRENT		4

void EldritchPersistence::Save( const IDataStream& Stream ) const
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_BankMoney );

	Stream.WriteUInt32( m_OpenLocks.Size() );
	FOR_EACH_SET( OpenLockIter, m_OpenLocks, HashedString )
	{
		Stream.WriteHashedString( OpenLockIter.GetValue() );
	}

	Stream.WriteUInt32( m_CharacterHeadIndex );
	Stream.WriteUInt32( m_CharacterBodyIndex );

	m_VariableMap.Save( Stream );
}

void EldritchPersistence::Load( const IDataStream& Stream )
{
	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_BANK )
	{
		m_BankMoney = Stream.ReadUInt32();
	}

	if( Version >= VERSION_OPENLOCKS )
	{
		const uint NumOpenLocks = Stream.ReadUInt32();
		for( uint OpenLockIndex = 0; OpenLockIndex < NumOpenLocks; ++OpenLockIndex )
		{
			m_OpenLocks.Insert( Stream.ReadHashedString() );
		}
	}

	if( Version >= VERSION_CHARACTER )
	{
		m_CharacterHeadIndex = Stream.ReadUInt32();
		m_CharacterBodyIndex = Stream.ReadUInt32();
	}

	if( Version >= VERSION_VARIABLEMAP )
	{
		m_VariableMap.Load( Stream );
	}
}