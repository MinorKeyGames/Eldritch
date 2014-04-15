#include "core.h"
#include "rodinbtnodeeldplaybark.h"
#include "configmanager.h"
#include "Components/wbcomprodinbehaviortree.h"
#include "wbeventmanager.h"
#include "animation.h"

RodinBTNodeEldPlayBark::RodinBTNodeEldPlayBark()
:	m_SoundDef()
,	m_SoundDefPE()
,	m_Category()
{
}

RodinBTNodeEldPlayBark::~RodinBTNodeEldPlayBark()
{
}

void RodinBTNodeEldPlayBark::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Sound );
	m_SoundDef = ConfigManager::GetString( sSound, "", sDefinitionName );

	STATICHASH( SoundPE );
	const SimpleString SoundPE = ConfigManager::GetString( sSoundPE, "", sDefinitionName );
	m_SoundDefPE.InitializeFromDefinition( SoundPE );

	STATICHASH( Category );
	m_Category = ConfigManager::GetHash( sCategory, HashedString::NullString, sDefinitionName );
}

RodinBTNode::ETickStatus RodinBTNodeEldPlayBark::Tick( float DeltaTime )
{
	Unused( DeltaTime );

	WBEntity* const pEntity = GetEntity();

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = pEntity;
	m_SoundDefPE.Evaluate( PEContext );
	const SimpleString SoundDef = ( m_SoundDefPE.GetType() == WBParamEvaluator::EPT_String ) ? m_SoundDefPE.GetString() : m_SoundDef;

	WB_MAKE_EVENT( PlayBark, pEntity );
	WB_SET_AUTO( PlayBark, Hash, Sound, SoundDef );
	WB_SET_AUTO( PlayBark, Hash, Category, m_Category );
	WB_DISPATCH_EVENT( GetEventManager(), PlayBark, pEntity );

	return ETS_Success;
}