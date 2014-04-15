#include "core.h"
#include "wbactioneldplayanim.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "wbeventmanager.h"

WBActionEldPlayAnim::WBActionEldPlayAnim()
:	m_AnimationName( "" )
,	m_Loop( false )
,	m_PlayRatePE()
{
}

WBActionEldPlayAnim::~WBActionEldPlayAnim()
{
}

/*virtual*/ void WBActionEldPlayAnim::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Animation );
	m_AnimationName = ConfigManager::GetHash( sAnimation, HashedString::NullString, sDefinitionName );

	STATICHASH( Loop );
	m_Loop = ConfigManager::GetBool( sLoop, false, sDefinitionName );

	STATICHASH( PlayRatePE );
	SimpleString PlayRateDef = ConfigManager::GetString( sPlayRatePE, "", sDefinitionName );
	m_PlayRatePE.InitializeFromDefinition( PlayRateDef );
}

/*virtual*/ void WBActionEldPlayAnim::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const pEntity = WBActionStack::Top().GetEntity( sEventOwner );

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = pEntity;
	m_PlayRatePE.Evaluate( PEContext );

	if( pEntity )
	{
		WB_MAKE_EVENT( PlayAnim, pEntity );
		WB_SET_AUTO( PlayAnim, Hash, AnimationName, m_AnimationName );
		WB_SET_AUTO( PlayAnim, Bool, Loop, m_Loop );
		WB_SET_AUTO( PlayAnim, Float, PlayRate, m_PlayRatePE.GetFloat() );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), PlayAnim, pEntity );
	}
}