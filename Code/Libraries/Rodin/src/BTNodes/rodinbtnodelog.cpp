#include "core.h"
#include "rodinbtnodelog.h"
#include "configmanager.h"
#include "wbentity.h"

RodinBTNodeLog::RodinBTNodeLog()
:	m_Message()
{
}

RodinBTNodeLog::~RodinBTNodeLog()
{
}

void RodinBTNodeLog::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Message );
	m_Message = ConfigManager::GetString( sMessage, "", sDefinitionName );
}

RodinBTNode::ETickStatus RodinBTNodeLog::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	PRINTF( "BT Log (%.2f): %s (%s)\n", GetTime(), m_Message.CStr(), GetEntity()->GetUniqueName().CStr() );

	return ETS_Success;
}