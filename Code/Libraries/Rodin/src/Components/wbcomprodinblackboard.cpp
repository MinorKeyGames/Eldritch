#include "core.h"
#include "wbcomprodinblackboard.h"
#include "wbparamevaluator.h"
#include "reversehash.h"
#include "idatastream.h"

WBCompRodinBlackboard::WBCompRodinBlackboard()
:	m_BlackboardEntries()
{
}

WBCompRodinBlackboard::~WBCompRodinBlackboard()
{
}

void WBCompRodinBlackboard::Report() const
{
	Super::Report();

	const WBEvent::TParameterMap& ParameterMap = m_BlackboardEntries.GetParameters();
	PRINTF( WBPROPERTY_REPORT_PREFIX "Num Entries: %d\n", ParameterMap.Size() );
	FOR_EACH_MAP( ParamIter, ParameterMap, HashedString, WBEvent::SParameter )
	{
		const HashedString&			NameHash	= ParamIter.GetKey();
		const WBEvent::SParameter&	Parameter	= ParamIter.GetValue();

		const SimpleString			Name		= ReverseHash::ReversedHash( NameHash );
		const SimpleString			Value		= Parameter.CoerceString();

		PRINTF( WBPROPERTY_REPORT_PREFIX WB_REPORT_SPACER "%s:\t%s\n", Name.CStr(), Value.CStr() );
	}
}

#define VERSION_EMPTY				0
#define VERSION_BLACKBOARDENTRIES	1
#define VERSION_CURRENT				1

uint WBCompRodinBlackboard::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;									// Version

	WBPackedEvent PackedEvent;
	m_BlackboardEntries.Pack( PackedEvent );

	Size += PackedEvent.GetSize();				// m_BlackboardEntries

	return Size;
}

void WBCompRodinBlackboard::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	WBPackedEvent PackedEvent;
	m_BlackboardEntries.Pack( PackedEvent );

	Stream.WriteUInt32( PackedEvent.GetSize() );
	Stream.Write( PackedEvent.GetSize(), PackedEvent.GetData() );
}

void WBCompRodinBlackboard::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_BLACKBOARDENTRIES )
	{
		const uint PackedEventSize = Stream.ReadUInt32();
		WBPackedEvent PackedEvent;
		PackedEvent.Reinit( NULL, PackedEventSize );
		Stream.Read( PackedEventSize, PackedEvent.GetData() );
		m_BlackboardEntries.Unpack( PackedEvent );
	}
}