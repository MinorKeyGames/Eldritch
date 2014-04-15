#include "core.h"
#include "wbcompeldfaction.h"
#include "configmanager.h"
#include "wbentity.h"
#include "wbevent.h"
#include "idatastream.h"

WBCompEldFaction::WBCompEldFaction()
:	m_Faction()
,	m_Immutable( false )
{
	EldritchFactions::AddRef();
}

WBCompEldFaction::~WBCompEldFaction()
{
	EldritchFactions::Release();
}

/*virtual*/ void WBCompEldFaction::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Faction );
	m_Faction = ConfigManager::GetInheritedHash( sFaction, HashedString::NullString, sDefinitionName );

	STATICHASH( Immutable );
	m_Immutable = ConfigManager::GetInheritedBool( sImmutable, false, sDefinitionName );
}

/*virtual*/ void WBCompEldFaction::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( SetFaction );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sSetFaction )
	{
		if( m_Immutable )
		{
			// Faction can't be changed, do nothing
		}
		else
		{
			STATIC_HASHED_STRING( Faction );
			m_Faction = Event.GetHash( sFaction );
		}
	}
}

/*virtual*/ void WBCompEldFaction::AddContextToEvent( WBEvent& Event ) const
{
	WB_SET_CONTEXT( Event, Hash, Faction, m_Faction );
}

EldritchFactions::EFactionCon WBCompEldFaction::GetCon( const WBEntity* const pEntityB )
{
	ASSERT( pEntityB );

	WBCompEldFaction* const pFactionB = GET_WBCOMP( pEntityB, EldFaction );
	ASSERT( pFactionB );

	return EldritchFactions::GetCon( m_Faction, pFactionB->m_Faction );
}

EldritchFactions::EFactionCon WBCompEldFaction::GetCon( const HashedString& FactionB )
{
	return EldritchFactions::GetCon( m_Faction, FactionB );
}

/*static*/ EldritchFactions::EFactionCon WBCompEldFaction::GetCon( const WBEntity* const pEntityA, const WBEntity* const pEntityB )
{
	ASSERT( pEntityA );
	ASSERT( pEntityB );

	WBCompEldFaction* const pFactionA = GET_WBCOMP( pEntityA, EldFaction );
	WBCompEldFaction* const pFactionB = GET_WBCOMP( pEntityB, EldFaction );

	// Null factions are always treated as neutral.
	if( pFactionA == NULL || pFactionB == NULL )
	{
		return EldritchFactions::EFR_Neutral;
	}

	return EldritchFactions::GetCon( pFactionA->m_Faction, pFactionB->m_Faction );
}

#define VERSION_EMPTY	0
#define VERSION_FACTION	1
#define VERSION_CURRENT	1

uint WBCompEldFaction::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;						// Version
	Size += sizeof( HashedString );	// m_Faction

	return Size;
}

void WBCompEldFaction::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.WriteHashedString( m_Faction );
}

void WBCompEldFaction::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_FACTION )
	{
		m_Faction = Stream.ReadHashedString();
	}
}