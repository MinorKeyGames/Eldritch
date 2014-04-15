#include "core.h"
#include "wbcomptransform.h"
#include "configmanager.h"
#include "mathcore.h"
#include "idatastream.h"

WBCompTransform::WBCompTransform()
:	m_Location()
,	m_Velocity()
{
}

WBCompTransform::~WBCompTransform()
{
}

void WBCompTransform::ServerTick( float DeltaTime )
{
	if( m_Velocity.LengthSquared() > EPSILON )	// Optimization, don't want to pay MoveBy costs for a ton of static entities
	{
		MoveBy( m_Velocity * DeltaTime );
	}
}

void WBCompTransform::SetLocation( const Vector& NewLocation )
{
	m_Location = NewLocation;
}

void WBCompTransform::MoveBy( const Vector& Offset )
{
	SetLocation( m_Location + Offset );
}

uint WBCompTransform::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += sizeof( Vector );	// m_Location
	Size += sizeof( Vector );	// m_Velocity

	return Size;
}

#define VERSION_EMPTY				0
#define VERSION_LOCATION			1
#define VERSION_VELOCITY			2
#define VERSION_CURRENT				2

void WBCompTransform::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.Write( sizeof( Vector ), &m_Location );
	Stream.Write( sizeof( Vector ), &m_Velocity );
}

void WBCompTransform::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_LOCATION )
	{
		Stream.Read( sizeof( Vector ), &m_Location );
	}

	if( Version >= VERSION_VELOCITY )
	{
		Stream.Read( sizeof( Vector ), &m_Velocity );
	}
}

void WBCompTransform::Report() const
{
	Super::Report();

	PRINTF( WBPROPERTY_REPORT_PREFIX "Location: %s\n", m_Location.GetString().CStr() );
	PRINTF( WBPROPERTY_REPORT_PREFIX "Velocity: %s\n", m_Velocity.GetString().CStr() );
}