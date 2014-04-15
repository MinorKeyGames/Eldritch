#include "core.h"
#include "wbcompeldvisible.h"
#include "wbcompeldtransform.h"
#include "wbcompeldcamera.h"
#include "wbentity.h"
#include "configmanager.h"
#include "wbevent.h"
#include "idatastream.h"

WBCompEldVisible::WBCompEldVisible()
:	m_Visible( false )
{
}

WBCompEldVisible::~WBCompEldVisible()
{
}

/*virtual*/ void WBCompEldVisible::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Visible );
	m_Visible = ConfigManager::GetInheritedBool( sVisible, true, sDefinitionName );
}

/*virtual*/ void WBCompEldVisible::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( SetVisible );
	STATIC_HASHED_STRING( SetInvisible );
	
	const HashedString EventName = Event.GetEventName();
	if( EventName == sSetVisible )
	{
		m_Visible = true;
	}
	else if( EventName == sSetInvisible )
	{
		m_Visible = false;
	}
}

/*virtual*/ void WBCompEldVisible::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Bool, Visible, m_Visible );
}

// This is a bit hackity. Maybe consider a better way, but I'm deep in other AI stuff now.
Vector WBCompEldVisible::GetVisibleLocation() const
{
	WBEntity* const pEntity = GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	Vector VisibleLocation = pTransform->GetLocation();

	WBCompEldCamera* const pCamera = GET_WBCOMP( pEntity, EldCamera );
	if( pCamera )
	{
		// *Don't* trace to lean location.
		VisibleLocation += pCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_OffsetZ );
	}

	return VisibleLocation;
}

#define VERSION_EMPTY	0
#define VERSION_VISIBLE	1
#define VERSION_CURRENT	1

uint WBCompEldVisible::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += 1;					// m_Visible

	return Size;
}

void WBCompEldVisible::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteBool( m_Visible );
}

void WBCompEldVisible::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_VISIBLE )
	{
		m_Visible = Stream.ReadBool();
	}
}