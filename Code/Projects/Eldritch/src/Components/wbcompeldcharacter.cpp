#include "core.h"
#include "wbcompeldcharacter.h"
#include "configmanager.h"
#include "idatastream.h"
#include "wbeventmanager.h"
#include "wbcompeldhands.h"
#include "eldritchgame.h"
#include "eldritchpersistence.h"
#include "Screens/uiscreen-eldmirror.h"

WBCompEldCharacter::WBCompEldCharacter()
:	m_HeadOptions()
,	m_BodyOptions()
,	m_CurrentHeadIndex( 0 )
,	m_CurrentBodyIndex( 0 )
{
}

WBCompEldCharacter::~WBCompEldCharacter()
{
}

/*virtual*/ void WBCompEldCharacter::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( NumHeadOptions );
	const uint NumHeadOptions = ConfigManager::GetInheritedInt( sNumHeadOptions, 0, sDefinitionName );
	for( uint HeadOptionIndex = 0; HeadOptionIndex < NumHeadOptions; ++HeadOptionIndex )
	{
		SHeadOption& HeadOption = m_HeadOptions.PushBack();

		const SimpleString HeadOptionDef = ConfigManager::GetInheritedSequenceString( "HeadOption%d", HeadOptionIndex, "", sDefinitionName );

		MAKEHASH( HeadOptionDef	);

		STATICHASH( Mesh );
		HeadOption.m_Mesh = ConfigManager::GetString( sMesh, "", sHeadOptionDef );

		STATICHASH( Texture );
		HeadOption.m_Texture = ConfigManager::GetString( sTexture, "", sHeadOptionDef );

		STATICHASH( VoiceSet );
		HeadOption.m_VoiceSet = ConfigManager::GetString( sVoiceSet, "", sHeadOptionDef );
	}

	STATICHASH( NumBodyOptions );
	const uint NumBodyOptions = ConfigManager::GetInheritedInt( sNumBodyOptions, 0, sDefinitionName );
	for( uint BodyOptionIndex = 0; BodyOptionIndex < NumBodyOptions; ++BodyOptionIndex )
	{
		SBodyOption& BodyOption = m_BodyOptions.PushBack();

		const SimpleString BodyOptionDef = ConfigManager::GetInheritedSequenceString( "BodyOption%d", BodyOptionIndex, "", sDefinitionName );

		MAKEHASH( BodyOptionDef	);

		STATICHASH( Mesh );
		BodyOption.m_Mesh = ConfigManager::GetString( sMesh, "", sBodyOptionDef );

		STATICHASH( Texture );
		BodyOption.m_Texture = ConfigManager::GetString( sTexture, "", sBodyOptionDef );

		STATICHASH( LeftHandMesh );
		BodyOption.m_LeftHandMesh = ConfigManager::GetString( sLeftHandMesh, "", sBodyOptionDef );

		STATICHASH( LeftHandTexture );
		BodyOption.m_LeftHandTexture = ConfigManager::GetString( sLeftHandTexture, "", sBodyOptionDef );

		STATICHASH( RightHandMesh );
		BodyOption.m_RightHandMesh = ConfigManager::GetString( sRightHandMesh, "", sBodyOptionDef );

		STATICHASH( RightHandTexture );
		BodyOption.m_RightHandTexture = ConfigManager::GetString( sRightHandTexture, "", sBodyOptionDef );
	}
}

/*virtual*/ void WBCompEldCharacter::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnSpawned );
	STATIC_HASHED_STRING( OnLoaded );
	STATIC_HASHED_STRING( OnRespawnedInventory );
	STATIC_HASHED_STRING( SetPrevHead );
	STATIC_HASHED_STRING( SetNextHead );
	STATIC_HASHED_STRING( SetPrevBody );
	STATIC_HASHED_STRING( SetNextBody );
	STATIC_HASHED_STRING( PushPersistence );
	STATIC_HASHED_STRING( PullPersistence );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnSpawned )
	{
		EldritchPersistence* const pPersistence = GetGame()->GetPersistence();
		ASSERT( pPersistence );

		m_CurrentHeadIndex = pPersistence->GetCharacterHeadIndex();
		m_CurrentBodyIndex = pPersistence->GetCharacterBodyIndex();

		PushCharacterOptions();
	}
	else if( EventName == sOnLoaded )
	{
		// Everything in the world should already be serialized properly, but we need to push to UI screen.
		PushCharacterOptions();
	}
	else if( EventName == sOnRespawnedInventory )
	{
		// Push hand meshes onto newly spawned hands.
		PushCharacterOptions();
	}
	else if( EventName == sSetPrevHead )
	{
		ChangeHead( -1 );
	}
	else if( EventName == sSetNextHead )
	{
		ChangeHead( 1 );
	}
	else if( EventName == sSetPrevBody )
	{
		ChangeBody( -1 );
	}
	else if( EventName == sSetNextBody )
	{
		ChangeBody( 1 );
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

void WBCompEldCharacter::PushCharacterOptions() const
{
	const SHeadOption& CurrentHead = GetCurrentHeadOption();
	const SBodyOption& CurrentBody = GetCurrentBodyOption();

	// Update hands
	{
		WB_MAKE_EVENT( SetHandMeshes, GetEntity() );
		WB_SET_AUTO( SetHandMeshes, Hash, LeftHandMesh, CurrentBody.m_LeftHandMesh );
		WB_SET_AUTO( SetHandMeshes, Hash, LeftHandTexture, CurrentBody.m_LeftHandTexture );
		WB_SET_AUTO( SetHandMeshes, Hash, RightHandMesh, CurrentBody.m_RightHandMesh );
		WB_SET_AUTO( SetHandMeshes, Hash, RightHandTexture, CurrentBody.m_RightHandTexture );
		WB_DISPATCH_EVENT( GetEventManager(), SetHandMeshes, GetEntity() );
	}

	// Update head in mirror
	{
		WB_MAKE_EVENT( SetHeadMesh, GetEntity() );
		WB_SET_AUTO( SetHeadMesh, Hash, HeadMesh, CurrentHead.m_Mesh );
		WB_SET_AUTO( SetHeadMesh, Hash, HeadTexture, CurrentHead.m_Texture );
		WB_DISPATCH_EVENT( GetEventManager(), SetHeadMesh, EldritchGame::GetMirrorScreen() );
	}

	// Update body in mirror
	{
		WB_MAKE_EVENT( SetBodyMesh, GetEntity() );
		WB_SET_AUTO( SetBodyMesh, Hash, BodyMesh, CurrentBody.m_Mesh );
		WB_SET_AUTO( SetBodyMesh, Hash, BodyTexture, CurrentBody.m_Texture );
		WB_DISPATCH_EVENT( GetEventManager(), SetBodyMesh, EldritchGame::GetMirrorScreen() );
	}

	// Update character persistence
	{
		EldritchPersistence* const pPersistence = GetGame()->GetPersistence();
		ASSERT( pPersistence );
		pPersistence->SetCharacterHeadIndex( m_CurrentHeadIndex );
		pPersistence->SetCharacterBodyIndex( m_CurrentBodyIndex );
	}
}

void WBCompEldCharacter::ChangeHead( int IndexStep )
{
	const uint NumHeadOptions = m_HeadOptions.Size();
	m_CurrentHeadIndex = ( m_CurrentHeadIndex + NumHeadOptions + IndexStep ) % NumHeadOptions;

	PushCharacterOptions();
}

void WBCompEldCharacter::ChangeBody( int IndexStep )
{
	const uint NumBodyOptions = m_BodyOptions.Size();
	m_CurrentBodyIndex = ( m_CurrentBodyIndex + NumBodyOptions + IndexStep ) % NumBodyOptions;

	PushCharacterOptions();
}

void WBCompEldCharacter::PushPersistence() const
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( HeadIndex );
	Persistence.SetInt( sHeadIndex, m_CurrentHeadIndex );

	STATIC_HASHED_STRING( BodyIndex );
	Persistence.SetInt( sBodyIndex, m_CurrentBodyIndex );
}

void WBCompEldCharacter::PullPersistence()
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( HeadIndex );
	m_CurrentHeadIndex = Persistence.GetInt( sHeadIndex );

	STATIC_HASHED_STRING( BodyIndex );
	m_CurrentBodyIndex = Persistence.GetInt( sBodyIndex );

	PushCharacterOptions();
}

#define VERSION_EMPTY			0
#define VERSION_CURRENTOPTIONS	1
#define VERSION_CURRENT			1

uint WBCompEldCharacter::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;	// Version
	Size += 4;	// m_CurrentHeadIndex
	Size += 4;	// m_CurrentBodyIndex

	return Size;
}

void WBCompEldCharacter::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_CurrentHeadIndex );
	Stream.WriteUInt32( m_CurrentBodyIndex );
}

void WBCompEldCharacter::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_CURRENTOPTIONS )
	{
		m_CurrentHeadIndex = Stream.ReadUInt32();
		m_CurrentBodyIndex = Stream.ReadUInt32();

		// Reset indices if the configured options have changed to invalidate them.

		if( m_CurrentHeadIndex >= m_HeadOptions.Size() )
		{
			m_CurrentHeadIndex = 0;
		}

		if( m_CurrentBodyIndex >= m_BodyOptions.Size() )
		{
			m_CurrentBodyIndex = 0;
		}
	}
}