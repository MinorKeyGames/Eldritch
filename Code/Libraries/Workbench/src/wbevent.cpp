#include "core.h"
#include "wbevent.h"
#include "memorystream.h"
#include "wbparamevaluator.h"
#include "reversehash.h"
#include "idatastream.h"

WBEvent WBPackedEvent::Unpack() const
{
	WBEvent UnpackedEvent;
	UnpackedEvent.Unpack( *this );
	return UnpackedEvent;
}

WBEvent::WBEvent()
:	m_Parameters()
{
}

WBEvent::~WBEvent()
{
}

bool WBEvent::SParameter::CoerceBool() const
{
	return m_Type == EWBEPT_Bool ? GetBool() : false;
}

int WBEvent::SParameter::CoerceInt() const
{
	if( m_Type == EWBEPT_Int )
	{
		return GetInt();
	}
	else if( m_Type == EWBEPT_Float )
	{
		return static_cast<int>( GetFloat() );
	}
	else
	{
		return 0;
	}
}

float WBEvent::SParameter::CoerceFloat() const
{
	if( m_Type == EWBEPT_Float )
	{
		return GetFloat();
	}
	else if( m_Type == EWBEPT_Int )
	{
		return static_cast<float>( GetInt() );
	}
	else
	{
		return 0.0f;
	}
}

HashedString WBEvent::SParameter::CoerceHash() const
{
	return m_Type == EWBEPT_Hash ? GetHash() : HashedString();
}

Vector WBEvent::SParameter::CoerceVector() const
{
	return m_Type == EWBEPT_Vector ? GetVector() : Vector();
}

Angles WBEvent::SParameter::CoerceAngles() const
{
	return m_Type == EWBEPT_Angles ? GetAngles() : Angles();
}

WBEntity* WBEvent::SParameter::CoerceEntity() const
{
	return m_Type == EWBEPT_Entity ? GetEntity().Get() : NULL;
}

SimpleString WBEvent::SParameter::CoerceString() const
{
	switch( m_Type )
	{
	case EWBEPT_None:		return "None";
	case EWBEPT_Bool:		return GetBool() ? "True" : "False";
	case EWBEPT_Int:		return SimpleString::PrintF( "%d", GetInt() );
	case EWBEPT_Float:		return SimpleString::PrintF( "%f", GetFloat() );
	case EWBEPT_Hash:		return SimpleString::PrintF( "%s", ReverseHash::ReversedHash( GetHash() ).CStr() );
	case EWBEPT_Vector:		return GetVector().GetString();
	case EWBEPT_Angles:		return GetAngles().GetString();
	case EWBEPT_Entity:		{ WBEntity* const pEntity = GetEntity().Get(); return pEntity ? pEntity->GetUniqueName() : "NULL"; }
	case EWBEPT_Pointer:	return SimpleString::PrintF( "Ptr: 0x%08X", GetPointer() );
	default:				return "Unknown";
	}
}

void* WBEvent::SParameter::CoercePointer() const
{
	return m_Type == EWBEPT_Pointer ? GetPointer() : NULL;
}

void WBEvent::Set( const HashedString& Name, const SParameter* const pParameter )
{
	if( !pParameter )
	{
		return;
	}

	switch( pParameter->GetType() )
	{
	case EWBEPT_Bool:		SetBool(	Name, pParameter->GetBool() );		break;
	case EWBEPT_Int:		SetInt(		Name, pParameter->GetInt() );		break;
	case EWBEPT_Float:		SetFloat(	Name, pParameter->GetFloat() );		break;
	case EWBEPT_Hash:		SetHash(	Name, pParameter->GetHash() );		break;
	case EWBEPT_Vector:		SetVector(	Name, pParameter->GetVector() );	break;
	case EWBEPT_Angles:		SetAngles(	Name, pParameter->GetAngles() );	break;
	case EWBEPT_Entity:		SetEntity(	Name, pParameter->GetEntity() );	break;
	case EWBEPT_Pointer:	SetPointer(	Name, pParameter->GetPointer() );	break;
	}
}

void WBEvent::Set( const HashedString& Name, const WBParamEvaluator& PE )
{
	switch( PE.GetType() )
	{
	case WBParamEvaluator::EPT_Bool:	SetBool(	Name, PE.GetBool() );	break;
	case WBParamEvaluator::EPT_Int:		SetInt(		Name, PE.GetInt() );	break;
	case WBParamEvaluator::EPT_Float:	SetFloat(	Name, PE.GetFloat() );	break;
	case WBParamEvaluator::EPT_String:	SetHash(	Name, PE.GetString() ); break;	// Conversion to hash is the best we can do
	case WBParamEvaluator::EPT_Entity:	SetEntity(	Name, PE.GetEntity() );	break;
	case WBParamEvaluator::EPT_Vector:	SetVector(	Name, PE.GetVector() );	break;
	}
}

void WBEvent::SetEventName( const HashedString& Value )
{
	STATIC_HASHED_STRING( EventName );
	return SetHash( sEventName, Value );
}

HashedString WBEvent::GetEventName() const
{
	STATIC_HASHED_STRING( EventName );
	return GetHash( sEventName );
}

const WBEvent::SParameter* WBEvent::GetParameter( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? &ParamIter.GetValue() : NULL;
}

WBEvent::EType WBEvent::GetType( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().GetType() : EWBEPT_None;
}

bool WBEvent::GetBool( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().CoerceBool() : false;
}

int WBEvent::GetInt( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().CoerceInt() : 0;
}

float WBEvent::GetFloat( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().CoerceFloat() : 0.0f;
}

HashedString WBEvent::GetHash( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().CoerceHash() : HashedString();
}

Vector WBEvent::GetVector( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().CoerceVector() : Vector();
}

Angles WBEvent::GetAngles( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().CoerceAngles() : Angles();
}

WBEntity* WBEvent::GetEntity( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().CoerceEntity() : NULL;
}

SimpleString WBEvent::GetString( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().CoerceString() : "";
}

void* WBEvent::GetPointer( const HashedString& Name ) const
{
	TParameterMap::Iterator ParamIter = m_Parameters.Search( Name );
	return ParamIter.IsValid() ? ParamIter.GetValue().CoercePointer() : NULL;
}

WBPackedEvent WBEvent::Pack() const
{
	WBPackedEvent PackedEvent;
	Pack( PackedEvent );
	return PackedEvent;
}

void WBEvent::Pack( WBPackedEvent& PackedEvent ) const
{
	PackedEvent.m_PackedEvent.Resize( 4 + ( m_Parameters.Size() * ( sizeof( HashedString ) + sizeof( SParameter ) ) ) );

	MemoryStream Stream( PackedEvent.GetData(), PackedEvent.GetSize() );
	Stream.WriteUInt32( m_Parameters.Size() );
	FOR_EACH_MAP( ParameterIter, m_Parameters, HashedString, SParameter )
	{
		const HashedString& ParameterName = ParameterIter.GetKey();
		const SParameter& Parameter = ParameterIter.GetValue();

		Stream.WriteHashedString( ParameterName );
		Stream.Write( sizeof( SParameter ), &Parameter );
	}
}

void WBEvent::Unpack( const WBPackedEvent& PackedEvent )
{
	m_Parameters.Clear();

	MemoryStream Stream( PackedEvent.GetData(), PackedEvent.GetSize() );

	uint NumParameters = Stream.ReadUInt32();
	for( uint ParameterIndex = 0; ParameterIndex < NumParameters; ++ParameterIndex )
	{
		const HashedString ParameterName = Stream.ReadHashedString();
		SParameter& Parameter = m_Parameters[ ParameterName ];

		Stream.Read( sizeof( SParameter ), &Parameter );
	}
}

uint WBEvent::GetSerializationSize() const
{
	WBPackedEvent PackedEvent;
	Pack( PackedEvent );

	return PackedEvent.GetSize() + 4;
}

void WBEvent::Save( const IDataStream& Stream ) const
{
	WBPackedEvent PackedEvent;
	Pack( PackedEvent );

	Stream.WriteUInt32( PackedEvent.GetSize() );
	Stream.Write( PackedEvent.GetSize(), PackedEvent.GetData() );
}

void WBEvent::Load( const IDataStream& Stream )
{
	const uint PackedEventSize = Stream.ReadUInt32();

	WBPackedEvent PackedEvent;
	PackedEvent.Reinit( NULL, PackedEventSize );
	Stream.Read( PackedEventSize, PackedEvent.GetData() );

	Unpack( PackedEvent );
}