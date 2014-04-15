#include "core.h"
#include "wbpeeldgetpersistentvar.h"
#include "configmanager.h"
#include "eldritchframework.h"
#include "eldritchgame.h"
#include "eldritchpersistence.h"

WBPEEldGetPersistentVar::WBPEEldGetPersistentVar()
:	m_Key()
{
}

WBPEEldGetPersistentVar::~WBPEEldGetPersistentVar()
{
}

/*virtual*/ void WBPEEldGetPersistentVar::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Key );
	m_Key = ConfigManager::GetHash( sKey, HashedString::NullString, sDefinitionName );
}

/*virtual*/ void WBPEEldGetPersistentVar::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EldritchFramework* const pFramework = EldritchFramework::GetInstance();
	ASSERT( pFramework );

	EldritchGame* const pGame = pFramework->GetGame();
	ASSERT( pGame );

	EldritchPersistence* const pPersistence = pGame->GetPersistence();
	ASSERT( pPersistence );

	const WBEvent& PersistentVars = pPersistence->GetVariableMap();
	const WBEvent::SParameter* pParam = PersistentVars.GetParameter( m_Key );

	EvaluatedParam.Set( pParam );
}