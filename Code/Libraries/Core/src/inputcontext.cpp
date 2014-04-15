#include "core.h"
#include "inputcontext.h"
#include "simplestring.h"
#include "configmanager.h"
#include "inputsystem.h"

InputContext::InputContext()
:	m_SuppressUnredirectedInputs( false )
,	m_InputRedirects()
{
}

InputContext::~InputContext()
{
}

void InputContext::Initialize( const SimpleString& DefinitionName, InputSystem* const pInputSystem )
{
	ASSERT( pInputSystem );

	// Use "InputContext_DummyInput" instead of NullString as the default, so that we can validly redirect to a null input.
	STATIC_HASHED_STRING( InputContext_DummyInput );

	MAKEHASH( DefinitionName );

	STATICHASH( Suppress );
	m_SuppressUnredirectedInputs = ConfigManager::GetInheritedBool( sSuppress, false, sDefinitionName );

	const uint NumInputs = pInputSystem->m_Inputs.Size();
	for( uint InputIndex = 0; InputIndex < NumInputs; ++InputIndex )
	{
		const InputSystem::SInput& Input = pInputSystem->m_Inputs[ InputIndex ];
		const HashedString RedirectedInput = ConfigManager::GetInheritedHash( Input.m_String, sInputContext_DummyInput, sDefinitionName );
		if( RedirectedInput != sInputContext_DummyInput )
		{
			m_InputRedirects[ Input.m_Hash ] = RedirectedInput;
		}
	}

	const uint NumAnalogInputs = pInputSystem->m_AnalogInputs.Size();
	for( uint AnalogInputIndex = 0; AnalogInputIndex < NumAnalogInputs; ++AnalogInputIndex )
	{
		const InputSystem::SAnalogInput& AnalogInput = pInputSystem->m_AnalogInputs[ AnalogInputIndex ];
		const HashedString RedirectedInput = ConfigManager::GetInheritedHash( AnalogInput.m_String, sInputContext_DummyInput, sDefinitionName );
		if( RedirectedInput != sInputContext_DummyInput )
		{
			m_InputRedirects[ AnalogInput.m_Hash ] = RedirectedInput;
		}
	}
}

bool InputContext::HasRedirect( const HashedString& Input ) const
{
	return m_InputRedirects.Search( Input ).IsValid();
}

HashedString InputContext::GetRedirect( const HashedString& Input ) const
{
	ASSERT( m_InputRedirects.Search( Input ).IsValid() );
	return m_InputRedirects[ Input ];
}