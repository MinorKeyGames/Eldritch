#include "core.h"
#include "wbcompvariablemap.h"
#include "wbparamevaluator.h"
#include "reversehash.h"
#include "idatastream.h"

WBCompVariableMap::WBCompVariableMap()
:	m_Variables()
{
}

WBCompVariableMap::~WBCompVariableMap()
{
}

/*virtual*/ void WBCompVariableMap::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( SetVariable );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sSetVariable )
	{
		STATIC_HASHED_STRING( Name );
		const HashedString Name = Event.GetHash( sName );

		STATIC_HASHED_STRING( Value );
		const WBEvent::SParameter* const pParameter = Event.GetParameter( sValue );
		m_Variables.Set( Name, pParameter );
	}
}

void WBCompVariableMap::Report() const
{
	Super::Report();

	const WBEvent::TParameterMap& ParameterMap = m_Variables.GetParameters();
	PRINTF( WBPROPERTY_REPORT_PREFIX "Num Variables: %d\n", ParameterMap.Size() );
	FOR_EACH_MAP( ParamIter, ParameterMap, HashedString, WBEvent::SParameter )
	{
		const HashedString&			NameHash	= ParamIter.GetKey();
		const WBEvent::SParameter&	Parameter	= ParamIter.GetValue();

		const SimpleString			Name		= ReverseHash::ReversedHash( NameHash );
		const SimpleString			Value		= Parameter.CoerceString();

		PRINTF( WBPROPERTY_REPORT_PREFIX WB_REPORT_SPACER "%s:\t%s\n", Name.CStr(), Value.CStr() );
	}
}

#define VERSION_EMPTY		0
#define VERSION_VARIABLES	1
#define VERSION_CURRENT		1

uint WBCompVariableMap::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;									// Version
	Size += m_Variables.GetSerializationSize();

	return Size;
}

void WBCompVariableMap::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	m_Variables.Save( Stream );
}

void WBCompVariableMap::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_VARIABLES )
	{
		m_Variables.Load( Stream );
	}
}