#include "core.h"
#include "wbcompstatmod.h"
#include "configmanager.h"
#include "wbevent.h"

WBCompStatMod::SStatModifier::SStatModifier()
:	m_Active( false )
,	m_Event()
,	m_Stat()
,	m_Function( EMF_None )
,	m_Value( 0.0f )
{
}

WBCompStatMod::WBCompStatMod()
:	m_StatModMap()
{
}

WBCompStatMod::~WBCompStatMod()
{
}

void WBCompStatMod::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( NumModifiers );
	STATICHASH( Event );
	STATICHASH( Stat );
	STATICHASH( Function );
	STATICHASH( Value );
	MAKEHASH( DefinitionName );

	const int NumModifiers = ConfigManager::GetInheritedInt( sNumModifiers, 0, sDefinitionName );
	for( int ModifierIndex = 0; ModifierIndex < NumModifiers; ++ModifierIndex )
	{
		const HashedString Modifier = ConfigManager::GetInheritedSequenceHash( "Modifier%d", ModifierIndex, HashedString::NullString, sDefinitionName );

		SStatModifier NewStatMod;
		NewStatMod.m_Event = ConfigManager::GetHash( sEvent, HashedString::NullString, Modifier );
		NewStatMod.m_Stat = ConfigManager::GetHash( sStat, HashedString::NullString, Modifier );
		NewStatMod.m_Function = GetModifierFunctionFromString( ConfigManager::GetHash( sFunction, HashedString::NullString, Modifier ) );
		NewStatMod.m_Value = ConfigManager::GetFloat( sValue, 0.0f, Modifier );

		m_StatModMap.Insert( NewStatMod.m_Stat, NewStatMod );
	}
}

/*static*/ WBCompStatMod::EModifierFunction WBCompStatMod::GetModifierFunctionFromString( const HashedString& Function )
{
	static HashedString sAdd( "Add" );
	static HashedString sMultiply( "Multiply" );

	if( Function == sAdd )		{ return EMF_Add; }
	if( Function == sMultiply )	{ return EMF_Multiply; }

	return EMF_None;
}

/*virtual*/ void WBCompStatMod::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( TriggerStatMod );
	STATIC_HASHED_STRING( UnTriggerStatMod );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sTriggerStatMod )
	{
		STATIC_HASHED_STRING( StatModEvent );
		const HashedString StatModEvent = Event.GetHash( sStatModEvent );

		TriggerEvent( StatModEvent );
	}
	else if( EventName == sUnTriggerStatMod )
	{
		STATIC_HASHED_STRING( StatModEvent );
		const HashedString StatModEvent = Event.GetHash( sStatModEvent );

		UnTriggerEvent( StatModEvent );
	}
}

void WBCompStatMod::TriggerEvent( const HashedString& Event )
{
	SetEventActive( Event, true );
}

void WBCompStatMod::UnTriggerEvent( const HashedString& Event )
{
	SetEventActive( Event, false );
}

void WBCompStatMod::SetEventActive( const HashedString& Event, bool Active )
{
	PROFILE_FUNCTION;

	// Activate all the modifiers that match this event
	FOR_EACH_MULTIMAP( StatModIter, m_StatModMap, HashedString, SStatModifier )
	{
		SStatModifier& StatModifier = *StatModIter;
		if( StatModifier.m_Event == Event )
		{
			StatModifier.m_Active = Active;
		}
	}
}

float WBCompStatMod::ModifyFloat( const float Value, const HashedString& StatName )
{
	PROFILE_FUNCTION;

	float AddValue = 0.0f;
	float MulValue = 1.0f;

	// Apply all active modifiers that match this stat name
	FOR_EACH_MULTIMAP_SEARCH( StatModIter, m_StatModMap, HashedString, SStatModifier, StatName )
	{
		SStatModifier& StatModifier = *StatModIter;
		if( StatModifier.m_Active )
		{
			if( StatModifier.m_Function == EMF_Add )
			{
				AddValue += StatModifier.m_Value;
			}
			else if( StatModifier.m_Function == EMF_Multiply )
			{
				MulValue *= StatModifier.m_Value;
			}
		}
	}

	// Always apply multipliers first, if any.
	// If needed, provide a sorting mechanism in data.
	return ( Value * MulValue ) + AddValue;
}