#include "core.h"
#include "wbcompeldlight.h"
#include "wbcompeldtransform.h"
#include "wbevent.h"
#include "eldritchworld.h"
#include "idatastream.h"
#include "configmanager.h"

WBCompEldLight::WBCompEldLight()
:	m_Radius( 0.0f )
,	m_Color()
,	m_HasAddedLight( false )
,	m_LightLocation()
,	m_DeferAddLight( false )
{
}

WBCompEldLight::~WBCompEldLight()
{
	RemoveLight();
}

/*virtual*/ void WBCompEldLight::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Radius );
	m_Radius = ConfigManager::GetInheritedFloat( sRadius, 0.0f, sDefinitionName );

	STATICHASH( ColorR );
	m_Color.r = ConfigManager::GetInheritedFloat( sColorR, 0.0f, sDefinitionName );

	STATICHASH( ColorG );
	m_Color.g = ConfigManager::GetInheritedFloat( sColorG, 0.0f, sDefinitionName );

	STATICHASH( ColorB );
	m_Color.b = ConfigManager::GetInheritedFloat( sColorB, 0.0f, sDefinitionName );

	m_Color.a = 1.0f;

	STATICHASH( DeferAddLight );
	m_DeferAddLight = ConfigManager::GetInheritedBool( sDeferAddLight, false, sDefinitionName );
}

/*virtual*/ void WBCompEldLight::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnMoved );
	STATIC_HASHED_STRING( OnDestroyed );
	STATIC_HASHED_STRING( AddLight );
	STATIC_HASHED_STRING( RemoveLight );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnMoved )
	{
		if( m_DeferAddLight )
		{
			// Do nothing
		}
		else
		{
			AddLight();
		}
	}
	else if( EventName == sAddLight )
	{
		AddLight();
	}
	else if( EventName == sOnDestroyed || EventName == sRemoveLight )
	{
		RemoveLight();
	}
}

void WBCompEldLight::AddLight()
{
	// I don't currently want to support dynamic light-emitting entities.
	// So this event handles the first time a static entity's location is set.
	// But if it moves again, that's a problem!
	if( m_HasAddedLight )
	{
		return;
	}

	WBCompEldTransform* pTransform = GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	m_LightLocation = pTransform->GetLocation();
	m_HasAddedLight = GetWorld()->AddLightAt( m_LightLocation, m_Radius, m_Color );
}

void WBCompEldLight::RemoveLight()
{
	if( m_HasAddedLight )
	{
		m_HasAddedLight = false;

		if( GetWorld() )
		{
			GetWorld()->RemoveLightAt( m_LightLocation );
		}
	}
}

#define VERSION_EMPTY	0
#define VERSION_LIGHT	1
#define VERSION_CURRENT	1

uint WBCompEldLight::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += sizeof( bool );		// m_HasAddedLight
	Size += sizeof( Vector );	// m_LightLocation

	return Size;
}

void WBCompEldLight::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.WriteBool( m_HasAddedLight );
	Stream.Write( sizeof( Vector ), &m_LightLocation );
}

void WBCompEldLight::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_LIGHT )
	{
		m_HasAddedLight = Stream.ReadBool();
		Stream.Read( sizeof( Vector ), &m_LightLocation );
	}
}