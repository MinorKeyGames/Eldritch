#include "core.h"
#include "wbcompeldusable.h"
#include "wbeventmanager.h"
#include "hashedstring.h"
#include "configmanager.h"
#include "inputsystem.h"
#include "idatastream.h"

WBCompEldUsable::WBCompEldUsable()
:	m_HoldReleaseMode( false )
,	m_RefireRate( 0.0f )
,	m_RefireTime( 0.0f )
{
}

WBCompEldUsable::~WBCompEldUsable()
{
}

/*virtual*/ void WBCompEldUsable::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( HoldReleaseMode );
	m_HoldReleaseMode = ConfigManager::GetInheritedBool( sHoldReleaseMode, false, sDefinitionName );

	STATICHASH( RefireRate );
	m_RefireRate = ConfigManager::GetInheritedFloat( sRefireRate, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldUsable::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( Use );
	STATIC_HASHED_STRING( ResetRefireTime );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sUse )
	{
		STATIC_HASHED_STRING( InputEdge );
		const int InputEdge = Event.GetInt( sInputEdge );

		MarshalUsedEvent( InputEdge );
	}
	else if( EventName == sResetRefireTime )
	{
		// Provided so that refire rate can be animation driven.
		m_RefireTime = 0.0f;
	}
}

void WBCompEldUsable::MarshalUsedEvent( const int InputEdge )
{
	if( m_HoldReleaseMode )
	{
		if( InputEdge == InputSystem::EIE_OnHold )
		{
			// NOTE: At the moment, on-hold actions aren't rate limited.
			SendOnUsedHeldEvent();
		}
		else if( InputEdge == InputSystem::EIE_OnFall )
		{
			TryUse();
		}
	}
	else
	{
		if( InputEdge == InputSystem::EIE_OnRise )
		{
			TryUse();
		}
	}
}

void WBCompEldUsable::TryUse()
{
	const float CurrentTime = GetTime();
	if( CurrentTime >= m_RefireTime )
	{
		m_RefireTime = CurrentTime + m_RefireRate;
		SendOnUsedEvent();
	}
}

void WBCompEldUsable::SendOnUsedEvent() const
{
	WB_MAKE_EVENT( OnUsed, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnUsed, GetEntity() );
}

void WBCompEldUsable::SendOnUsedHeldEvent() const
{
	WB_MAKE_EVENT( OnUsedHeld, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnUsedHeld, GetEntity() );
}

#define VERSION_EMPTY		0
#define VERSION_REFIRETIME	1
#define VERSION_CURRENT		1

uint WBCompEldUsable::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += 4;					// m_RefireTime (as time remaining)

	return Size;
}

void WBCompEldUsable::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteFloat( m_RefireTime - GetTime() );
}

void WBCompEldUsable::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_REFIRETIME )
	{
		m_RefireTime = GetTime() + Stream.ReadFloat();
	}
}