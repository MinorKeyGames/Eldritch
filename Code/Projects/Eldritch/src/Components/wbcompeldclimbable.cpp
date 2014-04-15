#include "core.h"
#include "wbcompeldclimbable.h"
#include "wbcompeldtransform.h"
#include "wbentity.h"
#include "configmanager.h"
#include "wbeventmanager.h"
#include "idatastream.h"
#include "matrix.h"

WBCompEldClimbable::WBCompEldClimbable()
:	m_UseSnapPlane( false )
,	m_SnapPlane()
{
}

WBCompEldClimbable::~WBCompEldClimbable()
{
}

/*virtual*/ void WBCompEldClimbable::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( UseSnapPlane );
	m_UseSnapPlane = ConfigManager::GetInheritedBool( sUseSnapPlane, false, sDefinitionName );
}

/*virtual*/ void WBCompEldClimbable::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnInitialOrientationSet );
	STATIC_HASHED_STRING( OnTouched );
	STATIC_HASHED_STRING( OnUntouched );
	
	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnInitialOrientationSet )
	{
		if( m_UseSnapPlane )
		{
			InitializeSnapPlane();
		}
	}
	else if( EventName == sOnTouched )
	{
		STATIC_HASHED_STRING( Touched );
		WBEntity* const pTouched = Event.GetEntity( sTouched );

		WB_MAKE_EVENT( OnTouchedClimbable, GetEntity() );
		WB_QUEUE_EVENT( GetEventManager(), OnTouchedClimbable, pTouched );
	}
	else if( EventName == sOnUntouched )
	{
		STATIC_HASHED_STRING( Untouched );
		WBEntity* const pUntouched = Event.GetEntity( sUntouched );

		WB_MAKE_EVENT( OnUntouchedClimbable, GetEntity() );
		WB_QUEUE_EVENT( GetEventManager(), OnUntouchedClimbable, pUntouched );
	}
}

/*virtual*/ void WBCompEldClimbable::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	WB_SET_CONTEXT( Event, Bool, UseSnapPlane, m_UseSnapPlane );
	WB_SET_CONTEXT( Event, Float, SnapPlaneDistance, m_SnapPlane.m_Distance );
	WB_SET_CONTEXT( Event, Vector, SnapPlaneNormal, m_SnapPlane.m_Normal );
}

void WBCompEldClimbable::InitializeSnapPlane()
{
	WBEntity* const				pEntity				= GetEntity();
	WBCompEldTransform* const	pTransform			= pEntity->GetTransformComponent<WBCompEldTransform>();
	const Vector				Location			= pTransform->GetLocation();
	const Angles				Orientation			= pTransform->GetOrientation();
	const Matrix				OrientationMatrix	= Orientation.ToMatrix();
	const Vector				Right				= Vector( 1.0f, 0.0f, 0.0f );
	const Vector				OrientedRight		= Right * OrientationMatrix;

	m_SnapPlane = Plane( OrientedRight, Location );
}

#define VERSION_EMPTY	0
#define VERSION_SNAPPLANE	1
#define VERSION_CURRENT		1

uint WBCompEldClimbable::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += sizeof( Plane );	// m_SnapPlane

	return Size;
}

void WBCompEldClimbable::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.Write( sizeof( Plane ), &m_SnapPlane );
}

void WBCompEldClimbable::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_SNAPPLANE )
	{
		Stream.Read( sizeof( Plane ), &m_SnapPlane );
	}
}