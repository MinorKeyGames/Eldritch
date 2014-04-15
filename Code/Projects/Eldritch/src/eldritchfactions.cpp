#include "core.h"
#include "eldritchfactions.h"
#include "map.h"
#include "hashedstring.h"
#include "configmanager.h"

typedef Map<HashedString, EldritchFactions::EFactionCon> TFactionConMap;
typedef Map<HashedString, TFactionConMap> TFactionMap;

static TFactionMap	sFactionMap;
static int			sRefCount = 0;

void StaticInitialize();
void StaticShutDown();
EldritchFactions::EFactionCon StaticGetCon( const HashedString& Con );

void EldritchFactions::AddRef()
{
	if( sRefCount++ == 0 )
	{
		StaticInitialize();
	}
}

void EldritchFactions::Release()
{
	if( --sRefCount == 0 )
	{
		StaticShutDown();
	}
}

EldritchFactions::EFactionCon EldritchFactions::GetCon( const HashedString& FactionA, const HashedString& FactionB )
{
	// Simple case: always friendly to own faction.
	if( FactionA == FactionB )
	{
		return EFR_Friendly;
	}

	const TFactionMap::Iterator FactionAIter = sFactionMap.Search( FactionA );
	if( FactionAIter.IsNull() )
	{
		return EFR_Hostile;
	}

	const TFactionConMap& FactionConMap = FactionAIter.GetValue();
	const TFactionConMap::Iterator FactionBIter = FactionConMap.Search( FactionB );
	if( FactionBIter.IsNull() )
	{
		return EFR_Hostile;
	}

	return FactionBIter.GetValue();
}

void StaticInitialize()
{
	STATICHASH( EldritchFactions );

	STATICHASH( NumFactionCons );
	const uint NumFactionCons = ConfigManager::GetInt( sNumFactionCons, 0, sEldritchFactions );

	for( uint FactionConIndex = 0; FactionConIndex < NumFactionCons; ++FactionConIndex )
	{
		const HashedString FactionA = ConfigManager::GetSequenceHash( "FactionCon%dA", FactionConIndex, HashedString::NullString, sEldritchFactions );
		const HashedString FactionB = ConfigManager::GetSequenceHash( "FactionCon%dB", FactionConIndex, HashedString::NullString, sEldritchFactions );
		const HashedString FactionC = ConfigManager::GetSequenceHash( "FactionCon%dC", FactionConIndex, HashedString::NullString, sEldritchFactions );
		EldritchFactions::EFactionCon Con = StaticGetCon( FactionC );

		TFactionConMap& FactionConMap = sFactionMap[ FactionA ];
		FactionConMap[ FactionB ] = Con;
	}
}

void StaticShutDown()
{
	FOR_EACH_MAP( FactionAIter, sFactionMap, HashedString, TFactionConMap )
	{
		TFactionConMap& FactionConMap = FactionAIter.GetValue();
		FactionConMap.Clear();
	}
	sFactionMap.Clear();
}

EldritchFactions::EFactionCon StaticGetCon( const HashedString& Con )
{
	STATIC_HASHED_STRING( Hostile );
	STATIC_HASHED_STRING( Neutral );
	STATIC_HASHED_STRING( Friendly );

	if( Con == sHostile )
	{
		return EldritchFactions::EFR_Hostile;
	}
	else if( Con == sNeutral )
	{
		return EldritchFactions::EFR_Neutral;
	}
	else if( Con == sFriendly )
	{
		return EldritchFactions::EFR_Friendly;
	}
	else
	{
		WARN;
		return EldritchFactions::EFR_Hostile;
	}
}