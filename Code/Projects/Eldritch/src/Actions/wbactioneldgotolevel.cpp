#include "core.h"
#include "wbactioneldgotolevel.h"
#include "eldritchframework.h"
#include "eldritchgame.h"
#include "configmanager.h"
#include "wbeventmanager.h"

WBActionEldGoToLevel::WBActionEldGoToLevel()
:	m_GoToNextLevel( false )
,	m_GoToPrevLevel( false )
,	m_ReturnToHub( false )
,	m_Restart( false )
,	m_FullRestart( false )
,	m_Immediate( false )
,	m_Level()
,	m_WorldDef()
{
}

WBActionEldGoToLevel::~WBActionEldGoToLevel()
{
}

/*virtual*/ void WBActionEldGoToLevel::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( GoToNextLevel );
	m_GoToNextLevel = ConfigManager::GetBool( sGoToNextLevel, false, sDefinitionName );

	STATICHASH( GoToPrevLevel );
	m_GoToPrevLevel = ConfigManager::GetBool( sGoToPrevLevel, false, sDefinitionName );

	STATICHASH( ReturnToHub );
	m_ReturnToHub = ConfigManager::GetBool( sReturnToHub, false, sDefinitionName );

	STATICHASH( Restart );
	m_Restart = ConfigManager::GetBool( sRestart, false, sDefinitionName );

	STATICHASH( FullRestart );
	m_FullRestart = ConfigManager::GetBool( sFullRestart, false, sDefinitionName );

	STATICHASH( Immediate );
	m_Immediate = ConfigManager::GetBool( sImmediate, false, sDefinitionName );

	STATICHASH( Level );
	m_Level = ConfigManager::GetString( sLevel, "", sDefinitionName );

	MAKEHASH( m_Level );

	STATICHASH( WorldDef );
	m_WorldDef = ConfigManager::GetHash( sWorldDef, HashedString::NullString, sm_Level );
}

/*virtual*/ void WBActionEldGoToLevel::Execute()
{
	WBAction::Execute();

	EldritchGame* const		pGame			= EldritchFramework::GetInstance()->GetGame();
	ASSERT( pGame );

	WBEventManager* const	pEventManager	= WBWorld::GetInstance()->GetEventManager();
	ASSERT( pEventManager );

	if( m_GoToNextLevel )
	{
		WB_MAKE_EVENT( GoToNextLevel, NULL );
		WB_LOG_EVENT( GoToNextLevel );
		WB_DISPATCH_EVENT( pEventManager, GoToNextLevel, pGame );
	}
	else if( m_GoToPrevLevel )
	{
		WB_MAKE_EVENT( GoToPrevLevel, NULL );
		WB_LOG_EVENT( GoToPrevLevel );
		WB_DISPATCH_EVENT( pEventManager, GoToPrevLevel, pGame );
	}
	else if( m_ReturnToHub )
	{
		WB_MAKE_EVENT( ReturnToHub, NULL );
		WB_LOG_EVENT( ReturnToHub );
		WB_SET_AUTO( ReturnToHub, Bool, Restart, m_Restart || m_FullRestart );
		WB_SET_AUTO( ReturnToHub, Bool, FlushHub, m_FullRestart );
		WB_DISPATCH_EVENT( pEventManager, ReturnToHub, pGame );
	}
	else
	{
		WB_MAKE_EVENT( GoToLevel, NULL );
		WB_LOG_EVENT( GoToLevel );
		WB_SET_AUTO( GoToLevel, Hash, Level, m_Level );
		WB_SET_AUTO( GoToLevel, Hash, WorldDef, m_WorldDef );
		WB_DISPATCH_EVENT( pEventManager, GoToLevel, pGame );
	}

	if( m_Immediate )
	{
		WB_MAKE_EVENT( GoToLevelImmediate, NULL );
		WB_LOG_EVENT( GoToLevelImmediate );
		WB_DISPATCH_EVENT( pEventManager, GoToLevelImmediate, pGame );
	}
}