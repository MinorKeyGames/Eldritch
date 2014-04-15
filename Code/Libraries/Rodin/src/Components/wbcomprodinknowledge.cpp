#include "core.h"
#include "wbcomprodinknowledge.h"
#include "configmanager.h"
#include "idatastream.h"
#include "mathcore.h"

WBCompRodinKnowledge::WBCompRodinKnowledge()
:	m_KnowledgeMap()
,	m_ExpireTime( 0.0f )
,	m_StaleSeenTime( 0.0f )
{
}

WBCompRodinKnowledge::~WBCompRodinKnowledge()
{
}

/*virtual*/ void WBCompRodinKnowledge::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( ExpireTime );
	m_ExpireTime = ConfigManager::GetInheritedFloat( sExpireTime, 0.0f, sDefinitionName );

	STATICHASH( StaleSeenTime );
	m_StaleSeenTime = ConfigManager::GetInheritedFloat( sStaleSeenTime, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompRodinKnowledge::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	const float CurrentTime = GetTime();

	FOR_EACH_MAP_NOINCR( KnowledgeIter, m_KnowledgeMap, WBEntityRef, TKnowledge )
	{
		WBEntity* const		pEntity			= KnowledgeIter.GetKey().Get();
		const TKnowledge&	Knowledge		= KnowledgeIter.GetValue();

		STATIC_HASHED_STRING( NeverExpire );
		const bool			NeverExpire		= Knowledge.GetBool( sNeverExpire );

		STATIC_HASHED_STRING( ExpireTime );
		const float			ExpireTime		= Knowledge.GetFloat( sExpireTime );
		const float			TimeRemaining	= ExpireTime - CurrentTime;

		if( pEntity == NULL || ( !NeverExpire && TimeRemaining < 0.0f ) )
		{
			m_KnowledgeMap.Remove( KnowledgeIter );
		}
		else
		{
			++KnowledgeIter;
		}
	}
}

const WBCompRodinKnowledge::TKnowledge* WBCompRodinKnowledge::GetKnowledge( const WBEntityRef& Entity ) const
{
	TKnowledgeMap::Iterator KnowledgeIterator = m_KnowledgeMap.Search( Entity );
	return ( KnowledgeIterator.IsValid() ) ? &KnowledgeIterator.GetValue() : NULL;
}

WBCompRodinKnowledge::TKnowledge* WBCompRodinKnowledge::GetKnowledge( const WBEntityRef& Entity )
{
	TKnowledgeMap::Iterator KnowledgeIterator = m_KnowledgeMap.Search( Entity );
	return ( KnowledgeIterator.IsValid() ) ? &KnowledgeIterator.GetValue() : NULL;
}

WBCompRodinKnowledge::TKnowledge& WBCompRodinKnowledge::UpdateEntity( const WBEntityRef& Entity, const float UpdateTime /*= 0.0f*/, const float ExpireTimeBonus /*= 0.0f*/ )
{
	TKnowledge& Knowledge = m_KnowledgeMap[ Entity ];

	STATIC_HASHED_STRING( LastKnownTime );
	STATIC_HASHED_STRING( ExpireTime );

	// Deal with serialized times that don't correlate to current time
	// (since I don't serialize world time and I can't easily serialize
	// time deltas because it's just a packed event).

	const float CurrentTime			= GetTime();
	const float OldLastKnownTime	= Knowledge.GetFloat( sLastKnownTime );
	const bool	FixTimes			= ( UpdateTime > CurrentTime ) || ( OldLastKnownTime > CurrentTime );

	if( FixTimes )
	{
		const float NewLastKnownTime	= CurrentTime;
		const float NewTimeRemaining	= m_ExpireTime + ExpireTimeBonus;
		const float NewExpireTime		= NewLastKnownTime + NewTimeRemaining;

		Knowledge.SetFloat( sLastKnownTime, NewLastKnownTime );
		Knowledge.SetFloat( sExpireTime, NewExpireTime );
	}
	else
	{
		const float OldExpireTime		= Knowledge.GetFloat( sExpireTime );
		const float OldTimeRemaining	= OldExpireTime - CurrentTime;
		const float NewLastKnownTime	= Max( OldLastKnownTime, ( UpdateTime > 0.0f ) ? UpdateTime : CurrentTime );
		const float NewTimeRemaining	= Max( OldTimeRemaining, m_ExpireTime + ExpireTimeBonus );
		const float NewExpireTime		= NewLastKnownTime + NewTimeRemaining;

		Knowledge.SetFloat( sLastKnownTime, NewLastKnownTime );
		Knowledge.SetFloat( sExpireTime, NewExpireTime );
	}

	return Knowledge;
}

bool WBCompRodinKnowledge::GetLastKnownLocationFor( const WBEntity* const pEntity, Vector& OutLocation ) const
{
	DEVASSERT( pEntity );

	const TKnowledge* const pKnowledgeEntry = GetKnowledge( pEntity );
	if( !pKnowledgeEntry )
	{
		return false;
	}

	STATIC_HASHED_STRING( LastKnownLocation );
	const Vector LastKnownLocation = pKnowledgeEntry->GetVector( sLastKnownLocation );

	STATIC_HASHED_STRING( LastSeenTime );
	const float LastSeenTime = pKnowledgeEntry->GetFloat( sLastSeenTime );

	STATIC_HASHED_STRING( LastSeenLocation );
	const Vector LastSeenLocation = pKnowledgeEntry->GetVector( sLastSeenLocation );

	STATIC_HASHED_STRING( LastHeardTime );
	const float LastHeardTime = pKnowledgeEntry->GetFloat( sLastHeardTime );

	STATIC_HASHED_STRING( LastHeardLocation );
	const Vector LastHeardLocation = pKnowledgeEntry->GetVector( sLastHeardLocation );

	if( LastSeenTime > 0.0f && LastHeardTime > 0.0f )
	{
		// Entity has been seen and heard; use seen location if it's more recent or not too stale.
		const float TimeSinceSeen = GetTime() - LastSeenTime;
		if( LastSeenTime > LastHeardTime || TimeSinceSeen < m_StaleSeenTime )
		{
			OutLocation = LastSeenLocation;
		}
		else
		{
			OutLocation = LastHeardLocation;
		}
	}
	else if( LastHeardTime > 0.0f )
	{
		// Entity has only ever been heard
		OutLocation = LastHeardLocation;
	}
	else if( LastSeenTime > 0.0f )
	{
		// Entity has only ever been seen
		OutLocation = LastSeenLocation;
	}
	else
	{
		// Entity has never been seen or heard, might be markup or sensed some other way
#if BUILD_DEV
		PRINTF( "Knowledge entity has never been seen or heard!\n" );
		PRINTF( "Entity: %s\n", GetEntity()->GetUniqueName().CStr() );
		PRINTF( "Target: %s\n", pEntity->GetUniqueName().CStr() );
		PRINTF( "Location: %s\n", LastKnownLocation.GetString().CStr() );
#endif

		OutLocation = LastKnownLocation;
	}

	ASSERT( !OutLocation.IsZero() );
	return true;
}

bool WBCompRodinKnowledge::IsCurrentlyVisible( const WBEntity* const pEntity ) const
{
	DEVASSERT( pEntity );

	const TKnowledge* const pKnowledgeEntry = GetKnowledge( pEntity );
	if( !pKnowledgeEntry )
	{
		return false;
	}

	STATIC_HASHED_STRING( LastSeenTime );
	const float LastSeenTime = pKnowledgeEntry->GetFloat( sLastSeenTime );

	if( LastSeenTime <= 0.0f )
	{
		return false;
	}

	const float TimeSinceSeen = GetTime() - LastSeenTime;
	if( TimeSinceSeen >= m_StaleSeenTime )
	{
		return false;
	}

	return true;
}

#define VERSION_EMPTY			0
#define VERSION_KNOWLEDGEMAP	1
#define VERSION_CURRENT			1

uint WBCompRodinKnowledge::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version

	Size += 4;					// m_KnowledgeMap.Size()
	FOR_EACH_MAP( KnowledgeIter, m_KnowledgeMap, WBEntityRef, TKnowledge )
	{
		const TKnowledge& Knowledge = KnowledgeIter.GetValue();

		WBPackedEvent PackedKnowledge;
		Knowledge.Pack( PackedKnowledge );

		Size += sizeof( WBEntityRef );		// Knowledge key
		Size += PackedKnowledge.GetSize();	// Knowledge value
	}

	return Size;
}

void WBCompRodinKnowledge::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_KnowledgeMap.Size() );
	FOR_EACH_MAP( KnowledgeIter, m_KnowledgeMap, WBEntityRef, TKnowledge )
	{
		const WBEntityRef& Entity	= KnowledgeIter.GetKey();
		const TKnowledge& Knowledge	= KnowledgeIter.GetValue();

		WBPackedEvent PackedKnowledge;
		Knowledge.Pack( PackedKnowledge );

		Stream.Write( sizeof( WBEntityRef ), &Entity );
		Stream.WriteUInt32( PackedKnowledge.GetSize() );
		Stream.Write( PackedKnowledge.GetSize(), PackedKnowledge.GetData() );
	}
}

void WBCompRodinKnowledge::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	ASSERT( m_KnowledgeMap.Size() == 0 );

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_KNOWLEDGEMAP )
	{
		const uint NumKnowledge = Stream.ReadUInt32();
		for( uint KnowledgeIndex = 0; KnowledgeIndex < NumKnowledge; ++KnowledgeIndex )
		{
			WBEntityRef Entity;
			Stream.Read( sizeof( WBEntityRef ), &Entity );

			const uint PackedKnowledgeSize = Stream.ReadUInt32();
			WBPackedEvent PackedKnowledge;
			PackedKnowledge.Reinit( NULL, PackedKnowledgeSize );
			Stream.Read( PackedKnowledgeSize, PackedKnowledge.GetData() );
			const TKnowledge UnpackedKnowledge = PackedKnowledge.Unpack();

			m_KnowledgeMap.Insert( Entity, UnpackedKnowledge );
		}
	}
}