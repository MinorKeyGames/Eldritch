#include "core.h"
#include "rodinbtnodeusestatmod.h"
#include "configmanager.h"
#include "Components/wbcompstatmod.h"
#include "wbentity.h"

RodinBTNodeUseStatMod::RodinBTNodeUseStatMod()
:	m_StatModEvent()
{
}

RodinBTNodeUseStatMod::~RodinBTNodeUseStatMod()
{
}

void RodinBTNodeUseStatMod::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	RodinBTNodeDecorator::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( StatModEvent );
	m_StatModEvent = ConfigManager::GetHash( sStatModEvent, HashedString::NullString, sDefinitionName );
}

/*virtual*/ void RodinBTNodeUseStatMod::OnStart()
{
	RodinBTNodeDecorator::OnStart();

	WBCompStatMod* const pStatMod = GET_WBCOMP( GetEntity(), StatMod );
	ASSERT( pStatMod );

	pStatMod->TriggerEvent( m_StatModEvent );
}

/*virtual*/ void RodinBTNodeUseStatMod::OnFinish()
{
	RodinBTNodeDecorator::OnFinish();

	WBCompStatMod* const pStatMod = GET_WBCOMP( GetEntity(), StatMod );
	ASSERT( pStatMod );

	pStatMod->UnTriggerEvent( m_StatModEvent );
}