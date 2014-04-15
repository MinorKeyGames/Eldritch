#include "core.h"
#include "wbcompeldlock.h"
#include "hashedstring.h"
#include "configmanager.h"
#include "idatastream.h"
#include "wbeventmanager.h"
#include "eldritchgame.h"
#include "eldritchpersistence.h"

WBCompEldLock::WBCompEldLock()
:	m_Locked( false )
,	m_Key()
{
}

WBCompEldLock::~WBCompEldLock()
{
}

/*virtual*/ void WBCompEldLock::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Locked );
	m_Locked = ConfigManager::GetInheritedBool( sLocked, false, sDefinitionName );

	STATICHASH( Key );
	m_Key = ConfigManager::GetInheritedHash( sKey, HashedString::NullString, sDefinitionName );
}

void WBCompEldLock::Unlock()
{
	m_Locked = false;

	EldritchPersistence* const pPersistence = GetGame()->GetPersistence();
	ASSERT( pPersistence );
	pPersistence->AddOpenLock( m_Key );

	WB_MAKE_EVENT( OnUnlocked, GetEntity() );
	WB_LOG_EVENT( OnUnlocked );
	WB_DISPATCH_EVENT( GetEventManager(), OnUnlocked, GetEntity() );
}

/*virtual*/ void WBCompEldLock::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnSpawned );
	STATIC_HASHED_STRING( Unlock );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnSpawned )
	{
		EldritchPersistence* const pPersistence = GetGame()->GetPersistence();
		ASSERT( pPersistence );

		if( pPersistence->IsOpenLock( m_Key ) )
		{
			Unlock();
		}
	}
	else if( EventName == sUnlock )
	{
		Unlock();
	}
}

/*virtual*/ void WBCompEldLock::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Bool, Locked, m_Locked );
	WB_SET_CONTEXT( Event, Hash, Key, m_Key );
}

#define VERSION_EMPTY	0
#define VERSION_LOCKED	1
#define VERSION_CURRENT	1

uint WBCompEldLock::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += sizeof( bool );		// m_Locked

	return Size;
}

void WBCompEldLock::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.WriteBool( m_Locked );
}

void WBCompEldLock::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_LOCKED )
	{
		m_Locked = Stream.ReadBool();
	}
}