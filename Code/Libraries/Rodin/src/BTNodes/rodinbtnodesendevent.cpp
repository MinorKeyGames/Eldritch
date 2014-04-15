#include "core.h"
#include "rodinbtnodesendevent.h"
#include "configmanager.h"
#include "wbeventmanager.h"

RodinBTNodeSendEvent::RodinBTNodeSendEvent()
:	m_EventName()
,	m_QueueEvent( false )
,	m_DispatchDelay( 0.0f )
,	m_RecipientPE()
,	m_Parameters()
{
}

RodinBTNodeSendEvent::~RodinBTNodeSendEvent()
{
}

void RodinBTNodeSendEvent::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( EventName );
	m_EventName = ConfigManager::GetHash( sEventName, "", sDefinitionName );

	STATICHASH( QueueEvent );
	m_QueueEvent = ConfigManager::GetBool( sQueueEvent, false, sDefinitionName );

	STATICHASH( DispatchDelay );
	m_DispatchDelay = ConfigManager::GetFloat( sDispatchDelay, 0.0f, sDefinitionName );

	STATICHASH( Recipient );
	const SimpleString RecipientDef = ConfigManager::GetString( sRecipient, "", sDefinitionName );
	m_RecipientPE.InitializeFromDefinition( RecipientDef );

	STATICHASH( NumParameters );
	const uint NumParameters = ConfigManager::GetInt( sNumParameters, 0, sDefinitionName );
	for( uint ParameterIndex = 0; ParameterIndex < NumParameters; ++ParameterIndex )
	{
		const HashedString Name = ConfigManager::GetSequenceHash( "Parameter%dName", ParameterIndex, HashedString::NullString, sDefinitionName );
		const SimpleString ValuePEDef = ConfigManager::GetSequenceString( "Parameter%dValue", ParameterIndex, "", sDefinitionName );

		SNamedParameter& Parameter = m_Parameters.PushBack();

		Parameter.m_Name = Name;
		Parameter.m_ValuePE.InitializeFromDefinition( ValuePEDef );
	}
}

RodinBTNode::ETickStatus RodinBTNodeSendEvent::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	WBEntity* const pEntity = GetEntity();

	WBEvent Event;

	Event.SetEventName( m_EventName );

	if( pEntity )
	{
		pEntity->AddContextToEvent( Event );
	}

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = pEntity;

	const uint NumParameters = m_Parameters.Size();
	for( uint ParameterIndex = 0; ParameterIndex < NumParameters; ++ParameterIndex )
	{
		SNamedParameter& Parameter = m_Parameters[ ParameterIndex ];
		Parameter.m_ValuePE.Evaluate( PEContext );
		Event.Set( Parameter.m_Name, Parameter.m_ValuePE );
	}

	m_RecipientPE.Evaluate( PEContext );

	if( m_QueueEvent )
	{
		WBWorld::GetInstance()->GetEventManager()->QueueEvent( Event, m_RecipientPE.GetEntity(), m_DispatchDelay );
	}
	else
	{
		WBWorld::GetInstance()->GetEventManager()->DispatchEvent( Event, m_RecipientPE.GetEntity() );
	}

	return ETS_Success;
}