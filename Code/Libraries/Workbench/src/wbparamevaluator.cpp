#include "core.h"
#include "wbparamevaluator.h"
#include "simplestring.h"
#include "wbpe.h"
#include "wbparamevaluatorfactory.h"
#include "reversehash.h"

WBParamEvaluator::SEvaluatedParam::SEvaluatedParam()
:	m_Type( EPT_None )
,	m_Bool( false )
,	m_Int( 0 )
,	m_Float( 0.0f )
,	m_String( "" )
,	m_Entity()
,	m_Vector()
{
}

WBParamEvaluator::SPEContext::SPEContext()
:	m_Entity( NULL )
{
}

WBParamEvaluator::WBParamEvaluator()
:	m_RootEvaluator( NULL )
{
}

WBParamEvaluator::~WBParamEvaluator()
{
	SafeDelete( m_RootEvaluator );
}

void WBParamEvaluator::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	if( DefinitionName == "" )
	{
		// Gracefully handle undefined PEs (used for pattern matching without a max, for instance).
		return;
	}

	// Just forward this definition to the root, we don't actually need to configure anything here
	m_RootEvaluator = WBParamEvaluatorFactory::Create( DefinitionName );
}

void WBParamEvaluator::Evaluate( const SPEContext& Context )
{
	if( m_RootEvaluator )
	{
		m_RootEvaluator->Evaluate( Context, m_EvaluatedParam );
	}
}

bool WBParamEvaluator::SEvaluatedParam::GetBool() const
{
	switch( m_Type )
	{
	case WBParamEvaluator::EPT_Bool:	return m_Bool;
	case WBParamEvaluator::EPT_Int:		return ( m_Int != 0 );
	case WBParamEvaluator::EPT_Float:	return ( m_Float != 0.0f );
	case WBParamEvaluator::EPT_Entity:	return ( m_Entity.Get() != NULL );
	case WBParamEvaluator::EPT_String:	return ( m_String != "" );
	default:							return false;
	}
}

int WBParamEvaluator::SEvaluatedParam::GetInt() const
{
	return m_Type == WBParamEvaluator::EPT_Int ? m_Int : m_Type == WBParamEvaluator::EPT_Float ? static_cast<int>( m_Float ) : 0;
}

float WBParamEvaluator::SEvaluatedParam::GetFloat() const
{
	return m_Type == WBParamEvaluator::EPT_Float ? m_Float : m_Type == WBParamEvaluator::EPT_Int ? static_cast<float>( m_Int ) : 0.0f;
}

SimpleString WBParamEvaluator::SEvaluatedParam::GetString() const
{
	return m_Type == WBParamEvaluator::EPT_String ? m_String : SimpleString( "" );
}

WBEntity* WBParamEvaluator::SEvaluatedParam::GetEntity() const
{
	return m_Type == WBParamEvaluator::EPT_Entity ? m_Entity.Get() : NULL;
}

Vector WBParamEvaluator::SEvaluatedParam::GetVector() const
{
	return m_Type == WBParamEvaluator::EPT_Vector ? m_Vector : Vector();
}

void WBParamEvaluator::SEvaluatedParam::Set( const WBEvent::SParameter* const pParameter )
{
	if( !pParameter )
	{
		return;
	}

	switch( pParameter->GetType() )
	{
	case WBEvent::EWBEPT_Bool:		m_Type = EPT_Bool;		m_Bool		= pParameter->GetBool();								break;
	case WBEvent::EWBEPT_Int:		m_Type = EPT_Int;		m_Int		= pParameter->GetInt();									break;
	case WBEvent::EWBEPT_Float:		m_Type = EPT_Float;		m_Float		= pParameter->GetFloat();								break;
	case WBEvent::EWBEPT_Hash:		m_Type = EPT_String;	m_String	= ReverseHash::ReversedHash( pParameter->GetHash() );	break;
	case WBEvent::EWBEPT_Vector:	m_Type = EPT_Vector;	m_Vector	= pParameter->GetVector();								break;
	case WBEvent::EWBEPT_Angles:	WARN;																						break;
	case WBEvent::EWBEPT_Entity:	m_Type = EPT_Entity;	m_Entity	= pParameter->GetEntity();								break;
	}
}